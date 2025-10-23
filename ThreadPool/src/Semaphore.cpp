// #include "Semaphore.h"

// Semaphore::Semaphore(int limit) 
//     : _resLimit(limit), _maxLimit(limit) {
//     if (limit < 0) {
//         throw std::invalid_argument("Semaphore limit must be non-negative");
//     }
// }

// void Semaphore::wait() {
//     std::unique_lock<std::mutex> lock(_mtx);
//     _cond.wait(lock, [this]() { return _resLimit > 0; });
//     --_resLimit;
// }

// void Semaphore::post() {
//     std::unique_lock<std::mutex> lock(_mtx);
//     if (_resLimit < _maxLimit) {
//         ++_resLimit;
//         _cond.notify_one();  // notify one waiting thread
//     }
// }

// bool Semaphore::try_wait() {
//     std::lock_guard<std::mutex> lock(_mtx);
//     if (_resLimit > 0) {
//         --_resLimit;
//         return true;
//     }
//     return false;
// }

// bool Semaphore::wait_for(std::chrono::milliseconds duration) {
//     std::unique_lock<std::mutex> lock(_mtx);
//     if (_cond.wait_for(lock, duration, [this]() { return _resLimit > 0; })) {
//         --_resLimit;
//         return true;
//     }
//     return false;  // Timeout occurred
// }

// int Semaphore::available() const {
//     std::lock_guard<std::mutex> lock(_mtx);
//     return _resLimit;
// }

// int Semaphore::max() const {
//     std::lock_guard<std::mutex> lock(_mtx);
//     return _maxLimit;
// }
