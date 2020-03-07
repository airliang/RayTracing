#include "imagetexture.h"
#include "imageio.h"

namespace AIR
{
template <typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<Mipmap<Tmemory>>> ImageTexture<Tmemory, Treturn>::s_textures;

template <typename Tmemory, typename Treturn> 
ImageTexture<Tmemory, Treturn>::ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma) : mapping(std::move(mapping))
                 
{
    mipmap = GetTexture(m, filename, doTri, maxAniso, wm, scale, gamma);
}


template <typename Tmemory, typename Treturn>
static Mipmap<Tmemory>* ImageTexture<Tmemory, Treturn>::GetTexture(const std::string& filename, bool doTri, Float maxAniso, ImageWrap wm, Float scale, bool gamma)
{
    TexInfo texInfo(filename, doTri, maxAniso, wm, scale, gamma);

    //�ȴ�table�����
    if (s_textures.find(texInfo) != s_textures.end())
    {
        return s_textures[texInfo].get();
    }
    Mipmap<Tmemory>* mipmap = nullptr;
    //���ļ����ȡmipmap
    Point2i resolution;
    std::unique_ptr<RGBSpectrum[]> texels = ReadImage(filename, &resolution);
    if (texels != nullptr)
    {
        //����һ��ת��
        //��ô֪����תFloat����RGBSpectrum��
        //Tmemory���;�������RGBSpectrum����Float
		std::unique_ptr<Tmemory[]> convertedTexels(new Tmemory[resolution.x *
			resolution.y]);

        for (int i = 0; i < resolution.x * resolution.y; ++i)
        {
            //�������ת��
            ConvertIn(texels[i], &convertedTexels[i], scale, gamma);
        }

        mipmap = new Mipmap<Tmemory>(resolution, convertedTexels, doTri, );

        s_textures[texInfo].reset(mipmap);
    }
    else
    {
        //û���ļ��Ļ�����һ�����ص�mipmap
		Tmemory oneVal = scale;
		mipmap = new Mipmap<Tmemory>(Point2i(1, 1), &oneVal);
    }

    return mipmap;
}

template <typename Tmemory, typename Treturn>
Treturn ImageTexture<Tmemory, Treturn>::Evaluate(const Interaction& si) const
{
    Vector2f dstdx, dstdy;
    //���ݱ��������������st
    Point2f st = mapping->Map(si, &dstdx, &dstdy);

    Tmemory texel = mipmap->Lookup(st, dstdx, dstdy);

    Treturn result;
    ConvertOut(texel, &result);

    return result;
}

}
