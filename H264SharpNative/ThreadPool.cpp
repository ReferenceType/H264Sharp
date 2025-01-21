#include "ThreadPool.h"

#ifndef _WIN32
auto newPool = std::make_unique<ThreadPoolC>();
auto ThreadPool::pool = std::move(newPool);
#else
int ThreadPool::UseCustomPool = 0;
std::unique_ptr<ThreadPoolC> ThreadPool::pool = nullptr;
#endif

std::mutex poolMutex;

void ThreadPool::SetCustomPool(int value)
{
    // Lazily initialize the pool in a thread-safe manner
    if (pool == nullptr) {
        std::lock_guard<std::mutex> lock(poolMutex);
        if (pool == nullptr) 
        { 
            auto newPool = std::make_unique<ThreadPoolC>(); 
            pool = std::move(newPool); 
        }
    }
    UseCustomPool = value;
}
