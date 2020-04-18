#pragma once
#include "filter.h"

namespace AIR
{
	class TriangleFilter : public Filter
	{
	public:
		TriangleFilter(const Vector2f& radius) : Filter(radius) {}
		virtual Float Evaluate(const Point2f &p) const
		{
			return std::max((Float)0, radius.x - std::abs(p.x)) *
				std::max((Float)0, radius.y - std::abs(p.y));
		}
	};
}