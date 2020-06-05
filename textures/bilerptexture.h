#pragma once

#include "texture.h"

namespace AIR
{
template <typename T>
class BilerpTexture : public Texture<T>
{
public:
    BilerpTexture(std::unique_ptr<TextureMapping2D> mapping, const T &v00,
              const T &v01, const T &v10, const T &v11) : mapping(std::move(mapping))
              v00(v00), v01(v01), v10(v10), v11(v11)
    {}

    //temp1 = (1 - s)v00 + s·v01
    //temp2 = (1 - s)v10 + s·v11
    //result = (1 - t)temp1 + t·temp2
    virtual T Evaluate(const SurfaceInteraction &si) const
    {
        Vector2f dstdx, dstdy;
        Point2f st = mapping->Map(si, &dstdx, &dstdy);
        Float s = st.x;
        Float t = st.y;
        return (1 - s) * (1 - t) * v00 + (1 - s) * (t) * v01 +
               (s) * (1 - t) * v10 + (s) * (t) * v11;
    }
private:
    std::unique_ptr<TextureMapping2D> mapping;
    T v00, v01, v10, v11;
};
}
