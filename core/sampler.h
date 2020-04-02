#pragma once
#include "geometry.h"
#include <vector>
#include "camera.h"
#include "rng.h"

namespace AIR
{
	//该sampler可以理解成一个pixel下的一个sampler
	//pixel
	class Sampler
	{
	public:
		Sampler(int64_t samplersPerPixel) : samplesPerPixel(samplersPerPixel)
		{

		}

		//返回当前sample + sampleNum的sample的序号
		//virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;

		//
		virtual void StartPixel(const Point2i &p);
		//获得一个1D的随机[0,1)的变量
		virtual Float Get1D() = 0;
		//获得一个2D的随机[0,1)的变量
		virtual Point2f Get2D() = 0;
		CameraSample GetCameraSample(const Point2i &pRaster);

		//要搞清楚这个n是什么意思
		//一般是表示有多少个要采样的对象。
		//例如场景上有多个light，那么Request12DArray就要Request light的数量的次数，
		//每个light有一个nSamples的成员变量，
		//那么每次采样针对light采样nSamples次，总共采样samplesPerPixel * nSamples的次数
		void Request1DArray(int n);
		void Request2DArray(int n);
		virtual int RoundCount(int n) const { return n; }
		const Float *Get1DArray(int n);
		const Point2f *Get2DArray(int n);
		//同一像素下的下一个sample
		virtual bool StartNextSample();
		//virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
		virtual bool SetSampleNumber(int64_t sampleNum);

		int64_t CurrentSampleNumber() const 
		{ 
			return currentPixelSampleIndex; 
		}
	public:
		//每个pixel有多少个samples
		const int64_t samplesPerPixel;

	protected:
		// Sampler Protected Data
		Point2i currentPixel;   //当前处理的像素
		int64_t currentPixelSampleIndex;    //当前像素的索引值，不会超过samplesPerPixel
		std::vector<int> samples1DArraySizes, samples2DArraySizes;
		std::vector<std::vector<Float>> sampleArray1D;
		std::vector<std::vector<Point2f>> sampleArray2D;

	private:
		// Sampler Private Data
		//同一个sample下的维度偏移，例如Get1DArray(n)后偏移n
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