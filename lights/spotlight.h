#pragma once

#include "light.h"

namespace AIR
{
	class SpotLight : public Light
	{
	public:
		SpotLight(const Transform& LightToWorld, const MediumInterface& m, const Spectrum& intensity,
			Float totalWidth, Float falloffStart) : Light((int)LightFlags::DeltaPosition, LightToWorld, m)
			, position(LightToWorld.ObjectToWorldPoint(Point3f::zero))
			, intensity(intensity)
			, cosTotalWidth(std::cos(totalWidth))
			, cosFalloffStart(std::cos(falloffStart))
		{

		}

		Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Spectrum Power() const;

		Float Falloff(const Vector3f& w) const;
	private:
		const Point3f position;
		const Spectrum intensity;
		//cone的半角的cos值
		const Float cosTotalWidth;
		//cone的起始的startoff角的cos值
		const Float cosFalloffStart;
	};
}
