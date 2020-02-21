#pragma once
#include "geometry.h"
#include <vector>
#include "camera.h"
#include "rng.h"

namespace AIR
{
	class Sampler
	{
	public:
		Sampler(int64_t samplersPerPixel) : samplesPerPixel(samplersPerPixel)
		{

		}

		//���ص�ǰsample + sampleNum��sample�����
		//virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;

		//
		virtual void StartPixel(const Point2i &p);
		virtual Float Get1D() = 0;
		virtual Point2f Get2D() = 0;
		CameraSample GetCameraSample(const Point2i &pRaster);
		void Request1DArray(int n);
		void Request2DArray(int n);
		virtual int RoundCount(int n) const { return n; }
		const Float *Get1DArray(int n);
		const Point2f *Get2DArray(int n);
		//ͬһ�����µ���һ��sample
		virtual bool StartNextSample();
		//virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
		virtual bool SetSampleNumber(int64_t sampleNum);

		int64_t CurrentSampleNumber() const 
		{ 
			return currentPixelSampleIndex; 
		}
	public:
		const int64_t samplesPerPixel;

	protected:
		// Sampler Protected Data
		Point2i currentPixel;   //��ǰ����������
		int64_t currentPixelSampleIndex;    //��ǰ���ص�����ֵ�����ᳬ��samplesPerPixel
		std::vector<int> samples1DArraySizes, samples2DArraySizes;
		std::vector<std::vector<Float>> sampleArray1D;
		std::vector<std::vector<Point2f>> sampleArray2D;

	private:
		// Sampler Private Data
		//ͬһ��sample�µ�ά��ƫ�ƣ�����Get1DArray(n)��ƫ��n
		size_t array1DOffset, array2DOffset;
	};


	class PixelSampler : public Sampler
	{
	public:
		PixelSampler(int64_t samplesPerPixel, int nSampledDimensions);
		bool StartNextSample();
		bool SetSampleNumber(int64_t);
		Float Get1D();
		Point2f Get2D();

	protected:
		// PixelSampler Protected Data
		std::vector<std::vector<Float>> samples1D;
		std::vector<std::vector<Point2f>> samples2D;
		int current1DDimension = 0, current2DDimension = 0;
		RNG rng;
	};
}