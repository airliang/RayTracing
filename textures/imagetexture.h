#pragma once

#include "texture.h"
#include "mipmap.h"

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

	//从文件的rgbspectrum转到rgbspectrum
	static void ConvertIn(const RGBSpectrum& from, RGBSpectrum* to,
		Float scale, bool gamma) {
		for (int i = 0; i < RGBSpectrum::nSamples; ++i)
			(*to)[i] = scale * (gamma ? InverseGammaCorrect(from[i])
				: from[i]);
	}

	//从文件的rgbspectrum转到float
	//RGBSpectrum::y()返回的是量度，所以用这个来代替
	static void ConvertIn(const RGBSpectrum& from, Float* to,
		Float scale, bool gamma) {
		*to = scale * (gamma ? InverseGammaCorrect(from.y())
			: from.y());
	}

	//转换到输出的spectrum(可能是SampledSpectrum)
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
