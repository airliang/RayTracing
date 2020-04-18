#pragma once
#include "geometry.h"
#include "filter.h"
#include "spectrum.h"
#include "atomicfloat.h"

namespace AIR
{
	//先搞清楚一个概念，像素是指我们最后输出到image的有具体颜色值的
	//sample是给像素提供颜色贡献的样本，能影响filter radius范围内的所有像素
	//像素最终颜色公式如下：
	//分子中的w是和真实摄像机相关的
	//         ∑[i]f(x-xi,y-yi)w L(xi,yi) 
	//I(x,y) = ------------------------
	//             ∑[i]f(x-xi,y-yi)
	struct FilmTilePixel 
	{
		Spectrum contribSum = 0.f;     //对应上式中的分子累加
		Float filterWeightSum = 0.f;   //filter' weight，对应上式中的分母累加
	};
	class FilmTile;

    //可以理解成相机里的胶片
	class Film
	{
	public:
		//resolution film的总分辨率
		//cropWindow NDC[0,1]坐标下的裁剪区域
		//filter     filter类型，用于抗锯齿
		//filename   保存的文件名
		Film(const Point2i &resolution, const Bounds2f &cropWindow,
			std::unique_ptr<Filter> filt, const std::string &filename);

		//get the whole image output bounds
		//maybe the output is not (0,0)
		//so the cropped pixel bounds is the true output area
		Bounds2i GetOutputSampleBounds() const;

		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i &sampleBounds);

		//merge the filmtile into the final image
		//executing in threads
		void MergeFilmTile(std::unique_ptr<FilmTile> tile);
		void Clear();

		void WriteImage(Float splatScale = 1);
	public:
		const Point2i fullResolution;
		//图像过滤器
		std::unique_ptr<Filter> filter;
		const std::string filename;
		//裁剪像素区域
		Bounds2i croppedPixelBounds;

		//相当于一个filter radius作用到多少个像素上
		static constexpr int filterTableWidth = 16;

		//相当于卷积核的weight值
		//filterTableWidth相当于filter的半径
		//基于性能的考虑，这个filterTable是precompute的
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
			Float pad;   //把pixel凑够32 bytes 考虑cache line性能
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

	//为了多线程的考虑，Film把整个image划分为多个FilmTile
	//每个filmTile独立的数据
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
