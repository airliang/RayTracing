#pragma once
#include "sampler.h"

namespace AIR
{
	class StratifiedSampler : public PixelSampler
	{
	public:
		// StratifiedSampler Public Methods
		StratifiedSampler(int xPixelSamples, int yPixelSamples, bool jitterSamples,
			int nSampledDimensions)
			: PixelSampler(xPixelSamples * yPixelSamples, nSampledDimensions),
			xPixelSamples(xPixelSamples),
			yPixelSamples(yPixelSamples),
			jitterSamples(jitterSamples) {}
		void StartPixel(const Point2i &);
		std::unique_ptr<Sampler> Clone(int seed);

	private:
		// StratifiedSampler Private Data
		const int xPixelSamples, yPixelSamples;

		//false就是平均，样本点在strata的中心
		const bool jitterSamples;
	};

	void StratifiedSample1D(Float *samples, int nsamples, RNG &rng,
		bool jitter = true);
	void StratifiedSample2D(Point2f *samples, int nx, int ny, RNG &rng,
		bool jitter = true);
	void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng);

	// Sampling Inline Functions
	template <typename T>
	void Shuffle(T *samp, int count, int nDimensions, RNG &rng) {
		for (int i = 0; i < count; ++i) {
			int other = i + rng.UniformUInt32(count - i);
			for (int j = 0; j < nDimensions; ++j)
				std::swap(samp[nDimensions * i + j], samp[nDimensions * other + j]);
		}
	}
}
