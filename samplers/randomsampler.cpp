#include "randomsampler.h"

namespace AIR
{
	RandomSampler::RandomSampler(int ns, int seed) : Sampler(ns), rng(seed) {}

	Float RandomSampler::Get1D() {
		//ProfilePhase _(Prof::GetSample);
		//CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		return rng.UniformFloat();
	}

	Point2f RandomSampler::Get2D() {
		//ProfilePhase _(Prof::GetSample);
		//CHECK_LT(currentPixelSampleIndex, samplesPerPixel);
		return { rng.UniformFloat(), rng.UniformFloat() };
	}

	std::unique_ptr<Sampler> RandomSampler::Clone(int seed) {
		RandomSampler* rs = new RandomSampler(*this);
		rs->rng.SetSequence(seed);
		return std::unique_ptr<Sampler>(rs);
	}

	void RandomSampler::StartPixel(const Point2i& p) {
		//ProfilePhase _(Prof::StartPixel);
		for (size_t i = 0; i < sampleArray1D.size(); ++i)
			for (size_t j = 0; j < sampleArray1D[i].size(); ++j)
				sampleArray1D[i][j] = rng.UniformFloat();

		for (size_t i = 0; i < sampleArray2D.size(); ++i)
			for (size_t j = 0; j < sampleArray2D[i].size(); ++j)
				sampleArray2D[i][j] = { rng.UniformFloat(), rng.UniformFloat() };
		Sampler::StartPixel(p);
	}
}
