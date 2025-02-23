#pragma once
#ifndef THREADPOOL
#define THREADPOOL
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <atomic>
#include"pch.h"
#include"concurrentqueue.h"

#ifdef _WIN32
#include<ppl.h>
#endif


class SemaphoreSlim
{
public:

    SemaphoreSlim() {}

    void Set()
    {
        std::lock_guard<std::mutex> _(mainMutex);
        cnt++;
        cv.notify_one();
    }

    void Set(int num)
    {
        std::lock_guard<std::mutex> _(mainMutex);
        cnt += num;
        for (int i = 0; i < num; i++)
        {
            cv.notify_one();
        }
    }

    void WaitOne()
    {
        std::unique_lock<std::mutex> lk(mainMutex);
        while (cnt == 0)
            cv.wait(lk);
        cnt--;
    }

private:

    SemaphoreSlim(const SemaphoreSlim&) = delete;
    SemaphoreSlim& operator=(const SemaphoreSlim&) = delete;
    std::mutex mainMutex;
    std::condition_variable cv;
    int cnt = 0;
};


template<typename T>
class ConcurrentQueue {
public:
    virtual void Enqueue(T&& item) = 0;
    virtual bool TryDequeue(T& out) = 0;
};

template<typename T>
class LockingQueue : public ConcurrentQueue<T>
{
public:

    void Enqueue(T&& item)
    {
        std::lock_guard l(qLock);
        workQueue.emplace_back(std::forward<T>(item));
    };

    bool TryDequeue(T& out)
    {
        std::lock_guard l(qLock);
        if (!workQueue.empty())
        {
            out = std::move(workQueue.front());
            workQueue.pop_front();
            return true;
        }
        return false;
    };

private:
    std::mutex qLock;
    std::deque<T> workQueue;

};



template<typename T>
class LockFreeQueue : public ConcurrentQueue<T>
{
public:

    void Enqueue(T&& item)
    {
        workQueue.enqueue(std::forward<T>(item));
    };

    bool TryDequeue(T& out)
    {
        return workQueue.try_dequeue(out);
    };

private:
    moodycamel::ConcurrentQueue<T> workQueue;
};

class ThreadPoolC
{
private:
    std::vector<std::thread> threads;
    std::atomic_bool kill = false;
    int poolSize=0;

    SemaphoreSlim signal;
    LockFreeQueue< std::function<void()> > workQueue;
    int maxPoolSize = std::thread::hardware_concurrency() - 1;
    std::mutex m;


public:

    ThreadPoolC()
    {  
    };
    ThreadPoolC(int poolSize)
    {
		ExpandPool(poolSize);
    };

    ~ThreadPoolC()
    {
        kill = true;
        signal.Set();
        for (int i = 0; i < poolSize; i++)
        {
            if (threads[i].joinable())
                threads[i].join();
            //std::cout << "Joined" << "\n";

        }
    }

    inline void ExpandPool(int numThreads)
    {
        int expansion = numThreads - poolSize;
        if (expansion < 1)
            return;  
        if (poolSize==maxPoolSize)
            return;
        std::lock_guard<std::mutex> _(m);

        expansion = numThreads - poolSize;
        if (expansion < 1)
            return;


        if (expansion > (maxPoolSize - poolSize))
            expansion = maxPoolSize - poolSize;

        std::cout << "Expanded by " << expansion << " threads\n";

        for (int i = 0; i < expansion; i++)
        {
            std::thread t([this]() { Execution(); });
          /*  HANDLE hThread = t.native_handle();
            if (!SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST)) {
                std::cerr << "Failed to set thread priority: " << GetLastError() << std::endl;
            }*/
            threads.emplace_back(std::move(t));
        }

        poolSize += expansion;  // Update pool size while holding the lock
    }

    template<typename F>
    void For(int fromInclusive, int toExclusive, F&& lambda)
    {

        const int numIter = toExclusive - fromInclusive;

        if (numIter <= 0)
            return;

        if (numIter == 1)
        {
            lambda(toExclusive - 1);
            return;
        }

        ExpandPool(numIter - 1);

        

        std::mutex m1;
        std::atomic<int> remainingWork(numIter);
        std::unique_lock<std::mutex> completionLock(m1, std::defer_lock);

        std::condition_variable cnd;
        bool alreadySignalled = false;

        signal.Set(numIter - 1);
        for (int i = fromInclusive; i < toExclusive - 1; i++)
        {
            workQueue.Enqueue([i, &cnd, &m1, &lambda, &alreadySignalled, &remainingWork]()
                {
                    lambda(i);
                    if (--remainingWork < 1)
                    {
                        std::lock_guard<std::mutex> lock(m1);
                        alreadySignalled = true;
                        cnd.notify_one();
                    }

                });
            //signal.Set();
        }
        signal.Set(numIter-1);
        lambda(toExclusive - 1);

        if (--remainingWork > 0)
        {
            Steal(remainingWork);

            completionLock.lock();
            if (!alreadySignalled)
                cnd.wait(completionLock, [&alreadySignalled] { return alreadySignalled; });
        }

    };
private:

    inline void Execution()
    {
        thread_local std::function<void()> task;
        while (true)
        {
            signal.WaitOne();
            if (kill)
            {
                signal.Set();
                break;
            }

            while (workQueue.TryDequeue(task))
            {

                try
                {
                    task();
                }
                catch (const std::exception& ex)
                {
                    std::cout << "Exception occured on thread pool delegate: " << ex.what() << "\n";
                }
            }

        }

    };


    inline void Steal(std::atomic<int>& remainingWork)
    {
        thread_local std::function<void()> task;
        while (remainingWork > 0)
        {

            if (workQueue.TryDequeue(task))
            {
                try
                {
                    //std::cout << "Stole : " << "\n";
                    task();
                }
                catch (const std::exception& ex)
                {
                    std::cout << "Exception occured on thread pool delegate : " << ex.what() << "\n";
                }

            }
        }
    }

};

class ThreadPool {
private:
   

public:
   
    static std::unique_ptr<ThreadPoolC> pool;
    

#ifdef _WIN32
    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        if (UseCustomPool>0) 
        {
            pool->For(i, j, std::forward<F>(lamb));
        }
        else 
        {
            concurrency::parallel_for(i, j, std::forward<F>(lamb));
        }
       
    }

    static int UseCustomPool;
    static int CustomPoolInitialized;
    static void SetCustomPool(int value);

#else
    
    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        pool->For(i, j, std::forward<F>(lamb));
    }

#endif

};


#endif





