#pragma once
#include "light.h"

namespace AIR
{
	class PointLight : public Light
	{
	public:
		PointLight(const Transform& LightToWorld,
			const MediumInterface& mediumInterface, const Spectrum& intensity)
			: Light((int)LightFlags::DeltaPosition, LightToWorld, mediumInterface)
			, position(LightToWorld.ObjectToWorldPoint(Point3f::zero))
			, intensity(intensity)
		{

		}

		Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Float Pdf_Li(const Interaction& ref, const Vector3f& wi) const
		{
			return 1.0f;
		}

		Spectrum Power() const;
	private:
		//position in world space
		const Point3f position;
		//the amount of power per unit solid angle
		const Spectrum intensity;
	};
}
