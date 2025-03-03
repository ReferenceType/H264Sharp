#include "ThreadPool.h"

#ifndef _WIN32
std::unique_ptr<ThreadPoolC> ThreadPool::pool = std::make_unique<ThreadPoolC>();
#else
int ThreadPool::UseCustomPool = 0;
std::unique_ptr<ThreadPoolC> ThreadPool::pool = nullptr;

std::mutex poolMutex;
int ThreadPool::reqNumThreads = 0;
int ThreadPool::minChunk = 16;

void ThreadPool::SetCustomPool(int value, int numTh)
{
    if (value < 1) {
        UseCustomPool = value;
        return;
    }

    // Lazily initialize the pool in a thread-safe manner
    if (pool == nullptr) {
        std::lock_guard<std::mutex> lock(poolMutex);
        if (pool == nullptr) {
            auto newPool = std::make_unique<ThreadPoolC>(numTh);
            pool = std::move(newPool);
        }
    }
    UseCustomPool = value;
}
#endif

void ThreadPool::Expand(int num) 
{
    ThreadPool::reqNumThreads = num;
    if (pool != nullptr) {
        pool.get()->ExpandPool(num-1);
        
    }
}


