#include "ThreadPool.h"

#ifndef _WIN32
std::unique_ptr<ThreadPoolC> ThreadPool::pool = std::make_unique<ThreadPoolC>();
#else
int ThreadPool::UseCustomPool = 0;
std::unique_ptr<ThreadPoolC> ThreadPool::pool = nullptr;

std::mutex poolMutex;

void ThreadPool::SetCustomPool(int value)
{
    if (value < 1) {
        UseCustomPool = value;
        return;
    }

    // Lazily initialize the pool in a thread-safe manner
    if (pool == nullptr) {
        std::lock_guard<std::mutex> lock(poolMutex);
        if (pool == nullptr) {
            auto newPool = std::make_unique<ThreadPoolC>();
            pool = std::move(newPool);
        }
    }
    UseCustomPool = value;
}
#endif


