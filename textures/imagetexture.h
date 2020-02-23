#pragma once

#include "texture.h"
#include "mipmap.h"

namespace AIR
{
template <typename Tmemory, typename Treturn>
    class ImageTexture : public Texture<Treturn> 
{
public:
    template <typename Tmemory, typename Treturn>
    ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma);

private:
    std::unique_ptr<TextureMapping2D> mapping;
    
};
}
