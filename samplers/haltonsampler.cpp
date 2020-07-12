#include "haltonsampler.h"

namespace AIR
{
	static constexpr int kMaxResolution = 128;

	std::vector<uint16_t> HaltonSampler::radicalInversePermutations;

	HaltonSampler::HaltonSampler(int samplesPerPixel, 
		const Bounds2i& sampleBounds) : GlobalSampler(samplesPerPixel),
		offsetForCurrentPixel(0)
	{
		if (radicalInversePermutations.size() == 0) 
		{
			RNG rng;
			radicalInversePermutations = ComputeRadicalInversePermutations(rng);
		}


		// Find radical inverse base scales and exponents that cover sampling area
		Vector2i res = sampleBounds.pMax - sampleBounds.pMin;
		for (int i = 0; i < 2; ++i) 
		{
			int base = (i == 0) ? 2 : 3;
			int scale = 1, exp = 0;
			while (scale < std::min(res[i], kMaxResolution)) 
			{
				scale *= base;
				++exp;
			}
			baseScales[i] = scale;
			baseExponents[i] = exp;
		}
		//整个图像总共产生了baseScales[0] * baseScales[1] * samplesPerPixel这么多个sample point
		sampleStride = baseScales[0] * baseScales[1];
	}

	int64_t HaltonSampler::GetIndexForSample(int64_t sampleNum) const
	{
		if (pixelForOffset != currentPixel)
		{
			offsetForCurrentPixel = 0;
			if (sampleStride > 1)
			{
				offsetForCurrentPixel %= sampleStride;
			}
		}
		return offsetForCurrentPixel + sampleNum * sampleStride;
	}

	Float HaltonSampler::SampleDimension(int64_t index, int dim) const {
		//if (sampleAtPixelCenter && (dim == 0 || dim == 1)) return 0.5f;
		if (dim == 0)
			return RadicalInverse(dim, index >> baseExponents[0]);
		else if (dim == 1)
			return RadicalInverse(dim, index / baseScales[1]);
		else
			return ScrambledRadicalInverse(dim, index,
				PermutationForDimension(dim));
	}
}
