#pragma once

#include "texture.h"

namespace AIR
{
template<typename T> class ConstantTexture : public Texture<T>
{
private:
    T value;
public:
    ConstantTexture(const T value) : value(value)
    {

    }
    ~ConstantTexture() {}

    virtual T Evaluate(const SurfaceInteraction &si) const
    {
        return value;
    }
};
    
    
}
