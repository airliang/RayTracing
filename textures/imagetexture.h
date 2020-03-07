#pragma once

#include "texture.h"
#include "mipmap.h"
#include <map>

namespace AIR
{
template <typename Tmemory, typename Treturn>
    class ImageTexture : public Texture<Treturn> 
{
public:
    ImageTexture(std::unique_ptr<TextureMapping2D> m,
                 const std::string &filename, bool doTri, Float maxAniso,
                 ImageWrap wm, Float scale, bool gamma);

	static void ClearCache() {
        s_textures.erase(s_textures.begin(), s_textures.end());
	}

	Treturn Evaluate(const Interaction& si) const;
private:
	static Mipmap<Tmemory>* GetTexture(const std::string& filename, bool doTri, Float maxAniso,
		ImageWrap wm, Float scale, bool gamma);

	//���ļ���rgbspectrumת��rgbspectrum
	static void ConvertIn(const RGBSpectrum& from, RGBSpectrum* to,
		Float scale, bool gamma) {
		for (int i = 0; i < RGBSpectrum::nSamples; ++i)
			(*to)[i] = scale * (gamma ? InverseGammaCorrect(from[i])
				: from[i]);
	}

	//���ļ���rgbspectrumת��float
	//RGBSpectrum::y()���ص������ȣ����������������
	static void ConvertIn(const RGBSpectrum& from, Float* to,
		Float scale, bool gamma) {
		*to = scale * (gamma ? InverseGammaCorrect(from.y())
			: from.y());
	}

	//ת���������spectrum(������SampledSpectrum)
	static void ConvertOut(const RGBSpectrum& from, Spectrum* to)
	{
		Float rgb[3];
		from.ToRGB(rgb);
		*to = Spectrum::FromRGB(rgb);
	}

	static void ConvertOut(Float from, Float* to)
	{
		*to = from;
	}
private:
    std::unique_ptr<TextureMapping2D> mapping;
    Mipmap<Tmemory>* mipmap;

    static std::map<TexInfo, std::unique_ptr<Mipmap<Tmemory>>> s_textures;
};
}
