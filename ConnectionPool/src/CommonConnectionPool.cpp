#include "CommonConnectionPool.h"

ConnectionPool* ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return &instance;
}

shared_ptr<Connection> ConnectionPool::getConnection() {
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQue.empty()) {
        if (cv_status::timeout == _cv.wait_for(lock, chrono::milliseconds(_connectionTimeout))) {
            if (_connectionQue.empty()) {
                LOG("获取连接超时...获取连接失败！");
                return nullptr;
            }
        }
    }

    shared_ptr<Connection> sp(_connectionQue.front(),
        [&](Connection *p) {
            unique_lock<mutex> lock(_queueMutex);
            p->refreshAliveTime();
            _connectionQue.push(p);
    });

    _connectionQue.pop();
    _cv.notify_all();
    return sp;
}

ConnectionPool::ConnectionPool() {
    if (!loadConfigFile()) {
        return ;
    }

    for (int i = 0; i < _initSize; ++i) {
        Connection *p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime();
        _connectionQue.push(p);
        _connectionCnt++;
    }

    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
    
}

bool ConnectionPool::loadConfigFile() {
    FILE* fp = fopen("mysql.conf", "r");
    if (fp == nullptr) {
        LOG("mysql.conf file is not exits!");
        return false;
    }

    while (!feof(fp)) {
        char line[1024] = {0};
        fgets(line, 1024, fp);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1) {
            continue;
        }

        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, endidx - idx - 1);

        if (key == "ip") {
            _ip = value; 
        } 
        else if (key == "port") {
            _port = atoi(value.c_str());
        }
        else if (key == "username") {
            _username = value;
        }
        else if (key == "password") {
            _password = value;
        }
        else if (key == "dbname") {
            _dbname = value;
        }
        else if (key == "initSize") {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxSize") {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime") {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeout") {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    return true;
}

void ConnectionPool::produceConnectionTask() {
    for (;;) {
        unique_lock<mutex> lock(_queueMutex);
        while (!_connectionQue.empty()) {
            _cv.wait(lock);
        }

        if (_connectionCnt < _maxSize) {
            Connection* p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            p->refreshAliveTime();
            _connectionQue.push(p);
            _connectionCnt++;
        }
        _cv.notify_all();
    }
}

void ConnectionPool::scannerConnectionTask() {
    for (;;) {
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));

        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize) {
            Connection *p = _connectionQue.front();
            if (p->getAliveTime() >= (_maxIdleTime * 1000)) {
                _connectionQue.pop();
                _connectionCnt--;
                delete p;   
            }
            else {
                break;
            }
        }
    }
}