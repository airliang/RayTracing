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

	//ѭ��������ķ�װ�����ڼ��ú������߳�ִ��״̬
	//֧��1D��2D�ĺ���
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
		//1Dѭ��������
		std::function<void(int)> func1D;
		//�����߳�index
		const int64_t maxIndex;
		const int chunkSize, profilerState;
		int64_t nextIndex = 0;

		//activeWorkers records how many worker threads are currently running iterations of the loop. 
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;
	};

	//����ִ��һ��ѭ���庯��
	//func ��ѭ������ĺ���
	//count ѭ���Ĵ���
	//chunkSize һ���̴߳���func�Ĵ���
	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize);

	//���cpu��core����
	int NumSystemCores();
}
