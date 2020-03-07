#include "pointlight.h"

namespace AIR
{
	Spectrum PointLight::Sample_Li(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, VisibilityTester* vis) const
	{
		*wi = Vector3f::Normalize(position - ref.interactPoint);
		*pdf = 1.0f;
		*vis = VisibilityTester(ref, Interaction(position, ref.time));

		return intensity / Vector3f::DistanceSquare(position, ref.interactPoint);
	}

	Spectrum PointLight::Power() const
	{
		return 4 * Pi * intensity;
	}
}

