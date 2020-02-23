#include "imagetexture.h"

namespace AIR
{
template <typename Tmemory, typename Treturn> 
ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma) : mapping(std::move(mapping))
                 ,
{
    
}
}
