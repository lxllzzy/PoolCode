#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>
#include <unordered_map>

class Any {
public:
    Any() = default;
    ~Any() = default;
    Any(const Any &) = delete;
    Any& operator=(const Any &) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data) : _base(std::make_unique<Derive<T>>(data)) {}

    template<typename T>
    T cast_() {
        Derive<T>* pd = dynamic_cast<Derive<T>*>(_base.get());
        if (pd == nullptr) {
            throw "type is unmatch!";
        }
        return pd->_data;
    }

private:
    class Base {
    public:
        virtual ~Base() = default;
    };

    template<typename T>
    class Derive : public Base {
    public:
        Derive(T data) : _data(data) {}
        T _data;
    };

private:
    std::unique_ptr<Base> _base;
};


class Semaphore {
public:
    Semaphore(int limit = 0) : _resLimit(limit) {}
    ~Semaphore() = default;

    void wait() {
        std::unique_lock<std::mutex> lock(_mtx);
        _cond.wait(lock, [&](){return _resLimit > 0;});
        _resLimit--;
    }

    void post() {
        std::unique_lock<std::mutex> lock(_mtx);
        _resLimit++;
        _cond.notify_all();
    }
private:
    int _resLimit;
    std::mutex _mtx;
    std::condition_variable _cond;
};

class Task;

class Result {
public:
    // 禁止拷贝构造和拷贝赋值
    // Result(const Result&) = delete;
    // Result& operator=(const Result&) = delete;

    // // 支持移动构造和移动赋值
    // Result(Result&&) noexcept = default;
    // Result& operator=(Result&&) noexcept = default;
    /*
    ================================================
    ================================================
    ===============解决问题方法======================
    ================================================
    ================================================
    */
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    void setVal(Any any);

    // template <typename T>
    // T cast() {
    //     _sem.wait();
    //     return _any.cast_<T>();
    // }

    Any get();
    
private:
    Any _any;
    Semaphore _sem;
    std::shared_ptr<Task> _task;
    std::atomic_bool _isValid;
};

class Task {
public:
    Task();
    ~Task() = default;
    void exec();
    void setResult(Result* res); 

    virtual Any run() = 0;
private:
    Result* _result;
};

//c++新标准class
enum class PoolMode {
    MODE_FIXED,
    MODE_CACHED,
};

class Thread {
public:
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func);
    ~Thread();

    void start();

private:
    ThreadFunc _func;

};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void setMode(PoolMode mode);

    void setTaskQueMaxThreshHold(int threshHold);

    // std::shared_ptr<Result> submitTask(std::shared_ptr<Task> sp);
    Result submitTask(std::shared_ptr<Task> sp);

    void startPool(int initThreadSize = 4);

    ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;


private:
    void threadFunc();//为什么这么设计
private: 
    std::vector<std::unique_ptr<Thread>> _threads;
    int _initThreadSize;
    int _threadSizeThreshHold;

    std::queue<std::shared_ptr<Task>> _taskQue;
    std::atomic_int _taskSize;
    int _taskQueMaxThreshHold;

    std::mutex _taskQueMtx;
    std::condition_variable _notFull;
    std::condition_variable _notEmpty;

    PoolMode _poolMode;
};