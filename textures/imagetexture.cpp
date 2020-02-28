#include "imagetexture.h"
#include "imageio.h"

namespace AIR
{
template <typename Tmemory, typename Treturn> 
ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma) : mapping(std::move(mapping))
                 
{
    mipmap = GetTexture(m, filename, doTri, maxAniso, wm, scale, gamma);
}

template <typename Tmemory, typename Treturn>
Treturn ImageTexture<Tmemory, Treturn>::Evaluate(const Interaction& si) const
{
    //计算出dstdx dstdy
    Vector2f dstdx, dstdy;
    Point2f st = mapping->Map(si, &dstdx, &dstdy);
    Tmemory img = mipmap->Lookup(st, dstdx, dstdy);
    Treturn out;
    ConvertOut(img, &out);
    return out;
}

static Mipmap<Tmemory>* ImageTexture::GetTexture(const std::string& filename, bool doTri, Float maxAniso, ImageWrap wm, Float scale, bool gamma)
{
    TexInfo texInfo(filename, doTri, maxAniso, wm, scale, gamma);

    //先从table里查找
    if (s_textures.find(texInfo) != s_textures.end())
    {
        return s_textures[texInfo].get();
    }
    Mipmap<Tmemory>* mipmap = nullptr;
    //从文件里读取mipmap
    Point2i resolution;
    std::unique_ptr<RGBSpectrum[]> texels = ReadImage(filename, &resolution);
    if (texels != nullptr)
    {
        //先做一次转换
        //怎么知道是转Float还是RGBSpectrum？
        //Tmemory类型决定了是RGBSpectrum还是Float
		std::unique_ptr<Tmemory[]> convertedTexels(new Tmemory[resolution.x *
			resolution.y]);

        for (int i = 0; i < resolution.x * resolution.y; ++i)
        {
            //逐个像素转换
            ConvertIn(texels[i], &convertedTexels[i], scale, gamma);
        }

        mipmap = new Mipmap<Tmemory>(resolution, convertedTexels, doTri, );

        s_textures[texInfo].reset(mipmap);
    }
    else
    {
        //没有文件的话创建一个像素的mipmap
		Tmemory oneVal = scale;
		mipmap = new Mipmap<Tmemory>(Point2i(1, 1), &oneVal);
    }

    return mipmap;
}

template <typename Tmemory, typename Treturn>
Treturn ImageTexture<Tmemory, Treturn>::Evaluate(const Interaction& si) const
{
    Vector2f dstdx, dstdy;
    //根据表面算出纹理坐标st
    Point2f st = mapping->Map(si, &dstdx, &dstdy);

    Tmemory texel = mipmap->Lookup(st, dstdx, dstdy);

    Treturn result;
    ConvertOut(texel, &result);

    return result;
}

}
