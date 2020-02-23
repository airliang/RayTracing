#pragma once

#include "spectrum.h"

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
    std::string filename;
    bool doTrilinear;
    Float maxAniso;
    ImageWrap wrapMode;
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

template <typename T>
class Mipmap
{
public:
    Mipmap(const Point2i& resolution, std::unique_ptr<Tmemory[]> memory,
    bool doTrilinear, );

};
}
