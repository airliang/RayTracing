#pragma once
#include "parallelism.h"
#include <thread>
#include <vector>

namespace AIR
{
	static std::vector<std::thread> threads;

	int NumSystemCores()
	{
		return std::max(1u, std::thread::hardware_concurrency());
	}

	void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize)
	{

	}
}
