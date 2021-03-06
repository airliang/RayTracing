#pragma once
#include "light.h"

namespace AIR
{
	class DistantLight : public Light
	{
	public:
		DistantLight(const Transform& LightToWorld, const Spectrum& L,
			const Vector3f& lightDirection)
			: Light((int)LightFlags::DeltaDirection, LightToWorld, MediumInterface())
			, L(L)
			, wLight(LightToWorld.ObjectToWorldVector(lightDirection))
		{

		}

		void Preprocess(const Scene& scene);

		Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Float Pdf_Li(const Interaction& ref, const Vector3f& wi) const
		{
			return 1.0f;
		}

		Spectrum Power() const;
	private:
		const Spectrum L;
		const Vector3f wLight;

		Float worldRadius;
		Point3f worldCenter;
	};


}
