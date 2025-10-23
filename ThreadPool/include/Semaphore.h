// #pragma once
// #include <mutex>
// #include <condition_variable>
// #include <chrono>
// #include <stdexcept>

// class Semaphore {
// public:
//     explicit Semaphore(int limit = 0); 
//     ~Semaphore() = default;
    
//     void wait();
//     void post();
//     bool try_wait();
//     bool wait_for(std::chrono::milliseconds duration);

//     int available() const;
//     int max() const;

// private:
//     int _resLimit;
//     const int _maxLimit;
//     mutable std::mutex _mtx;
//     std::condition_variable _cond;
// };

