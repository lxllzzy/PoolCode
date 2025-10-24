#include "ThreadPool.h"

const int TASK_MAX_THRESHHOLD = 1000;

ThreadPool::ThreadPool()
    : _initThreadSize(4)
    , _threadSizeThreshHold(1024) 
    , _taskSize(0)
    , _taskQueMaxThreshHold(TASK_MAX_THRESHHOLD)
    , _poolMode(PoolMode::MODE_FIXED){

}

ThreadPool::~ThreadPool() {

}

void ThreadPool::setMode(PoolMode mode) {
    _poolMode = mode;
}

void ThreadPool::setTaskQueMaxThreshHold(int threshHold) {
    _taskQueMaxThreshHold = threshHold;
}

Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock<std::mutex> lock(_taskQueMtx);

    if (!_notFull.wait_for(lock, std::chrono::seconds(1),
        [&](){return _taskQue.size() < (size_t)_taskQueMaxThreshHold;})) {
            std::cerr << "task queue is full, submit task failed!" << std::endl;
            // return std::make_shared<Result>(sp, false);
            return Result(sp, false);
    }

    _taskQue.push(sp);
    _taskSize++;

    _notEmpty.notify_all();
    // return std::make_shared<Result>(sp);
    return Result(sp);
}

void ThreadPool::startPool(int initThreadSize) {
    _initThreadSize = initThreadSize;

    for (int i = 0; i < _initThreadSize; ++i) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        _threads.emplace_back(std::move(ptr));
    }

    for (int i = 0; i < _initThreadSize; ++i) {
        _threads[i]->start();

    }
}

void ThreadPool::threadFunc() {
    for (;;) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(_taskQueMtx);
            _notEmpty.wait(lock, [&](){return _taskQue.size() > 0;});
            task = _taskQue.front();
            _taskQue.pop();
            _taskSize--;
            if (_taskQue.size() > 0) {
                _notEmpty.notify_all();
            }

            _notFull.notify_all();
        }
        if (task != nullptr) {
            task->exec();
        }
    }
}

Thread::Thread(ThreadFunc func):_func(func) {

}

Thread::~Thread() {

}

void Thread::start() {
    std::thread t(_func);
    t.detach();

}

Result::Result(std::shared_ptr<Task> task, bool isValid)
    : _isValid(isValid)
    , _task(task) {
        task->setResult(this);
}

void Result::setVal(Any any) {
    this->_any = std::move(any);
    _sem.post();
}

Any Result::get() {
    if (!_isValid) {
        return "";
    }
    _sem.wait();
    return std::move(_any);
}

Task::Task():_result(nullptr) {}

void Task::exec() {
    if (_result != nullptr){
        _result->setVal(run());
    }
}

void Task::setResult(Result* res) {
    _result = res;
}