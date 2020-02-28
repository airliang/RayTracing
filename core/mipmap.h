#pragma once

#include "spectrum.h"
#include "geometry.h"
#include "memory.h"

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
    std::string filename;   //文件名
    bool doTrilinear;       //是否用trilinear来过滤，true是，false用EWA来过滤
    Float maxAniso;         //最大各向异性差值
    ImageWrap wrapMode;     //纹理坐标模式
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

//重采样的新的texel只受不多于的4个texel的权重影响
struct ResampleWeight 
{
	int firstTexel;  //原来的像素序号offset
	Float weight[4]; //影响的4个权重
};

template <typename T>
class Mipmap
{
public:
    Mipmap(const Point2i& res, const T* memory,
        bool doTrilinear, Float maxAnisotropy, ImageWrap wrapMode);

    T Lookup(const Point2f& st, const Vector2f& dstdx, const Vector2f& dstdy) const;

    //返回mipmap的texel值
    //st 纹理坐标
    //width 过滤的宽度，用于查找level，width的范围是[0-1]
    //假设mipmap实际宽度是L，width可理解成 1/L * scale
    T Lookup(const Point2f& st, Float width = 0.f) const;

    //获得纹理内存具体texel的值
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
private:
    //determines which original texels contribute to each new texel 
    //and what the values are of the contribution weights for each new texel.
    //return ResampleWeight[] the contribution weights for each new texel,长度是newRes
    //每个newTexel受影响的4个权重值
    //int oldRes 原来分辨率
    //int newRes 新的分辨率
    std::unique_ptr<ResampleWeight[]> ResampleWeights(int oldRes, int newRes);
    Point2i resolution;
    const bool trilinear;
    const ImageWrap wrapMode;
    const Float maxAnisotropy;

    //mipmap每层的内存数据
    std::vector<std::unique_ptr<BlockedArray<T>>> pyramid;
};
}
