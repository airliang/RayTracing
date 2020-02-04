#pragma once
#include "geometry.h"
#include "filter.h"
#include "spectrum.h"
#include "atomicfloat.h"

namespace AIR
{
	struct FilmTilePixel 
	{
		Spectrum contribSum = 0.f;
		Float filterWeightSum = 0.f;   //filter' weight
	};
	class FilmTile;

	class Film
	{
	public:
		Film(const Point2i &resolution, const Bounds2f &cropWindow,
			std::unique_ptr<Filter> filt, const std::string &filename);

		//get the whole image output bounds
		Bounds2i GetOutputSampleBounds() const;

		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i &sampleBounds);
		void Clear();

		void WriteImage(Float splatScale = 1);
	public:
		const Point2i fullResolution;
		std::unique_ptr<Filter> filter;
		const std::string filename;
		Bounds2i croppedPixelBounds;
		static constexpr int filterTableWidth = 16;
		Float filterTable[filterTableWidth * filterTableWidth];
	private:
		struct Pixel 
		{
			Float xyz[3] = { 0, 0, 0 };
			Float filterWeightSum = 0;
			AtomicFloat splatXYZ[3];
			Float pad;
		};
		std::unique_ptr<Pixel[]> pixels;

		Pixel &GetPixel(const Point2i &p) 
		{
			//CHECK(InsideExclusive(p, croppedPixelBounds));
			int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
			int offset = (p.x - croppedPixelBounds.pMin.x) +
				(p.y - croppedPixelBounds.pMin.y) * width;
			return pixels[offset];
		}
	};


	class FilmTile
	{
	public:
		FilmTile(const Bounds2i &pixelBounds, const Vector2f &filterRadius,
			const Float *filterTable, int filterTableSize);

		//pFilm是具体的样本位置,
		//L是样本的radiance值
		//sampleWeight是样本的权重，真实镜头才用到的。
		//详细见如下公式：
		//I(x,y) = ∑[i]f(x - xi,y - yi)w(xi,yi)L(xi,yi) / ∑if(x - xi,y - yi)
		//w(xi,yi)是sampleWeight
		//xi,yi是pFilm
		//L是L(xi,yi)
		void AddSample(const Point2f &pFilm, Spectrum L,
			Float sampleWeight = 1.);

		//这里获得的是整个pixelBounds范围内的pixel
		FilmTilePixel &GetPixel(const Point2i &p) 
		{
			//CHECK(InsideExclusive(p, pixelBounds));
			int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
			int offset =
				(p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
			return pixels[offset];
		}

		Bounds2i GetPixelBounds() const 
		{ 
			return pixelBounds; 
		}
	private:
		const Bounds2i pixelBounds;
		const Vector2f filterRadius, invFilterRadius;
		const Float *filterTable;
		const int filterTableSize;
		std::vector<FilmTilePixel> pixels;
	};

}
