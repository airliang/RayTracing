#include "stat.h"

namespace AIR
{

    thread_local uint64_t ProfilerState;
std::vector<std::function<void(StatsAccumulator&)>>* StatRegisterer::funcs;
static StatsAccumulator statsAccumulator;

void ReportThreadStats() 
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    StatRegisterer::CallCallbacks(statsAccumulator);
}

void StatRegisterer::CallCallbacks(StatsAccumulator& accum) 
{
	for (auto func : *funcs) 
        func(accum);
}

}
