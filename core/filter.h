#pragma once
#include "geometry.h"

namespace AIR
{
	class Filter
	{
	public:
		virtual ~Filter()
		{

		}
		Filter(const Vector2f &radius)
			: radius(radius), invRadius(Vector2f(1 / radius.x, 1 / radius.y)) {}

		//return filter's value at the point p
		//该技巧很好，因为把filter当成是1维函数，无论多少维的值都适用
		virtual Float Evaluate(const Point2f &p) const = 0;

		// Filter Public Data
		const Vector2f radius, invRadius;
	};
}