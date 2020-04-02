#pragma once
#include "geometry.h"
#include <vector>
#include "camera.h"
#include "rng.h"

namespace AIR
{
	//��sampler��������һ��pixel�µ�һ��sampler
	//pixel
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
		//���һ��1D�����[0,1)�ı���
		virtual Float Get1D() = 0;
		//���һ��2D�����[0,1)�ı���
		virtual Point2f Get2D() = 0;
		CameraSample GetCameraSample(const Point2i &pRaster);

		//Ҫ��������n��ʲô��˼
		//һ���Ǳ�ʾ�ж��ٸ�Ҫ�����Ķ���
		//���糡�����ж��light����ôRequest12DArray��ҪRequest light�������Ĵ�����
		//ÿ��light��һ��nSamples�ĳ�Ա������
		//��ôÿ�β������light����nSamples�Σ��ܹ�����samplesPerPixel * nSamples�Ĵ���
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
		//ÿ��pixel�ж��ٸ�samples
		const int64_t samplesPerPixel;

	protected:
		// Sampler Protected Data
		Point2i currentPixel;   //��ǰ���������
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