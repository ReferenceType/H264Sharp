#ifndef THREADPOOL
#define THREADPOOL
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <functional>
#include <iostream>
#include <atomic>
#include"pch.h"
#include <variant>

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/syscall.h>
#include <linux/futex.h>
#include <unistd.h> 
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach/semaphore.h>
#endif

#ifdef _WIN32
#include<ppl.h>
#include <limits.h>
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#define CPU_PAUSE() _mm_pause()

#elif defined(ARM)
#include <arm_acle.h>
#define CPU_PAUSE() __yield()

#else
#define CPU_PAUSE() std::this_thread::yield()
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
#define WFUTEX
class Semaphore {

public:

#if defined(_WIN32)
#ifndef WFUTEX
    Semaphore(int initial = 0)
    {
        win_semaphore = CreateSemaphore(nullptr, initial, INT_MAX, nullptr);
    }

    ~Semaphore()
    {
        CloseHandle(win_semaphore);
    }

    void Set()
    {
        Set(1);
    }

    void Set(int num)
    {
        ReleaseSemaphore(win_semaphore, num, nullptr);
    }

    void WaitOne()
    {
        WaitForSingleObject(win_semaphore, INFINITE);
    }

#else

    Semaphore(int initial = 0) : semCnt(initial) {}


    void Set() {
        Set(1);
    }

    void Set(int num) {
        InterlockedAdd(&semCnt, num);

        if (num == 1) {
            WakeByAddressSingle(&semCnt);
        }
        else {
            WakeByAddressAll(&semCnt);
        }
    }

    void WaitOne() {
        while (true) {
            long current = semCnt;

            if (current > 0) {
                if (InterlockedCompareExchange(&semCnt, current - 1, current) == current) {
                    return;
                }
                continue;
            }

            long zero = 0;
            WaitOnAddress(&semCnt, &zero, sizeof(semCnt), INFINITE);
        }
    }

#endif // !WFUTEX

#elif defined(__linux__) || defined(__ANDROID__)
    Semaphore(int initial = 0)
    {
        count.store(initial, std::memory_order_release);
    }

    void Set()
    {
        Set(1);
    }

    void Set(int num)
    {
        count.fetch_add(num, std::memory_order_release);
        syscall(SYS_futex, &count, FUTEX_WAKE_PRIVATE, num, nullptr, nullptr, 0);
    }

    void WaitOne()
    {
        while (true) {
            int expected = count.load(std::memory_order_acquire);
            if (expected > 0 && count.compare_exchange_weak(expected, expected - 1, std::memory_order_acquire)) {
                return;
            }
            syscall(SYS_futex, &count, FUTEX_WAIT_PRIVATE, 0, nullptr, nullptr, 0);
        }
    }
#elif defined(__APPLE__)
    Semaphore(int initial = 0)
    {
        semaphore_create(mach_task_self(), &mach_semaphore, SYNC_POLICY_FIFO, initial);
    }

    ~Semaphore()
    {
        semaphore_destroy(mach_task_self(), mach_semaphore);
    }

    void Set()
    {
        Set(1);
    }

    void Set(int num)
    {
        for (int i = 0; i < num; ++i)
        {
            semaphore_signal(mach_semaphore);
        }
    }

    void WaitOne()
    {
        semaphore_wait(mach_semaphore);
    }
#endif

private:
#if defined(__linux__) || defined(__ANDROID__)
    std::atomic<int> count;
#elif defined(__APPLE__)
    semaphore_t mach_semaphore;
#elif defined(_WIN32)
#ifndef WFUTEX
    HANDLE win_semaphore;
#endif // !WFUTEX
    alignas(8) long semCnt;
#endif
};

class AutoresetEvent {

public:

#if defined(_WIN32)
#ifndef WFUTEX
    AutoresetEvent()
    {
        win_semaphore = CreateSemaphore(nullptr, 0, INT_MAX, nullptr);
    }

    ~AutoresetEvent()
    {
        CloseHandle(win_semaphore);
    }

    void Set()
    {
        ReleaseSemaphore(win_semaphore, 1, nullptr);
    }
    void WaitOne()
    {
        WaitForSingleObject(win_semaphore, INFINITE);
    }

#else

    AutoresetEvent() : semCnt(0) {}

    void Set()
    {
        if (InterlockedExchange(&semCnt, 1) == 0)
        {
            WakeByAddressSingle(&semCnt);
        }
    }

    void WaitOne()
    {
        while (true)
        {
            long current = InterlockedCompareExchange(&semCnt, 0, 0);

            if (current == 1) {

                if (InterlockedCompareExchange(&semCnt, 0, 1) == 1) {

                    return;
                }

                continue;
            }



            WaitOnAddress(&semCnt, &current, sizeof(semCnt), INFINITE);
        }
    }



#endif // !WFUTEX

#elif defined(__linux__) || defined(__ANDROID__)
    AutoresetEvent()
    {
        count.store(0, std::memory_order_release);
    }

    void Set()
    {
        if (count.exchange(1, std::memory_order_release) == 0)
        {
            syscall(SYS_futex, &count, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
        }
    }

    void WaitOne()
    {
        while (true) {

            int current = count.load(std::memory_order_acquire);

            if (current == 1)
            {

                bool success = count.compare_exchange_strong(current, 0, std::memory_order_acquire);
                if (success)
                {
                    return;
                }

                continue;
            }


            int expected = current;
            syscall(SYS_futex, &count, FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0);
        }
    }
#elif defined(__APPLE__)
    AutoresetEvent()
    {
        semaphore_create(mach_task_self(), &mach_semaphore, SYNC_POLICY_FIFO, 0);
    }

    ~AutoresetEvent()
    {
        semaphore_destroy(mach_task_self(), mach_semaphore);
    }

    void Set()
    {
        semaphore_signal(mach_semaphore);
    }

    void WaitOne()
    {
        semaphore_wait(mach_semaphore);
    }
#endif

private:
#if defined(__linux__) || defined(__ANDROID__)
    std::atomic<int> count;
#elif defined(__APPLE__)
    semaphore_t mach_semaphore;
#elif defined(_WIN32)
#ifndef WFUTEX
    HANDLE win_semaphore;
#endif // !WFUTEX
    alignas(8) long semCnt;
#endif
};

class SpinWait {
public:
    void wait() {
        int backoff = 1;
        while (!flag.load(std::memory_order_acquire))
        {
            for (size_t i = 0; i < backoff; i++)
            {
                CPU_PAUSE();
            }
            if (backoff < 16)
                backoff *= 2;
        }

    }

    void notify() {
        flag.store(true, std::memory_order_release);
    }

    void reset() {
        flag.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> flag{ false };
};


class SpinLock {
public:
    SpinLock() { flag.clear(std::memory_order_relaxed); }

    void lock()
    {
        int backoff = 1;
        while (flag.test_and_set(std::memory_order_acquire))
        {
            for (size_t i = 0; i < backoff; i++)
            {
                CPU_PAUSE();
            }
            if (backoff < 8)
                backoff *= 2;
        }
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
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
        //std::lock_guard<std::mutex> lock(qLock);

        spinLock.lock();
        workQueue.emplace_back(std::forward<T>(item));
        count++;
        spinLock.unlock();
    };

    bool TryDequeue(T& out)
    {
        //std::lock_guard<std::mutex> lock(qLock);


        spinLock.lock();
        if (!workQueue.empty())
        {
            out = std::move(workQueue.front());
            workQueue.pop_front();
            count--;
            spinLock.unlock();
            return true;
        }
        spinLock.unlock();
        return false;
    };
    bool TryDequeueBack(T& out)
    {
        //std::lock_guard<std::mutex> lock(qLock);


        spinLock.lock();
        if (!workQueue.empty())
        {
            out = std::move(workQueue.back());
            workQueue.pop_back();
            count--;
            spinLock.unlock();
            return true;
        }
        spinLock.unlock();
        return false;
    };

    bool IsEmpty()
    {
        return count == 0;
    }


private:
    std::mutex qLock;
    std::deque<T> workQueue;
    SpinLock spinLock;
    std::atomic<int> count = 0;
};

struct ITask
{
    virtual void operator()() = 0;
    virtual ~ITask() = default;
};

template <typename F>
struct SingleTask : ITask
{
    F lambda;
    int i;
    std::atomic<int>* remainingWork;
    SpinWait* spin;

    SingleTask(F&& lambda, int i, std::atomic<int>* rem, SpinWait* s)
        : lambda(std::forward<F>(lambda)), i(i), remainingWork(rem), spin(s) {
    }

    void operator()() override
    {
        lambda(i);
        if (--(*remainingWork) < 1)
        {
            spin->notify();
        }
    }
};

template <typename F>
struct RangeTask : ITask
{
    F lambda;
    int start, end;
    std::atomic<int>* remainingWork;
    SpinWait* spin;

    RangeTask(F&& lambda, int start, int end, std::atomic<int>* rem, SpinWait* s)
        : lambda(std::forward<F>(lambda)), start(start), end(end), remainingWork(rem), spin(s) {
    }

    void operator()() override
    {
        lambda(start, end);
        if (--(*remainingWork) < 1)
        {
            spin->notify();
        }
    }
};

class ThreadPoolC
{
private:
    using Task = ITask*;

    std::vector<std::thread> threads;
    std::atomic_bool kill = false;
    std::atomic<int> poolSize = 0;
    int maxPoolSize = std::thread::hardware_concurrency() - 1;

    std::vector< LockingQueue<Task>* > threadLocalQueues;
    std::vector< AutoresetEvent* > threadLocalSignals;
    std::mutex m;
    std::atomic<int> activeThreadCount = 0;

public:
    int minChunk = 16;

    ThreadPoolC()
    {
        for (size_t i = 0; i < maxPoolSize; i++)
        {
            threadLocalQueues.emplace_back(nullptr);
            threadLocalSignals.emplace_back(nullptr);
        }

    };
    ThreadPoolC(int poolSize)
    {
        for (size_t i = 0; i < maxPoolSize; i++)
        {
            threadLocalQueues.emplace_back(nullptr);
            threadLocalSignals.emplace_back(nullptr);
        }

        ExpandPool(poolSize - 1);
    };

    ~ThreadPoolC()
    {
        kill = true;
        for (size_t i = 0; i < activeThreadCount; i++)
        {
            if (threadLocalSignals[i] != nullptr)
                threadLocalSignals[i]->Set();
        }
        for (int i = 0; i < poolSize; i++)
        {
            if (threads[i].joinable())
                threads[i].join();
            //logger << "Joined" << "\n";

        }
    }
    
    inline void ExpandPool(int numThreads)
    {
        int expansion = numThreads - poolSize;
        if (expansion < 1)
            return;
        if (poolSize == maxPoolSize)
            return;

        std::lock_guard<std::mutex> _(m);

        expansion = numThreads - poolSize;
        if (expansion < 1)
            return;

        if (expansion > (maxPoolSize - poolSize))
            expansion = maxPoolSize - poolSize;

        //logger << "Expanded by " << expansion << " threads\n";

        for (int i = 0; i < expansion; i++)
        {
            SpinWait initSignal;
            std::thread t([this, &initSignal]() { Execution(initSignal); });

            threads.emplace_back(std::move(t));
            initSignal.wait();
        }

        poolSize += expansion;
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
        std::atomic<int> remainingWork(numIter);
        SpinWait spin;
        alignas(alignof(SingleTask<F>)) std::byte taskStorage[numIter - 1][sizeof(SingleTask<F>)];
        int idx = 0;

        for (int i = fromInclusive; i < toExclusive - 1; i++)
        {
            auto* task = new (&taskStorage[idx++]) SingleTask<F>(std::forward<F>(lambda),i, &remainingWork, &spin);

            threadLocalQueues[i]->Enqueue(task);
            threadLocalSignals[i]->Set();
        }

        lambda(toExclusive - 1);

        if (--remainingWork > 0)
        {
            Steal(remainingWork);
            spin.wait();
        }

    };
    //template<typename F>
    //void ForX(int fromInclusive, int toExclusive, F&& lambda, int numThreads)
    //{
    //    //64-2
    //    //32-2 balance
    //    int minRangeSize = 32;
    //    int numIter = toExclusive - fromInclusive;
    //    int chunksPerTh = 1;

    //    if (numIter <= 0) return;

    //    // Start with max threads and reduce dynamically if needed
    //    int chunkLen = numIter / (numThreads * chunksPerTh);

    //    // Ensure chunk length is at least minChunkSize and even
    //    chunkLen = max(minRangeSize, (chunkLen / 2) * 2);

    //    // Reduce thread count if there are too few chunks
    //    int numChunks = numIter / chunkLen;
    //    while (numChunks < numThreads * chunksPerTh && numThreads > 1) {
    //        numThreads--;
    //        chunkLen = max(minRangeSize, numIter / (numThreads * chunksPerTh));
    //        chunkLen = (chunkLen / 2) * 2;
    //        numChunks = numIter / chunkLen;
    //    }

    //    std::atomic<int> remainingWork(numChunks);
    //    SpinWait spin;

    //    int assignedChunks = 0;
    //    int chunksPerThread = numChunks / numThreads;

    //    for (int t = 0; t < numThreads - 1; ++t)
    //    {

    //        int startChunk = t * chunksPerThread;
    //        int endChunk = startChunk + chunksPerThread;

    //        for (int i = startChunk; i < endChunk; ++i)
    //        {
    //            int start = fromInclusive + i * chunkLen;
    //            int end = min(start + chunkLen, toExclusive);

    //            threadLocalQueues[t]->Enqueue(std::make_unique<RangeTask<F>>
    //                (std::forward<F>(lambda),
    //                    start,
    //                    end,
    //                    &remainingWork,
    //                    &spin));
    //        }
    //        threadLocalSignals[t]->Set();

    //        assignedChunks += chunksPerThread;
    //    }

    //    int start = fromInclusive + assignedChunks * chunkLen;
    //    int end = toExclusive;
    //    lambda(start, end);
    //    remainingWork -= (numChunks - assignedChunks);

    //    if (remainingWork > 0)
    //    {
    //        Steal(remainingWork);
    //        spin.wait();
    //    }
    //}
   
    template<typename F>
    void ForRange(int fromInclusive, int toExclusive, F&& lambda, int numThreads, int minChunk)
    {
        int numIter = toExclusive - fromInclusive;
        if (numIter <= 0) return;

        ExpandPool(numThreads - 1);


        numThreads = min(numThreads, numIter / minChunk);
        int chunkLen = ((numIter / numThreads) / 2) * 2;

        std::atomic<int> remainingWork(numThreads);
        SpinWait spin;

        alignas(alignof(RangeTask<F>)) std::byte taskStorage[numThreads][sizeof(RangeTask<F>)];

        for (int t = 0; t < numThreads - 1; ++t)
        {
            int start = fromInclusive + t * chunkLen;
            int end = start + chunkLen;
            
            // stack alloc
            auto* task = new (&taskStorage[t]) RangeTask<F>(std::forward<F>(lambda), start, end, &remainingWork, &spin);

            threadLocalQueues[t]->Enqueue(task);
            threadLocalSignals[t]->Set();
        }

        int start = fromInclusive + (numThreads - 1) * chunkLen;
        int end = toExclusive;
        lambda(start, end);
        remainingWork--;

        if (remainingWork > 0)
        {
            Steal(remainingWork);
            spin.wait();
        }
    }


private:

    inline void Execution(SpinWait& qsignal)
    {
        thread_local Task task;
        LockingQueue< Task > workQueue;
        AutoresetEvent signal;
        int currThreadId = activeThreadCount;

        threadLocalQueues[activeThreadCount] = &workQueue;
        threadLocalSignals[activeThreadCount] = &signal;
        activeThreadCount++;

        qsignal.notify();

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
                    (*task)();
                }
                catch (const std::exception& ex)
                {
                    logger << "Exception occured on thread pool delegate: " << ex.what() << "\n";
                }
            }

            int activeCnt = activeThreadCount;

            for (int offset = 0; offset < activeCnt; offset++)
            {
                if (!workQueue.IsEmpty())
                    break;

                int i = (currThreadId + offset) % activeCnt;// circular steal

                if (threadLocalQueues[i] != nullptr)
                {
                    while (!threadLocalQueues[i]->IsEmpty() && workQueue.IsEmpty())
                    {
                        if (threadLocalQueues[i]->TryDequeue(task))
                        {
                            try
                            {
                                (*task)();
                                //logger << "Stole : ";
                            }
                            catch (const std::exception& ex)
                            {
                                logger << "Exception occured on thread pool delegate : " << ex.what() << "\n";
                            }
                        }
                    }

                }
            }

        }

    };


    inline void Steal(std::atomic<int>& remainingWork)
    {
        thread_local Task task;
        if (remainingWork > 0)
        {

            for (size_t i = 0; i < activeThreadCount; i++)
            {
                if (threadLocalQueues[i] != nullptr)
                {
                    while (!threadLocalQueues[i]->IsEmpty())
                    {
                        if (threadLocalQueues[i]->TryDequeue(task))
                        {
                            try
                            {
                                //logger << "Stole : ";
                                (*task)();

                                if (remainingWork == 0)
                                    return;
                            }
                            catch (const std::exception& ex)
                            {
                                logger << "Exception occured on thread pool delegate : " << ex.what() << "\n";
                            }
                        }
                    }
                }
            }
        }
    }

};
class ThreadPool {
private:


public:

    static std::unique_ptr<ThreadPoolC> pool;
    static void Expand(int value);


#ifdef _WIN32
    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        if (UseCustomPool > 0)
        {
            pool->For(i, j, std::forward<F>(lamb));
        }
        else
        {
            concurrency::parallel_for(i, j, std::forward<F>(lamb));
        }

    }

    template<typename F>
    static void ForRange(int width, int height, F&& lamb, int numThreads)
    {
        if (UseCustomPool > 0)
        {
            // based on experiment data. need to proc at least 245760 pixels to make sense
            int minChunkSize = (245760) / width;

            pool->ForRange(0, height, std::forward<F>(lamb),numThreads, minChunkSize);
        }
        else
        {

            int chunkLen = height / numThreads;
            if (chunkLen % 2 != 0) {
                chunkLen -= 1;
            }

            concurrency::parallel_for(0, numThreads, [&](int j)
                {
                    int bgn = chunkLen * j;
                    int end = bgn + chunkLen;

                    if (j == numThreads - 1) 
                        end = height;

                    lamb(bgn, end);

                }, concurrency::static_partitioner());
        }

    }

    static int UseCustomPool;
    static int CustomPoolInitialized;
    static void SetCustomPool(int value, int numTh);

#else

    template<typename F>
    static void For(int i, int j, F&& lamb)
    {
        pool->For(i, j, std::forward<F>(lamb));
    }

    template<typename F>
    static void ForRange(int width, int height, F&& lamb, int numThreads)
    {
        // based on experiment data. need to proc at least 245760 pixels to make sense
        int minChunkSize = 245760 / width;

        pool->ForRange<F>(0, height, std::forward<F>(lamb), numThreads, minChunkSize);

    }

#endif

};


#endif





