#pragma once
#include "parallelism.h"
#include "stat.h"
#include <thread>
#include <vector>
#include "log.h"

namespace AIR
{
	static std::vector<std::thread> threads;

	static bool shutdownThreads = false;
	class ParallelForLoop;

	//任务队列
	static ParallelForLoop* workList = nullptr;
	//任务队列的锁
	static std::mutex workListMutex;
    //
    static std::condition_variable workListCondition;

    static std::atomic<bool> reportWorkerStats{false};
// Number of workers that still need to report their stats.
static std::atomic<int> reporterCount;
static std::condition_variable reportDoneCondition;
static std::mutex reportDoneMutex;

    thread_local int ThreadIndex;

    static void workerThreadFunc(int tIndex, std::shared_ptr<Barrier> barrier);

	int NumSystemCores()
	{
		return std::max(1u, std::thread::hardware_concurrency());
	}

    int MaxThreadIndex() 
    {
        //return g_globalOptions.nThreads == 0 ? NumSystemCores() : g_globalOptions.nThreads;
        //pbrt有个配置使用线程的数量，这里先使用系统cpu数量
        return 1;   //先用1条线程测试
        return NumSystemCores();
    }

    void ParallelInit() {
        //CHECK_EQ(threads.size(), 0);
        int nThreads = MaxThreadIndex();
        ThreadIndex = 0;

        // Create a barrier so that we can be sure all worker threads get past
        // their call to ProfilerWorkerThreadInit() before we return from this
        // function.  In turn, we can be sure that the profiling system isn't
        // started until after all worker threads have done that.
        std::shared_ptr<Barrier> barrier = std::make_shared<Barrier>(nThreads);

        // Launch one fewer worker thread than the total number we want doing
        // work, since the main thread helps out, too.
        for (int i = 0; i < nThreads - 1; ++i)
            threads.push_back(std::thread(workerThreadFunc, i + 1, barrier));

        barrier->Wait();
    }

    void ParallelCleanup() 
    {
        if (threads.empty()) return;

        {
            std::lock_guard<std::mutex> lock(workListMutex);
            shutdownThreads = true;
            workListCondition.notify_all();
        }

        for (std::thread& thread : threads) thread.join();
        threads.erase(threads.begin(), threads.end());
        shutdownThreads = false;
    }

    void Barrier::Wait() 
    {
        std::unique_lock<std::mutex> lock(mutex);
        //CHECK_GT(count, 0);
        if (--count == 0)
            // This is the last thread to reach the barrier; wake up all of the
            // other ones before exiting.
            cv.notify_all();
        else
            // Otherwise there are still threads that haven't reached it. Give
            // up the lock and wait to be notified.
            cv.wait(lock, [this] { return count == 0; });
    }

    static void workerThreadFunc(int tIndex, std::shared_ptr<Barrier> barrier) 
    {
        //LOG << "Started execution in worker thread " << tIndex << std::endl;
        ThreadIndex = tIndex;

        // Give the profiler a chance to do per-thread initialization for
        // the worker thread before the profiling system actually stops running.
        //ProfilerWorkerThreadInit();

        // The main thread sets up a barrier so that it can be sure that all
        // workers have called ProfilerWorkerThreadInit() before it continues
        // (and actually starts the profiling system).
        barrier->Wait();

        // Release our reference to the Barrier so that it's freed once all of
        // the threads have cleared it.
        barrier.reset();

        std::unique_lock<std::mutex> lock(workListMutex);
        while (!shutdownThreads) 
        {
            if (reportWorkerStats) 
            {
                ReportThreadStats();
                if (--reporterCount == 0)
                    // Once all worker threads have merged their stats, wake up
                    // the main thread.
                    reportDoneCondition.notify_one();
                // Now sleep again.
                workListCondition.wait(lock);
            }
            else if (!workList) {
                // Sleep until there are more tasks to run
                workListCondition.wait(lock);
            }
            else {
                // Get work from _workList_ and run loop iterations
                ParallelForLoop& loop = *workList;

                // Run a chunk of loop iterations for _loop_

                // Find the set of loop iterations to run next
                int64_t indexStart = loop.nextIndex;
                int64_t indexEnd =
                    std::min(indexStart + loop.chunkSize, loop.maxIndex);

                // Update _loop_ to reflect iterations this thread will run
                loop.nextIndex = indexEnd;
                if (loop.nextIndex == loop.maxIndex) workList = loop.next;
                loop.activeWorkers++;

                // Run loop indices in _[indexStart, indexEnd)_
                lock.unlock();
                for (int64_t index = indexStart; index < indexEnd; ++index) {
                    //uint64_t oldState = ProfilerState;
                    //ProfilerState = loop.profilerState;
                    if (loop.func1D) 
                    {
                        loop.func1D(index);
                    }
                    // Handle other types of loops
                    else 
                    {
                        //CHECK(loop.func2D);
                        loop.func2D(Point2i(index % loop.nX, index / loop.nX));
                    }
                    //ProfilerState = oldState;
                }
                lock.lock();

                // Update _loop_ to reflect completion of iterations
                loop.activeWorkers--;
                if (loop.Finished()) 
                    workListCondition.notify_all();
            }
        }
        //LOG << "Exiting worker thread " << tIndex<<std::endl;
    }

    
	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize)
	{
		//如果处理的量很少，不需要多线程直接处理并返回
		if (threads.empty() || count < chunkSize) 
		{
			for (int64_t i = 0; i < count; ++i) 
				func(i);
			return;
		}

        ParallelForLoop loop(std::move(func), count, chunkSize, 0/*CurrentProfilerState()*/);
        workListMutex.lock();

        //把新任务放到队列的头部
        loop.next = workList;
        workList = &loop;
        workListMutex.unlock();

        // Notify worker threads of work to be done
        std::unique_lock<std::mutex> lock(workListMutex);
        workListCondition.notify_all();

        // Help out with parallel loop iterations in the current thread
        while (!loop.Finished()) {
            // Run a chunk of loop iterations for _loop_

            // Find the set of loop iterations to run next
            //例如这里循环总数是
            //任务循环开始的序号
            int64_t indexStart = loop.nextIndex;
            //任务循环结束的序号
            int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

            // Update _loop_ to reflect iterations this thread will run
            loop.nextIndex = indexEnd;
            if (loop.nextIndex == loop.maxIndex) 
                workList = loop.next;
            loop.activeWorkers++;

            // Run loop indices in _[indexStart, indexEnd)_
            lock.unlock();
            
            //主线程执行下面的函数，经过上面的lock后，indexStart会发生如下变化：
            //假如上面有其他线程争夺了一个loop，chunkSize = 32的话
            //这里的indexStart可能等于32而不是0，
            //因为indexStart = 0被另一个线程夺取并执行了
            for (int64_t index = indexStart; index < indexEnd; ++index) 
            {
                //uint64_t oldState = ProfilerState;
                //ProfilerState = loop.profilerState;
                if (loop.func1D) {
                    loop.func1D(index);
                }
                // Handle other types of loops
                else 
                {
                    //CHECK(loop.func2D);
                    loop.func2D(Point2i(index % loop.nX, index / loop.nX));
                }
                //ProfilerState = oldState;
            }
            lock.lock();

            // Update _loop_ to reflect completion of iterations
            //主线程执行完，所以要--
            loop.activeWorkers--;
        }
	}

    void ParallelFor2D(const std::function<void(Point2i)>& func, const Point2i& count)
    {
		if (threads.empty() || count.x * count.y <= 1) 
        {
			for (int y = 0; y < count.y; ++y)
				for (int x = 0; x < count.x; ++x) 
                    func(Point2i(x, y));
			return;
		}

		ParallelForLoop loop(std::move(func), count, 0/*, CurrentProfilerState()*/);
		{
			std::lock_guard<std::mutex> lock(workListMutex);
			loop.next = workList;
			workList = &loop;
		}

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

        //下面代码可以理解成在主线程执行
		// Help out with parallel loop iterations in the current thread
		while (!loop.Finished()) {
			// Run a chunk of loop iterations for _loop_

			// Find the set of loop iterations to run next
			int64_t indexStart = loop.nextIndex;
			int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);

			// Update _loop_ to reflect iterations this thread will run
			loop.nextIndex = indexEnd;
			if (loop.nextIndex == loop.maxIndex) 
                workList = loop.next;
			loop.activeWorkers++;

			// Run loop indices in _[indexStart, indexEnd)_
			lock.unlock();
			for (int64_t index = indexStart; index < indexEnd; ++index) {
				//uint64_t oldState = ProfilerState;
				//ProfilerState = loop.profilerState;
				if (loop.func1D) {
					loop.func1D(index);
				}
				// Handle other types of loops
				else {
					//CHECK(loop.func2D);
					loop.func2D(Point2i(index % loop.nX, index / loop.nX));
				}
				//ProfilerState = oldState;
			}
			lock.lock();

			// Update _loop_ to reflect completion of iterations
			loop.activeWorkers--;
		}
        
    }

    void MergeWorkerThreadStats() 
    {
        std::unique_lock<std::mutex> lock(workListMutex);
        std::unique_lock<std::mutex> doneLock(reportDoneMutex);
        // Set up state so that the worker threads will know that we would like
        // them to report their thread-specific stats when they wake up.
        reportWorkerStats = true;
        reporterCount = threads.size();

        // Wake up the worker threads.
        workListCondition.notify_all();

        // Wait for all of them to merge their stats.
        reportDoneCondition.wait(lock, []() { return reporterCount == 0; });

        reportWorkerStats = false;
    }
}
