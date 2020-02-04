#include "film.h"
#include "imageio.h"

namespace AIR
{
	Film::Film(const Point2i &resolution, const Bounds2f &cropWindow, std::unique_ptr<Filter> filt, const std::string &filename)
		: fullResolution(resolution),
		filter(std::move(filt)),
		croppedPixelBounds(cropWindow),
		filename(filename)
	{

	}

	Bounds2i Film::GetOutputSampleBounds() const
	{
		Bounds2f floatBounds(Point2f::Floor(Point2f(croppedPixelBounds.pMin) +
			Vector2f(0.5f, 0.5f) - filter->radius),
			Point2f::Ceil(Point2f(croppedPixelBounds.pMax) -
				Vector2f(0.5f, 0.5f) + filter->radius));
		return (Bounds2i)floatBounds;
	}

	std::unique_ptr<FilmTile> Film::GetFilmTile(const Bounds2i &sampleBounds) 
	{
		// Bound image pixels that samples in _sampleBounds_ contribute to
		Vector2f halfPixel = Vector2f(0.5f, 0.5f);
		Bounds2f floatBounds = (Bounds2f)sampleBounds;
		Point2i p0 = (Point2i)Point2f::Ceil(floatBounds.pMin - halfPixel - filter->radius);
		Point2i p1 = (Point2i)Point2f::Floor(floatBounds.pMax - halfPixel + filter->radius) +
			Point2i(1, 1);
		Bounds2i tilePixelBounds = Bounds2i::Intersect(Bounds2i(p0, p1), croppedPixelBounds);
		return std::unique_ptr<FilmTile>(new FilmTile(
			tilePixelBounds, filter->radius, filterTable, filterTableWidth));
	}

	void Film::WriteImage(Float splatScale)
	{
		std::unique_ptr<Float[]> rgb(new Float[3 * croppedPixelBounds.Area()]);
		int offset = 0;
		//Bounds2iIterator pixelBounds(croppedPixelBounds, croppedPixelBounds.pMin);
		for (Point2i p : croppedPixelBounds)
		{
			// Convert pixel XYZ color to RGB
			Pixel &pixel = GetPixel(p);
			XYZToRGB(pixel.xyz, &rgb[3 * offset]);

			// Normalize pixel with weight sum
			Float filterWeightSum = pixel.filterWeightSum;
			if (filterWeightSum != 0) 
			{
				Float invWt = (Float)1 / filterWeightSum;
				rgb[3 * offset] = std::max((Float)0, rgb[3 * offset] * invWt);
				rgb[3 * offset + 1] =
					std::max((Float)0, rgb[3 * offset + 1] * invWt);
				rgb[3 * offset + 2] =
					std::max((Float)0, rgb[3 * offset + 2] * invWt);
			}

			// Add splat value at pixel
			//why?
			Float splatRGB[3];
			Float splatXYZ[3] = { pixel.splatXYZ[0], pixel.splatXYZ[1],
								 pixel.splatXYZ[2] };
			XYZToRGB(splatXYZ, splatRGB);
			rgb[3 * offset] += splatScale * splatRGB[0];
			rgb[3 * offset + 1] += splatScale * splatRGB[1];
			rgb[3 * offset + 2] += splatScale * splatRGB[2];

			// Scale pixel value by _scale_
			//rgb[3 * offset] *= scale;
			//rgb[3 * offset + 1] *= scale;
			//rgb[3 * offset + 2] *= scale;
			++offset;
		}

		//���Ҫд��image��
		ImageIO::WriteImage(filename, &rgb[0], croppedPixelBounds, fullResolution);
	}

	void Film::Clear() 
	{
		for (Point2i p : croppedPixelBounds) {
			Pixel &pixel = GetPixel(p);
			for (int c = 0; c < 3; ++c)
				pixel.splatXYZ[c] = pixel.xyz[c] = 0;
			pixel.filterWeightSum = 0;
		}
	}

	FilmTile::FilmTile(const Bounds2i &pixelBounds, const Vector2f &filterRadius, const Float *filterTable, int filterTableSize)
		: pixelBounds(pixelBounds), 
		filterRadius(filterRadius),
		invFilterRadius(1 / filterRadius.x, 1 / filterRadius.y),
		filterTable(filterTable),
		filterTableSize(filterTableSize)
	{
		pixels = std::vector<FilmTilePixel>(std::max(0, pixelBounds.Area()));
	}

	void FilmTile::AddSample(const Point2f &pFilm, Spectrum L, Float sampleWeight /* = 1. */)
	{
		//����������sampleӰ������أ���Ϊint���궼�Ǽ�0.5���൱������ȫ��ƫ��0.5�����㡣
		Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
		Point2i p0 = (Point2i)Point2f::Ceil(pFilmDiscrete - filterRadius);
		Point2i p1 =
			(Point2i)Point2f::Floor(pFilmDiscrete + filterRadius) + Point2i(1, 1);
		p0 = Max(p0, pixelBounds.pMin);
		p1 = Min(p1, pixelBounds.pMax);

		// Loop over filter support and add sample to pixel arrays

		// Precompute $x$ and $y$ filter table offsets
		//This can be done directly by dividing each component of the sample offset by the filter radius in that direction, 
		//giving a value between 0 and 1, and then multiplying by the table size.
		
		int *ifx = ALLOCA(int, p1.x - p0.x);
		for (int x = p0.x; x < p1.x; ++x) 
		{
			Float fx = std::abs((x - pFilmDiscrete.x) * invFilterRadius.x *
				filterTableSize);
			ifx[x - p0.x] = std::min((int)std::floor(fx), filterTableSize - 1);
		}
		int *ify = ALLOCA(int, p1.y - p0.y);
		for (int y = p0.y; y < p1.y; ++y) 
		{
			Float fy = std::abs((y - pFilmDiscrete.y) * invFilterRadius.y *
				filterTableSize);
			ify[y - p0.y] = std::min((int)std::floor(fy), filterTableSize - 1);
		}
		for (int y = p0.y; y < p1.y; ++y)
		{
			for (int x = p0.x; x < p1.x; ++x) 
			{
				// Evaluate filter value at $(x,y)$ pixel
				//�����x,y���������sample�µ�filter
				int offset = ify[y - p0.y] * filterTableSize + ifx[x - p0.x];
				Float filterWeight = filterTable[offset];

				// Update pixel values with filtered sample contribution
				FilmTilePixel &pixel = GetPixel(Point2i(x, y));
				pixel.contribSum += L * sampleWeight * filterWeight;
				pixel.filterWeightSum += filterWeight;
			}
		}
	}
}