#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <mysql/mysql.h>
#include "public.h"
using namespace std;

class Connection {
public:
    Connection();
    ~Connection();

    bool connect(string ip, 
        unsigned short port,
        string user, 
        string password,
        string dbName);

    bool update(string sql);
    
    MYSQL_RES* query(string sql);
    
    void refreshAliveTime() {
        _aliveTime = clock();
    }

    clock_t getAliveTime() const {
        return clock() - _aliveTime;
    }

private:
    MYSQL* _conn;
    clock_t _aliveTime;
};