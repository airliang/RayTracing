#pragma once
#include "geometry.h"

namespace AIR
{
	class ImageIO
	{
	public:
		static void WriteImage(const std::string &name, const Float *rgb,
			const Bounds2i &outputBounds, const Point2i &totalResolution);


	};
}
