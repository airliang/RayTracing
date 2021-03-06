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
			, currentPixelSampleIndex(0), array1DOffset(0), array2DOffset(0)
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
		virtual std::unique_ptr<Sampler> Clone(int seed) = 0;
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

	class GlobalSampler : public Sampler
	{
	public:
		GlobalSampler(int64_t samplesPerPixel) : Sampler(samplesPerPixel) 
		{
		}
		void StartPixel(const Point2i&);
		bool SetSampleNumber(int64_t sampleNum);
		bool StartNextSample();
		Float Get1D();
		Point2f Get2D();

		//获得当前像素对应的sample index
		//假如image大小是2 x 3，
		//当前像素是(1,0)，要得到当前像素的第0个sample，
		//调用的是GetIndexForSample(0) = 1 
		//        GetIndexForSample(1) = 7 
		//@param sampleNum  当前像素的第sampleNum个sample
		//@return   当前像素对应的sample的global index
		virtual int64_t GetIndexForSample(int64_t sampleNum) const = 0;

		//获得第index个sample中的第dimension个 sample的值
		//由于sample的值必须是0 - 1中，
		//所以返回的sample必须也是0 - 1之间，但由于sample在pixelcoordinate里，
		//返回值要减去pixel的整数
		//例如2 x 3的image，
		//halton生成的第4个sample的pixelcoordinate是(0.25, 1.33333)，
		//那SampleDimension(4, 1) = 1.33333 - 1 = 0.33333
		//@param  index  当前像素对应的sample的global index
		//@param  dimension 当前像素的sample的维度
		//@return 样本的值[0,1)
		virtual Float SampleDimension(int64_t index, int dimension) const = 0;
	protected:
	private:
		//当前样本的维度
		int dimension;

		//当前像素的当前样本在global的index
		//
		int64_t intervalSampleIndex;
		//这里为何=5？
		static const int arrayStartDim = 5;

		//样本结束的维度，注意，pbrbook用dimension来表示长度
		//实际是样本的个数
		int arrayEndDim;
	};
}