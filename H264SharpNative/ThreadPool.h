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
        for (size_t i = 0; i < num; i++)
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
    void Enqueue(T&& item, SemaphoreSlim& signal)
    {
        std::lock_guard l(qLock);
        signal.Set();
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



class ThreadPoolC
{
private:
    std::vector<std::thread> threads;
    std::atomic_bool kill = false;
    int poolSize;

    SemaphoreSlim signal;
    LockingQueue< std::function<void()> > workQueue;


public:

    ThreadPoolC()
    {
        poolSize = std::thread::hardware_concurrency() - 1;

        for (int i = 0; i < poolSize; i++)
        {
            threads.push_back(std::thread([this]() {Execution(); }));
        }
    };
    ThreadPoolC(int poolSize)
    {
        if (poolSize < 1)
            poolSize = 1;

        if (poolSize > 1)
            poolSize--;

        for (int i = 0; i < poolSize; i++)
        {
            threads.push_back(std::thread([this]() {Execution(); }));
        }

        this->poolSize = poolSize;
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

        std::atomic<int> remainingWork(numIter);
        std::mutex m1;
        std::unique_lock<std::mutex> completionLock(m1, std::defer_lock);

        std::condition_variable cnd;
        bool alreadySignalled = false;



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
            signal.Set();
        }




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


    void Steal(std::atomic<int>& remainingWork)
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
public:
   
#ifdef _WIN32
    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        concurrency::parallel_for(i, j, std::forward<F>(lamb));
    }

#else
    static ThreadPoolC pool;
    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        pool.For(i, j, std::forward<F>(lamb));
    }

#endif

};


#endif





