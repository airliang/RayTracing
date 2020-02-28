#pragma once
#include "../RayTracing.h"
#include "mathdef.h"
#include <atomic>
#include <functional>

namespace AIR
{
	class AtomicFloat
	{

	public:
		explicit AtomicFloat(Float v = 0)
		{
			bits = FloatToBits(v);
		}
		
		operator Float() const
		{
			return BitsToFloat(bits);
		}

		Float operator = (Float v)
		{
			bits = FloatToBits(v);
			return v;
		}

		void Add(Float v) 
		{
			uint32_t oldBits = bits;
			uint32_t newBits;
			do {
				newBits = FloatToBits(BitsToFloat(oldBits) + v);
			} while (!bits.compare_exchange_weak(oldBits, newBits));
		}

	private:
		std::atomic<uint32_t> bits;
	};

	//循环函数体的封装，用于检测该函数的线程执行状态
	//支持1D和2D的函数
	class ParallelForLoop
	{
	public:
		ParallelForLoop(std::function<void(int)> func1D,
			int64_t maxIndex, int chunkSize, int profilerState)
			: func1D(std::move(func1D)), maxIndex(maxIndex),
			chunkSize(chunkSize), profilerState(profilerState) { }

		bool Finished() const 
		{
			return nextIndex >= maxIndex &&
				activeWorkers == 0;
		}
	private:
		//1D循环函数体
		std::function<void(int)> func1D;
		//最大的线程index
		const int64_t maxIndex;
		const int chunkSize, profilerState;
		int64_t nextIndex = 0;

		//activeWorkers records how many worker threads are currently running iterations of the loop. 
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;
	};

	//并行执行一段循环体函数
	//func 在循环体里的函数
	//count 循环的次数
	//chunkSize 一条线程处理func的次数
	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize);

	//获得cpu的core数量
	int NumSystemCores();
}
