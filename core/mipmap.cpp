#include "mipmap.h"

namespace AIR
{
template <typename T>
Mipmap<T>::Mipmap(const Point2i& res, const T* memory, bool doTrilinear, Float maxAnisotropy, ImageWrap wrapMode) : resolution(res)
    , trilinear(doTrilinear)
    , maxAnisotropy(maxAnisotropy)
    , wrapMode(wrapMode)
{
    //第一步，把resolution resize up to power of 2
    //需要用到resampling（放大就是resample）
    std::unique_ptr<T[]> resampledImage = nullptr;
    if (!IsPowerOf2(resolution[0]) || !IsPowerOf2(resolution[0]))
    {
        Point2i resPow2(RoundUpPow2(resolution[0]), RoundUpPow2(resolution[1]));

        //t方向的resampledWeight
        std::unique_ptr<ResampleWeight[]> resampleWeights = ResampleWeight(resolution[0], resPow2[0]);

        resampledImage.reset(resPow2[0] * resPow2[1]);

        //并行计算resampledImage中s方向的zoomed texel
        ParallelFor([&](int t) {
            for (int s = 0; s < resPow2[0]; ++s)
            {
                //计算出当前的
                int index = t * resPow2[0] + s;
                resampledImage[index] = 0.0f;

                for (int j = 0; j < 4; ++j)
                {
                    int oldTexelIndex = resampleWeights[s].firstTexel + j;
                    if (wrapMode == ImageWrap::Repeat)
                    {
                        if (oldTexelIndex < 0)
                            oldTexelIndex += resolution[0];
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
                    //这个条件是代表如果是Black的话，就是0
                    if (oldTexelIndex >= 0 && oldTexelIndex < resolution[0])
                        resampledImage[index] += memory[t * resolution[0] + oldTexelIndex] * tWeights[s].weight[j];
                }
            }
        }, resPow2[1], 16);


        //处理t方向的resample
        std::unique_ptr<ResampleWeight[]> tWeights = ResampleWeight(resolution[1], resPow2[1]);
		std::vector<T*> resampleBufs;
		int nThreads = MaxThreadIndex();
		for (int i = 0; i < nThreads; ++i)
			resampleBufs.push_back(new T[resPow2[1]]);
		ParallelFor([&](int s) {
            //原来的图像已经被写入，用新的临时buf来代替
            T* tDatas = resampleBufs[ThreadIndex];
			for (int t = 0; t < resPow2[1]; ++t)
			{
				
                tDatas[t] = 0.0f;

				for (int j = 0; j < 4; ++j)
				{
                    //第几行
					int row = tWeights[t].firstTexel + j;
					if (wrapMode == ImageWrap::Repeat)
					{
						if (oldTexelIndex < 0)
							oldTexelIndex += resolution[1];
					}
					else if (wrapMode == ImageWrap::Clamp)
					{
						oldTexelIndex = Clamp(row, 0, resolution[1] - 1);
					}
					else
					{

					}
					//这个条件是代表如果是Black的话，就是0
                    //由于resampledImage在处理s方向时已经全部有了想关像素，这里取权重不是取原图像，而是已处理过的resampledImage
					if (row >= 0 && row < resolution[1])
                        tDatas[t] += resampledImage[row * resPow2[0] + s] * tWeights[t].weight[j];
				}
			}

            for (int t = 0; t < resPow2[1]; ++t)
            {
                resampledImage[index] = Clamp(tDatas[t], 0, 1);
            }
		}, resPow2[0], 16);

        //delete 之前new的数据
		for (auto ptr : resampleBufs) 
            delete[] ptr;
		resolution = resPow2;
    }
    else
    {

    }

    //计算各级mipmap的level
    int levels = Log2Int(std::max(resolution[0], resolution[1]));
    pyramid.resize(levels);

    //第0层的mipmap内存数据
	pyramid[0].reset(new BlockedArray<T>(resolution[0], resolution[1],
		resampledImage ? resampledImage.get() : memory));

    for (int i = 1; i < levels; ++i)
    {
        int sWidth = std::max(1, pyramid[i - 1]->uSize() / 2);
        int tHeight = std::max(1, pyramid[i - 1]->vSize() / 2);

        //box filter to caculate the current level from the last level
        ParallelFor(
            [&](int t) {
            for (int s = 0; s < sWidth; ++s)
            {
                (*pyramid[i])(s, t) = 0.25f * (Texel(i - 1, s, t) +
                    Texel(i - 1, s + 1, t) + Texel(i - 1, s, t + 1) + Texel(i - 1, s + 1, t + 1));
            }
        }, tHeight, 16);
    }
}

template <typename T>
std::unique_ptr<ResampleWeight[]> Mipmap<T>::ResampleWeights(int oldRes, int newRes)
{
    std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);
    Float filterRadius = 2.0f;

    for (int i = 0; i < newRes; ++i)
    {
        //取原像素的center位置
        Float center = (i + 0.5f) * oldRes / newRes;

        //影响他的第一个像素
        wt[i].firstTexel = std::floor(center - filterRadius + 0.5f);
        for (int j = 0; j < 4; ++j)
        {
            //计算影响他的像素的pos
            Float pos = wt[i].firstTexel + j + 0.5f;

            //pos - center代表在fitler中的位置
            //Lanczos的解释？
            wt[i].weight[j] = Lanczos((pos - center) / filterRadius);
        }

        //把4个weight归一化，4个加起来等于1
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
    const BlockedArray& l = pyramid[level];

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
    //根据dstdx计算width
	if (doTrilinear) 
    {
		Float width = std::max(std::max(std::abs(dstdx[0]), std::abs(dstdx[1])),
			std::max(std::abs(dstdy[0]), std::abs(dstdy[1])));
		return Lookup(st, 2 * width);
	}

    //ewa方法
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
            //第0层，直接返回最大层的texel
            return Triangle(0, st);
        }
        else if (level >= Levels() - 1)
        {
            //最后一层，只有一个texel值
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
    //把level clamp一下
    level = Clamp(level, 0, Levels());

    // s0,t1-------s1,t1
    //
    // --ds----s,t
    //          |
    //         dt
    // s0,t0----|--s1,t0
    int s0 = std::floor(st[0] * pyramid[level]->uSize() - 0.5f);
    int s1 = s0 + 1;
    int t0 = std::floor(st[1] * pyramid[level]->uSize() - 0.5f);
    int t1 = t0 + 1;

    Float ds = st[0] - s0;
    Float dt = st[1] - t0;

    return (1.0f - ds) * (1.0f - dt) * Texel(level, s0, t0) 
        + ds * (1.0f - dt) * Texel(level, s1, t0)
        + (1.0f - ds) * dt * Texel(level, s0, t1) 
        + ds * dt * Texel(level, s1, t1);
}

}
