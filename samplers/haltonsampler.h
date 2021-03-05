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

		//����ĳ��ά�ȵĵ�index������
		//@param index �������
		//@param dimension �ڼ���ά��
		//return �������
		Float SampleDimension(int64_t index, int dimension) const;

		std::unique_ptr<Sampler> Clone(int seed);
	protected:
	private:
		//���������base����
		//������ǰ�������ģ�
		//   2     3         5 ���� prime base
		//[0,1,0,1,2,0,1,2,3,4,����] before permutation
		//[1,0,0,2,1,2,0,4,3,1,����] after  permutation
		static std::vector<uint16_t> radicalInversePermutations;

		//������0-1֮�䣬��GlobalSampler������image�ڲ���
	    //baseScale��scale���ٱ���x,y����ֱ���2^j��3^k��
		//baseExponents�����2,3��ָ��j��k
		Point2i baseScales, baseExponents;

		//ͬһ�����������һ��sample��radicalInversePermutations�еļ��
		int sampleStride;
		int multInverse[2];
		//��ǰ���ڴ��������
		mutable Point2i pixelForOffset = Point2i(std::numeric_limits<int>::max(),
			std::numeric_limits<int>::max());

		//��ǰ���ڴ������������
		mutable int64_t offsetForCurrentPixel;

		//����ĳ��ά�ȵ��������ʼλ��
		const uint16_t* PermutationForDimension(int dim) const {
			if (dim >= PrimeTableSize)
				Log::Error("HaltonSampler can only sample {}{} "
					"dimensions.", PrimeTableSize);
			return &radicalInversePermutations[PrimeSums[dim]];
		}
	};
}
