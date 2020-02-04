#pragma once
#include "filter.h"

namespace AIR
{
	class BoxFilter : public Filter
	{
	public:
		BoxFilter(const Vector2f &radius) : Filter(radius) {}
		virtual Float Evaluate(const Point2f &p) const
		{
			return 1.f;
		}
	};
}