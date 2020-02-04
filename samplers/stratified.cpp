#include "stratified.h"

namespace AIR
{
	void StratifiedSampler::StartPixel(const Point2i &p) 
	{
		//ProfilePhase _(Prof::StartPixel);
		// Generate single stratified samples for the pixel
		for (size_t i = 0; i < samples1D.size(); ++i) 
		{
			StratifiedSample1D(&samples1D[i][0], xPixelSamples * yPixelSamples, rng,
				jitterSamples);
			Shuffle(&samples1D[i][0], xPixelSamples * yPixelSamples, 1, rng);
		}
		for (size_t i = 0; i < samples2D.size(); ++i) {
			StratifiedSample2D(&samples2D[i][0], xPixelSamples, yPixelSamples, rng,
				jitterSamples);
			Shuffle(&samples2D[i][0], xPixelSamples * yPixelSamples, 1, rng);
		}

		// Generate arrays of stratified samples for the pixel
		for (size_t i = 0; i < samples1DArraySizes.size(); ++i)
			for (int64_t j = 0; j < samplesPerPixel; ++j) 
			{
				int count = samples1DArraySizes[i];
				StratifiedSample1D(&sampleArray1D[i][j * count], count, rng,
					jitterSamples);
				Shuffle(&sampleArray1D[i][j * count], count, 1, rng);
			}
		for (size_t i = 0; i < samples2DArraySizes.size(); ++i)
			for (int64_t j = 0; j < samplesPerPixel; ++j) 
			{
				int count = samples2DArraySizes[i];
				LatinHypercube(&sampleArray2D[i][j * count].x, count, 2, rng);
			}
		PixelSampler::StartPixel(p);
	}

	std::unique_ptr<Sampler> StratifiedSampler::Clone(int seed) 
	{
		StratifiedSampler *ss = new StratifiedSampler(*this);
		ss->rng.SetSequence(seed);
		return std::unique_ptr<Sampler>(ss);
	}

	

	void StratifiedSample1D(Float *samp, int nSamples, RNG &rng, bool jitter) 
	{
		Float invNSamples = (Float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i) {
			Float delta = jitter ? rng.UniformFloat() : 0.5f;
			samp[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
		}
	}

	void StratifiedSample2D(Point2f *samp, int nx, int ny, RNG &rng, bool jitter) 
	{
		Float dx = (Float)1 / nx, dy = (Float)1 / ny;
		for (int y = 0; y < ny; ++y)
			for (int x = 0; x < nx; ++x) {
				Float jx = jitter ? rng.UniformFloat() : 0.5f;
				Float jy = jitter ? rng.UniformFloat() : 0.5f;
				samp->x = std::min((x + jx) * dx, OneMinusEpsilon);
				samp->y = std::min((y + jy) * dy, OneMinusEpsilon);
				++samp;
			}
	}

	void LatinHypercube(Float *samples, int nSamples, int nDim, RNG &rng) 
	{
		// Generate LHS samples along diagonal
		Float invNSamples = (Float)1 / nSamples;
		for (int i = 0; i < nSamples; ++i)
			for (int j = 0; j < nDim; ++j) {
				Float sj = (i + (rng.UniformFloat())) * invNSamples;
				samples[nDim * i + j] = std::min(sj, OneMinusEpsilon);
			}

		// Permute LHS samples in each dimension
		for (int i = 0; i < nDim; ++i) {
			for (int j = 0; j < nSamples; ++j) {
				int other = j + rng.UniformUInt32(nSamples - j);
				std::swap(samples[nDim * j + i], samples[nDim * other + i]);
			}
		}
	}
}
