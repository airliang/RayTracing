#pragma once
#include "geometry.h"

namespace AIR
{
	class RGBSpectrum;
	class ImageIO
	{
	public:
		static void WriteImage(const std::string &name, const Float *rgb,
			const Bounds2i &outputBounds, const Point2i &totalResolution);

		static std::unique_ptr<RGBSpectrum[]> ReadImage(const std::string& filename, Point2i& resolution);

		static void InitPath(const std::string& root);

		static std::string imageLoadPath;
	};
}
