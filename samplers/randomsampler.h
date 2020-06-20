#pragma once

#include "sampler.h"

namespace AIR
{
	class RandomSampler : public Sampler
	{
	public:
		RandomSampler(int ns, int seed = 0);
		void StartPixel(const Point2i& p);
		Float Get1D();
		Point2f Get2D();
		std::unique_ptr<Sampler> Clone(int seed);

	private:
		RNG rng;
	};
}
