#include "stat.h"

namespace AIR
{
static StatsAccumulator statsAccumulator;

void ReportThreadStats() 
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    StatRegisterer::CallCallbacks(statsAccumulator);
}
}
