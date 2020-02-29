#pragma once
#include "../RayTracing.h"
#include "geometry.h"
#include <atomic>
#include <functional>
#include <mutex>

namespace AIR
{

	// Simple one-use barrier; ensures that multiple threads all reach a
// particular point of execution before allowing any of them to proceed
// past it.
//
// Note: this should be heap allocated and managed with a shared_ptr, where
// all threads that use it are passed the shared_ptr. This ensures that
// memory for the Barrier won't be freed until all threads have
// successfully cleared it.
	//中文解释：该类的目的是等待所有线程运行到一段代码点
	class Barrier {
	public:
		Barrier(int count) : count(count) 
		{ 
			//CHECK_GT(count, 0); 
		}
		~Barrier() 
		{ 
			//CHECK_EQ(count, 0); 
		}
		void Wait();

	private:
		std::mutex mutex;
		std::condition_variable cv;

		//线程数量（包括主线程）
		int count;
	};

	//循环函数体的封装，每执行一次ParallelFor，生成一个ParallelForLoop对象
	//一个ParallelForLoop处理ParallelFor函数里的indexStart到indexEnd的次数的func
	//支持1D和2D的函数
	//
	class ParallelForLoop
	{
	public:
		ParallelForLoop(std::function<void(int)> func1D,
			int64_t maxIndex, int chunkSize, int profilerState)
			: func1D(std::move(func1D)), maxIndex(maxIndex),
			chunkSize(chunkSize), profilerState(profilerState) { }

		ParallelForLoop(const std::function<void(Point2i)>& f, const Point2i& count,
			uint64_t profilerState)
			: func2D(f),
			maxIndex(count.x * count.y),
			chunkSize(1),
			profilerState(profilerState) 
		{
			nX = count.x;
		}

		bool Finished() const 
		{
			return nextIndex >= maxIndex &&
				activeWorkers == 0;
		}
	public:
		//1D循环函数体
		std::function<void(int)> func1D;
		std::function<void(Point2i)> func2D;
		//最大的循环的index
		const int64_t maxIndex;

		//chunkSize该任务要执行的次数
		const int chunkSize, profilerState;

		//tracks the next loop index to be executed.
		//可以理解为下一个任务执行的循环的序号
		//例如一个循环1024次，maxIndex = 1024，chunkSize = 32
		//假如该任务的起始index是64，
		//那么nextIndex就是64 + chunkSize = 96
		int64_t nextIndex = 0;

		//activeWorkers records how many worker threads are currently running iterations of the loop. 
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;

		int nX = -1;
	};

	//并行执行一段循环体函数
	//func 在循环体里的函数
	//count 循环的次数
	//chunkSize 一条线程处理的【最多】循环次数
	//例如循环1024次，每条线程处理【最多】32次，那么参数是
	//count = 1024, chunkSize = 32
	//注意：该函数是在主进程里执行的
	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize);

	int MaxThreadIndex();
	//获得cpu的core数量
	int NumSystemCores();
	void ParallelInit();
	void ParallelCleanup();
}
