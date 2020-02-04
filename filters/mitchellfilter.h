#pragma once
#include "filter.h"

namespace AIR
{
	class MitchellFilter : public Filter
	{
	public:
		MitchellFilter(const Vector2f &radius, Float B, Float C)
			: Filter(radius), B(B), C(C) {}
		virtual Float Evaluate(const Point2f &p) const
		{
			return Mitchell1D(p.x * invRadius.x) * Mitchell1D(p.y * invRadius.y);
		}

		Float Mitchell1D(Float x) const {
			x = std::abs(2 * x);
			if (x > 1)
				return ((-B - 6 * C) * x * x * x + (6 * B + 30 * C) * x * x +
				(-12 * B - 48 * C) * x + (8 * B + 24 * C)) *
					(1.f / 6.f);
			else
				return ((12 - 9 * B - 6 * C) * x * x * x +
				(-18 + 12 * B + 6 * C) * x * x + (6 - 2 * B)) *
					(1.f / 6.f);
		}

	private:
		const Float B, C;
	};
}