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
        bool doTrilinear, Float maxAnisotropy, ImageWrap wrapMode);

    T Lookup(const Point2f& st, const Vector2f& dstdx, const Vector2f& dstdy) const;

    //����mipmap��texelֵ
    //st ��������
    //width ���˵Ŀ�ȣ����ڲ���level��width�ķ�Χ��[0-1]
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
}
