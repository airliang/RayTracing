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

    //�������������Ľ�Ƭ
	class Film
	{
	public:
		//resolution film���ֱܷ���
		//cropWindow NDC[0,1]�����µĲü�����
		//filter     filter���ͣ����ڿ����
		//filename   ������ļ���
		Film(const Point2i &resolution, const Bounds2f &cropWindow,
			std::unique_ptr<Filter> filt, const std::string &filename);

		//get the whole image output bounds
		//maybe the output is not (0,0)
		//so the cropped pixel bounds is the true output area
		Bounds2i GetOutputSampleBounds() const;

		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i &sampleBounds);

		void MergeFilmTile(std::unique_ptr<FilmTile> tile);
		void Clear();

		void WriteImage(Float splatScale = 1);
	public:
		const Point2i fullResolution;
		//ͼ�������
		std::unique_ptr<Filter> filter;
		const std::string filename;
		//�ü���������
		Bounds2i croppedPixelBounds;

		//�൱��һ��filter radius���õ����ٸ�������
		static constexpr int filterTableWidth = 16;

		//�൱�ھ���˵�weightֵ
		//filterTableWidth�൱��filter�İ뾶
		//�������ܵĿ��ǣ����filterTable��precompute��
		Float filterTable[filterTableWidth * filterTableWidth];
	private:
		struct Pixel 
		{
			Float xyz[3] = { 0, 0, 0 };
			//filterWeightSum holds the sum of filter weight values 
			//for the sample contributions to the pixel. 
			Float filterWeightSum = 0;
			//splatXYZ holds an (unweighted) sum of sample splats.
			AtomicFloat splatXYZ[3];
			Float pad;   //��pixel�չ�32 bytes ����cache line����
		};
		std::unique_ptr<Pixel[]> pixels;
		std::mutex mutex;

		Pixel &GetPixel(const Point2i &p) 
		{
			//CHECK(InsideExclusive(p, croppedPixelBounds));
			int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
			int offset = (p.x - croppedPixelBounds.pMin.x) +
				(p.y - croppedPixelBounds.pMin.y) * width;
			return pixels[offset];
		}
	};

	//Ϊ�˶��̵߳Ŀ��ǣ�Film������image����Ϊ���FilmTile
	//ÿ��filmTile����������
	class FilmTile
	{
	public:
		FilmTile(const Bounds2i &pixelBounds, const Vector2f &filterRadius,
			const Float *filterTable, int filterTableSize);

		//pFilm�Ǿ��������λ��,
		//L��������radianceֵ
		//sampleWeight��������Ȩ�أ���ʵ��ͷ���õ��ġ�
		//��ϸ�����¹�ʽ��
		//I(x,y) = ��[i]f(x - xi,y - yi)w(xi,yi)L(xi,yi) / ��if(x - xi,y - yi)
		//w(xi,yi)��sampleWeight
		//xi,yi��pFilm
		//L��L(xi,yi)
		void AddSample(const Point2f &pFilm, Spectrum L,
			Float sampleWeight = 1.);

		//�����õ�������pixelBounds��Χ�ڵ�pixel
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
