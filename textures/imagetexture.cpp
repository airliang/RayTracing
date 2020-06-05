#include "imagetexture.h"
#include "imageio.h"

namespace AIR
{
	template class ImageTexture<Float, Float>;
	template class ImageTexture<RGBSpectrum, Spectrum>;

template <typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<Mipmap<Tmemory>>> ImageTexture<Tmemory, Treturn>::s_textures;

template <typename Tmemory, typename Treturn> 
ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma) : mapping(std::move(m))
                 
{
    mipmap = GetTexture(filename, doTri, maxAniso, wm, scale, gamma);
}


template <typename Tmemory, typename Treturn>
Mipmap<Tmemory>* ImageTexture<Tmemory, Treturn>::GetTexture(const std::string& filename, bool doTri, Float maxAniso, ImageWrap wm, Float scale, bool gamma)
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
    std::unique_ptr<RGBSpectrum[]> texels = ImageIO::ReadImage(filename, resolution);
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
            ConvertIn(texels[i], &convertedTexels[i], scale, /*gamma*/true);
        }

        mipmap = new Mipmap<Tmemory>(resolution, convertedTexels.get(), doTri, maxAniso, wm);

        s_textures[texInfo].reset(mipmap);
    }
    else
    {
        //没有文件的话创建一个像素的mipmap
		//RGBSpectrum* oneVal = new RGBSpectrum[1];
  //      *oneVal = RGBSpectrum(scale);
  //      texels.reset(oneVal)
        Tmemory oneVal = scale;
		mipmap = new Mipmap<Tmemory>(Point2i(1, 1), &oneVal);
    }

    return mipmap;
}

template <typename Tmemory, typename Treturn>
Treturn ImageTexture<Tmemory, Treturn>::Evaluate(const SurfaceInteraction& si) const
{
    Vector2f dstdx, dstdy;
    //根据表面算出纹理坐标st
    Point2f st = mapping->Map(si, &dstdx, &dstdy);

    Tmemory texel = mipmap->Lookup(st, dstdx, dstdy);

    Treturn result;
    ConvertOut(texel, &result);

    return result;
}


ImageTexture<RGBSpectrum, Spectrum>* CreateImageSpectrumTexture(
    const Matrix4f& tex2world, const TextureParams& param)
{
    std::unique_ptr<TextureMapping2D> map;
    if (param.mapping == "uv")
    {
        map.reset(new UVMapping2D());
    }
    else if (param.mapping == "spherical")
    {
        map.reset(new SphericalMapping2D(tex2world));
    }
    else if (param.mapping == "planer")
    {
    }
    bool trilerp = true;

    ImageWrap wrapMode = ImageWrap::Repeat;
    if (param.wrap == "repeat")
    {
        wrapMode = ImageWrap::Repeat;
    }
    else if (param.wrap == "clamp")
    {
        wrapMode = ImageWrap::Clamp;
    }

    bool gamma = false;
    Float maxAniso = 8.0f;

	return new ImageTexture<RGBSpectrum, Spectrum>(
		std::move(map), param.filename, trilerp, maxAniso, wrapMode, 1.0f, gamma);
}

}
