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
	//���Ľ��ͣ������Ŀ���ǵȴ������߳����е�һ�δ����
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

		//�߳��������������̣߳�
		int count;
	};

	//ѭ��������ķ�װ��ÿִ��һ��ParallelFor������һ��ParallelForLoop����
	//һ��ParallelForLoop����ParallelFor�������indexStart��indexEnd�Ĵ�����func
	//֧��1D��2D�ĺ���
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
			maxIndex((int)count.x * count.y),
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
		//1Dѭ��������
		std::function<void(int)> func1D;
		std::function<void(Point2i)> func2D;
		//����ѭ����index
		const int64_t maxIndex;

		//chunkSize������Ҫִ�еĴ���
		const int chunkSize, profilerState;

		//tracks the next loop index to be executed.
		//�������Ϊ��һ������ִ�е�ѭ�������
		//����һ��ѭ��1024�Σ�maxIndex = 1024��chunkSize = 32
		//������������ʼindex��64��
		//��ônextIndex����64 + chunkSize = 96
		int64_t nextIndex = 0;

		//activeWorkers records how many worker threads are currently running iterations of the loop. 
		int activeWorkers = 0;
		ParallelForLoop* next = nullptr;

		int nX = -1;
	};

	//����ִ��һ��ѭ���庯��
	//func ��ѭ������ĺ���
	//count ѭ���Ĵ���
	//chunkSize һ���̴߳���ġ���ࡿѭ������
	//����ѭ��1024�Σ�ÿ���̴߳�����ࡿ32�Σ���ô������
	//count = 1024, chunkSize = 32
	//ע�⣺�ú���������������ִ�е�
	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize);

	//����ִ��2Dѭ������
	//func ��ѭ������ĺ���
	//count tile�Ķ�ά����
	//���� 700 x 700��image tileSize = 16
	//count.x = (700 + 16 - 1) / 16
	//count.y = (700 + 16 - 1) / 16
	//����û��chunkSize���ں����о���ִ��һ��tileSize * tileSize��2Dѭ��
	void ParallelFor2D(const std::function<void(Point2i)>& func, const Point2i& count);

	int MaxThreadIndex();
	//���cpu��core����
	int NumSystemCores();
	void ParallelInit();
	void ParallelCleanup();
}
