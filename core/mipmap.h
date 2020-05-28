#pragma once

#include "spectrum.h"
#include "geometry.h"
#include "memory.h"
#include "parallelism.h"
#include "imageio.h"
namespace AIR
{
enum class ImageWrap 
{ 
    Repeat, 
    Black, 
    Clamp 
};

struct TexInfo {
    TexInfo(const std::string &f, bool dt, Float ma, ImageWrap wm, Float sc,
            bool gamma)
        : filename(f),
          doTrilinear(dt),
          maxAniso(ma),
          wrapMode(wm),
          scale(sc),
          gamma(gamma) {}
    std::string filename;   //�ļ���
    bool doTrilinear;       //�Ƿ���trilinear�����ˣ�true�ǣ�false��EWA������
    Float maxAniso;         //���������Բ�ֵ
    ImageWrap wrapMode;     //��������ģʽ
    Float scale;
    bool gamma;
    bool operator<(const TexInfo &t2) const 
    {
        if (filename != t2.filename) 
            return filename < t2.filename;
        if (doTrilinear != t2.doTrilinear) 
            return doTrilinear < t2.doTrilinear;
        if (maxAniso != t2.maxAniso) 
            return maxAniso < t2.maxAniso;
        if (scale != t2.scale) 
            return scale < t2.scale;
        if (gamma != t2.gamma) 
            return !gamma;
        return wrapMode < t2.wrapMode;
    }
};

//�ز������µ�texelֻ�ܲ����ڵ�4��texel��Ȩ��Ӱ��
struct ResampleWeight 
{
	int firstTexel;  //ԭ�����������offset
	Float weight[4]; //Ӱ���4��Ȩ��
};

template <typename T>
class Mipmap
{
public:
    Mipmap(const Point2i& res, const T* memory,
        bool doTrilinear = true, Float maxAnisotropy = 8.0f, ImageWrap wrapMode = ImageWrap::Repeat);

    T Lookup(const Point2f& st, const Vector2f& dstdx, const Vector2f& dstdy) const;

    //����mipmap��texelֵ
    //st ��������
    //width ���˵Ŀ�ȣ����ڲ���level��width�ķ�Χ��[0-1]
	//Ҳ�����ظ��ǵ����Χ
    //����mipmapʵ�ʿ����L��width������ 1/L * scale
    T Lookup(const Point2f& st, Float width = 0.f) const;

    //��������ڴ����texel��ֵ
    const T& Texel(int level, int s, int t) const;

    int Levels() const
    {
        return (int)pyramid.size();
    }

    int Width() const
    {
        return resolution[0];
    }

    int Height() const
    {
        return resolution[1];
    }
private:
    //triangle filter
    //f(x, y) = (1 - |x|)(1 - |y|)
    T Triangle(int level, const Point2f& st) const;

	Float clamp(Float v)
	{
		return Clamp(v, 0.f, Infinity);
	}
	RGBSpectrum clamp(const RGBSpectrum& v)
	{
		return v.Clamp(0.f, Infinity);
	}

	SampledSpectrum clamp(const SampledSpectrum& v)
	{
		return v.Clamp(0.f, Infinity);
	}
private:
    //determines which original texels contribute to each new texel 
    //and what the values are of the contribution weights for each new texel.
    //return ResampleWeight[] the contribution weights for each new texel,������newRes
    //ÿ��newTexel��Ӱ���4��Ȩ��ֵ
    //int oldRes ԭ���ֱ���
    //int newRes �µķֱ���
    std::unique_ptr<ResampleWeight[]> ResampleWeights(int oldRes, int newRes);
    Point2i resolution;
    const bool trilinear;
    const ImageWrap wrapMode;
    const Float maxAnisotropy;

    //mipmapÿ����ڴ�����
    std::vector<std::unique_ptr<BlockedArray<T>>> pyramid;
};


template <typename T>
Mipmap<T>::Mipmap(const Point2i& res, const T* memory, bool doTrilinear, Float maxAnisotropy, ImageWrap wrapMode) : resolution(res)
, trilinear(doTrilinear)
, maxAnisotropy(maxAnisotropy)
, wrapMode(wrapMode)
{
	//��һ������resolution resize up to power of 2
	//��Ҫ�õ�resampling���Ŵ����resample��
	std::unique_ptr<T[]> resampledImage = nullptr;
	if (!IsPowerOf2(resolution[0]) || !IsPowerOf2(resolution[0]))
	{
		Point2i resPow2(RoundUpPow2(resolution[0]), RoundUpPow2(resolution[1]));

		//t�����resampledWeight
		std::unique_ptr<ResampleWeight[]> resampleWeights = ResampleWeights(resolution[0], resPow2[0]);

		resampledImage.reset(new T(resPow2[0] * resPow2[1]));

		//���м���resampledImage��s�����zoomed texel
		ParallelFor([&](int t) {
			for (int s = 0; s < resPow2[0]; ++s)
			{
				//�������ǰ��
				int index = t * resPow2[0] + s;
				resampledImage[index] = 0.0f;

				for (int j = 0; j < 4; ++j)
				{
					int oldTexelIndex = resampleWeights[s].firstTexel + j;
					if (wrapMode == ImageWrap::Repeat)
					{
						//if (oldTexelIndex < 0)
						//	oldTexelIndex += resolution[0];
						oldTexelIndex = Mod(oldTexelIndex, resolution[0]);
					}
					else if (wrapMode == ImageWrap::Clamp)
					{
						//if (oldTexelIndex < 0)
						//    oldTexelIndex = 0;
						//else if (oldTexelIndex >= resolution[0])
						//    oldTexelIndex = resolution[0] - 1;
						oldTexelIndex = Clamp(oldTexelIndex, 0, resolution[0] - 1);
					}
					else
					{

					}
					//��������Ǵ��������Black�Ļ�������0
					if (oldTexelIndex >= 0 && oldTexelIndex < resolution[0])
						resampledImage[index] += memory[t * resolution[0] + oldTexelIndex] * resampleWeights[s].weight[j];
				}
			}
		}, resPow2[1], 16);


		//����t�����resample
		std::unique_ptr<ResampleWeight[]> tWeights = ResampleWeights(resolution[1], resPow2[1]);
		std::vector<T*> resampleBufs;
		int nThreads = MaxThreadIndex();
		for (int i = 0; i < nThreads; ++i)
			resampleBufs.push_back(new T[resPow2[1]]);
		ParallelFor([&](int s) {
			//ԭ����ͼ���Ѿ���д�룬���µ���ʱbuf������
			T* tDatas = resampleBufs[ThreadIndex];
			for (int t = 0; t < resPow2[1]; ++t)
			{

				tDatas[t] = 0.0f;

				for (int j = 0; j < 4; ++j)
				{
					//�ڼ���
					int oldTexelIndex = tWeights[t].firstTexel + j;
					if (wrapMode == ImageWrap::Repeat)
					{
						oldTexelIndex = Mod(oldTexelIndex, resolution[1]);
					}
					else if (wrapMode == ImageWrap::Clamp)
					{
						oldTexelIndex = Clamp(oldTexelIndex, 0, resolution[1] - 1);
					}
					else
					{

					}
					//��������Ǵ��������Black�Ļ�������0
					//����resampledImage�ڴ���s����ʱ�Ѿ�ȫ������������أ�����ȡȨ�ز���ȡԭͼ�񣬶����Ѵ������resampledImage
					if (oldTexelIndex >= 0 && oldTexelIndex < resolution[1])
						tDatas[t] += resampledImage[oldTexelIndex * resPow2[0] + s] * tWeights[t].weight[j];
				}
			}

			for (int t = 0; t < resPow2[1]; ++t)
			{
				int index = t * resPow2[0] + s;
				resampledImage[index] = clamp(tDatas[t]);
			}
		}, resPow2[0], 16);

		//delete ֮ǰnew������
		for (auto ptr : resampleBufs)
			delete[] ptr;
		resolution = resPow2;
	}
	else
	{

	}

	//�������mipmap��level
	int levels = Log2Int(std::max(resolution[0], resolution[1]));
	pyramid.resize(levels);

	//��0���mipmap�ڴ�����
	pyramid[0].reset(new BlockedArray<T>(resolution[0], resolution[1],
		resampledImage ? resampledImage.get() : memory));

	for (int i = 1; i < levels; ++i)
	{
		int sWidth = std::max(1, pyramid[i - 1]->uSize() / 2);
		int tHeight = std::max(1, pyramid[i - 1]->vSize() / 2);
		pyramid[i].reset(new BlockedArray<T>(sWidth, tHeight));

		//box filter to caculate the current level from the last level
		ParallelFor(
			[&](int t) {
			for (int s = 0; s < sWidth; ++s)
			{
				(*pyramid[i])(s, t) = 0.25f * 
					(Texel(i - 1, 2 * s, 2 * t)
					+ Texel(i - 1, 2 * s + 1, 2 * t) 
					+ Texel(i - 1, 2 * s, 2 * t + 1) 
					+ Texel(i - 1, 2 * s + 1, 2 * t + 1));
			}
		}, tHeight, 16);

		//char buffer[33];
		//itoa(i, buffer, 10);
		//std::string temp = buffer;
		//temp += ".png";
		//ImageIO::WriteImage(std::string("E:\\RayTracing\\resources\\images\\mipmap_") + temp, (Float*)&((*pyramid[i])(0, 0)), 
		//	Bounds2i(Vector2i::zero, Vector2i(sWidth, tHeight)), Point2i(sWidth, tHeight));
	}
}



template <typename T>
std::unique_ptr<ResampleWeight[]> Mipmap<T>::ResampleWeights(int oldRes, int newRes)
{
	std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);
	Float filterRadius = 2.0f;

	for (int i = 0; i < newRes; ++i)
	{
		//ȡԭ���ص�centerλ��
		Float center = (i + 0.5f) * oldRes / newRes;

		//Ӱ�����ĵ�һ������
		wt[i].firstTexel = std::floor(center - filterRadius + 0.5f);
		for (int j = 0; j < 4; ++j)
		{
			//����Ӱ���������ص�pos
			Float pos = wt[i].firstTexel + j + 0.5f;

			//pos - center������fitler�е�λ��
			//Lanczos�Ľ��ͣ�
			wt[i].weight[j] = Lanczos((pos - center) / filterRadius);
		}

		//��4��weight��һ����4������������1
		Float invWeightsum = 1.0f / (wt[i].weight[0] + wt[i].weight[1] + wt[i].weight[2] + wt[i].weight[3]);

		for (int j = 0; j < 4; ++j)
		{
			wt[i].weight[j] *= invWeightsum;
		}
	}

	return wt;
}

template <typename T>
const T& Mipmap<T>::Texel(int level, int s, int t) const
{
	const BlockedArray<T>& l = *pyramid[level];

	switch (wrapMode)
	{
	case ImageWrap::Repeat:
		s = Mod(s, l.uSize());
		t = Mod(t, l.vSize());
		break;
	case ImageWrap::Clamp:
		s = Clamp(s, 0, l.uSize());
		t = Clamp(t, 0, l.vSize());
		break;
	case ImageWrap::Black:
	{
		if (s < 0 || s >= (int)l.uSize() || t < 0 || t >= (int)l.vSize())
		{
			static const T black = 0.0f;
			return black;
		}
	}
	default:
		break;
	}

	return l(s, t);
}

template <typename T>
T Mipmap<T>::Lookup(const Point2f& st, const Vector2f& dstdx, const Vector2f& dstdy) const
{
	//����dstdx����width
	if (trilinear)
	{
		Float width = std::max(std::max(std::abs(dstdx[0]), std::abs(dstdx[1])),
			std::max(std::abs(dstdy[0]), std::abs(dstdy[1])));
		return Lookup(st, 2 * width);
	}

	//ewa����
	return 0.0f;
}

template <typename T>
T Mipmap<T>::Lookup(const Point2f& st, Float width /* = 0.f */) const
{
	//1 / w = 2 ^ (nLeveL - 1 - l)
	//nLeveL - 1 - l = Log2(1 / w) = -Log2(w)
	//l = nLeveL - 1 + Log2(w)
	Float level = Levels() - 1 + Log2(std::max(width, 0.0f));

	if (trilinear)
	{
		if (level < 0)
		{
			//��0�㣬ֱ�ӷ��������texel
			return Triangle(0, st);
		}
		else if (level >= Levels() - 1)
		{
			//���һ�㣬ֻ��һ��texelֵ
			return Texel(Levels() - 1, 0, 0);
		}
		else
		{
			int nLevel = std::floor(level);
			Float delta = level - nLevel;
			return Lerp(delta, Triangle(nLevel, st), Triangle(nLevel + 1, st));
		}
	}
	else
	{
		return 0.0f;
	}
}


template <typename T>
T Mipmap<T>::Triangle(int level, const Point2f& st) const
{
	//��level clampһ��
	level = Clamp(level, 0, Levels());

	// s0,t1-------s1,t1
	//
	// --ds----s,t
	//          |
	//         dt
	// s0,t0----|--s1,t0
	Float s = st[0] * pyramid[level]->uSize() - 0.5f;
	Float t = st[1] * pyramid[level]->vSize() - 0.5f;
	int s0 = std::floor(s);
	int s1 = s0 + 1;
	int t0 = std::floor(t);
	int t1 = t0 + 1;

	Float ds = s - s0;
	Float dt = t - t0;

	return (1.0f - ds) * (1.0f - dt) * Texel(level, s0, t0)
		+ ds * (1.0f - dt) * Texel(level, s1, t0)
		+ (1.0f - ds) * dt * Texel(level, s0, t1)
		+ ds * dt * Texel(level, s1, t1);
}

}
