#include "ThreadPool.h"

const int TASK_MAX_THRESHHOLD = 1000;
const int THREAD_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 10;

ThreadPool::ThreadPool()
    : _initThreadSize(0)
    , _threadSizeThreshHold(THREAD_MAX_THRESHHOLD)
    , _curThreadSize(0)
    , _idleThreadSize(0)
    , _taskSize(0)
    , _taskQueMaxThreshHold(TASK_MAX_THRESHHOLD)
    , _poolMode(PoolMode::MODE_FIXED)
    , _isPoolRunning(false){

}

ThreadPool::~ThreadPool() {
    _isPoolRunning = false;
    std::unique_lock<std::mutex> lock(_taskQueMtx);
    _notEmpty.notify_all();
    _notFull.notify_all();
    _exitCond.wait(lock, [&](){return _threads.size() == 0;});
}

void ThreadPool::setMode(PoolMode mode) {
    if (checkRunningState()) {
        return ;
    }
    _poolMode = mode;
}

void ThreadPool::setTaskQueMaxThreshHold(int threshHold) {
    if (checkRunningState()) {
        return ;
    }
    _taskQueMaxThreshHold = threshHold;
}

void ThreadPool::setThreadSizeThreshHold(int threshhold) {
    if (checkRunningState()) {
        return;
    }
    if (_poolMode == PoolMode::MODE_CACHED) {
        _threadSizeThreshHold = threshhold;
    }
}

bool ThreadPool::checkRunningState() const {
    return _isPoolRunning;
}

std::shared_ptr<Result> ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    
    std::unique_lock<std::mutex> lock(_taskQueMtx);

    if (!_notFull.wait_for(lock, std::chrono::seconds(1),
        [&](){return _taskQue.size() < (size_t)_taskQueMaxThreshHold;})) {
            std::cerr << "task queue is full, submit task failed!" << std::endl;
            return std::make_shared<Result>(sp, false);
            // return Result(sp, false);
            
    }

    //=======================================
    //=========修改代码=======================
    auto result = std::make_shared<Result>(sp);
    sp->setResult(result);
    //shared_ptr 可以隐式转换为 weak_ptr,所以直接声明为shared_ptr就行
    //=========修改代码=======================
    //=======================================

    _taskQue.emplace(sp);
    _taskSize++;

    _notEmpty.notify_all();

    if (_poolMode == PoolMode::MODE_CACHED 
        && _taskSize > _idleThreadSize
        && _curThreadSize < _threadSizeThreshHold) {
            std::cout << ">>> create new thread..." << std::endl;
            auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
            int threadId = ptr->getId();
            _threads.emplace(threadId, std::move(ptr));
            _threads[threadId]->start();
            _curThreadSize++;
            _idleThreadSize++;
        }

    // return std::make_shared<Result>(sp);
    return result;
}

void ThreadPool::startPool(int initThreadSize) {
    _isPoolRunning = true;
    _initThreadSize = initThreadSize;
    _curThreadSize = initThreadSize;

    for (int i = 0; i < _initThreadSize; ++i) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        _threads.emplace(threadId, std::move(ptr));
    }

    for (int i = 0; i < _initThreadSize; ++i) {
        _threads[i]->start();
        _idleThreadSize++;
    }
}

void ThreadPool::threadFunc(int threadid) {
    auto lastTime = std::chrono::high_resolution_clock().now();
    for (;;) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> lock(_taskQueMtx);

            while (_taskQue.size() == 0) {

                if (!_isPoolRunning) {
                    _threads.erase(threadid);
                    std::cout << "threadid:" << std::this_thread::get_id() << "exit!" << std::endl;
                    _exitCond.notify_all();
                    return ;
                }

                if (_poolMode == PoolMode::MODE_CACHED) {
                    if (std::cv_status::timeout == _notEmpty.wait_for(lock, std::chrono::seconds(1))) {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now-lastTime);
                        if (dur.count() >= THREAD_MAX_IDLE_TIME && _curThreadSize > _initThreadSize) {
                            _threads.erase(threadid);
                            _curThreadSize--;
                            _idleThreadSize--;
                            std::cout << "threadid:" << std::this_thread::get_id() << "exit!" << std::endl;
                            //===========================================
                            //======必须真正退出（return）并通知等待者======
                            _exitCond.notify_all();  // ✅ 建议加上
                            return ;
                        }
                    }
                }
                else {
                    _notEmpty.wait(lock);       
                }
            }
            _idleThreadSize--;
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
        _idleThreadSize++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
}

int Thread::_generateTd = 0;

int Thread::getId() const {
    return _threadId;
}

Thread::Thread(ThreadFunc func):_func(func),_threadId(_generateTd++) {}

Thread::~Thread() {}

void Thread::start() {
    std::thread t(_func, _threadId);
    t.detach();

}

Result::Result(std::shared_ptr<Task> task, bool isValid)
    : _isValid(isValid)
    , _task(task) {
        // task->setResult(this);
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

// void Task::exec() {
//     auto result = _result.lock();
//     if (result){
//         result->setVal(run());
//     }
//     else {
//         std::cout << "Result已被销毁，跳过写入" << std::endl;
//     }
// }

void Task::exec() {
    auto result = _result.lock();
    if (!result) {
        // Result 已经被销毁，直接返回（或者记录日志）
        std::cout << "Task::exec: result expired, skip." << std::endl;
        return;
    }

    try {
        // 正常设置结果
        result->setVal(run());
    } catch (const std::exception &e) {
        // 把异常信息以可识别的 Any 形式返回，或者用你们的错误通道
        result->setVal(Any(std::string(std::string("task exception: ") + e.what())));
    } catch (...) {
        result->setVal(Any(std::string("task unknown exception")));
    }
    // 注意：Result::setVal 内部应该会做 semaphore.post() / notify，这里我们保证至少会调用 setVal
}


void Task::setResult(std::shared_ptr<Result> res) {
    _result = res;
}