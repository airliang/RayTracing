#pragma once
#include "filter.h"

namespace AIR
{
	class GaussianFilter : public Filter
	{
	public:
		GaussianFilter(const Vector2f& radius, Float alpha) : Filter(radius),
			alpha(alpha),
			expX(std::exp(-alpha * radius.x * radius.x)),
			expY(std::exp(-alpha * radius.y * radius.y))
		{

		}
		virtual Float Evaluate(const Point2f &p) const
		{
			return Gaussian(p.x, expX) * Gaussian(p.y, expY);
		}



	private:
		// GaussianFilter Private Data
		const Float alpha;
		const Float expX, expY;

		// GaussianFilter Utility Functions
		Float Gaussian(Float d, Float expv) const {
			return std::max((Float)0, Float(std::exp(-alpha * d * d) - expv));
		}
	};
}