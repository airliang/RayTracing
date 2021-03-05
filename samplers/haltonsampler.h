#pragma once

#include "sampler.h"
#include "log.h"
#include "lowdiscrepancy.h"

namespace AIR
{
	class HaltonSampler : public GlobalSampler
	{
	public:
		HaltonSampler(int samplesPerPixel, const Bounds2i& sampleBounds);

		int64_t GetIndexForSample(int64_t sampleNum) const;

		//采样某个维度的第index个样本
		//@param index 样本序号
		//@param dimension 第几个维度
		//return 随机变量
		Float SampleDimension(int64_t index, int dimension) const;

		std::unique_ptr<Sampler> Clone(int seed);
	protected:
	private:
		//重新排序的base数组
		//不排序前是这样的：
		//   2     3         5 …… prime base
		//[0,1,0,1,2,0,1,2,3,4,……] before permutation
		//[1,0,0,2,1,2,0,4,3,1,……] after  permutation
		static std::vector<uint16_t> radicalInversePermutations;

		//样本是0-1之间，但GlobalSampler是整个image内采样
	    //baseScale是scale多少倍，x,y方向分别是2^j和3^k倍
		//baseExponents存的是2,3的指数j和k
		Point2i baseScales, baseExponents;

		//同一个像素里的下一个sample在radicalInversePermutations中的间隔
		int sampleStride;
		int multInverse[2];
		//当前正在处理的像素
		mutable Point2i pixelForOffset = Point2i(std::numeric_limits<int>::max(),
			std::numeric_limits<int>::max());

		//当前正在处理的像素索引
		mutable int64_t offsetForCurrentPixel;

		//返回某个维度的数组的起始位置
		const uint16_t* PermutationForDimension(int dim) const {
			if (dim >= PrimeTableSize)
				Log::Error("HaltonSampler can only sample {}{} "
					"dimensions.", PrimeTableSize);
			return &radicalInversePermutations[PrimeSums[dim]];
		}
	};
}
