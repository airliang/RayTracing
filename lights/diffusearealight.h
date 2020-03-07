#pragma once
#include "light.h"

namespace AIR
{
	class DiffuseAreaLight : public AreaLight
	{
	public:
		DiffuseAreaLight(const Transform& LightToWorld,
			const Spectrum& Lemit, int nSamples, const std::shared_ptr<Shape>& shape) :
			AreaLight(LightToWorld, nSamples)
			, Lemit(Lemit)
			, shape(shape)
			, area(shape->Area())
		{

		}

		Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Spectrum Power() const;

		Spectrum L(const Interaction& intr, const Vector3f& w) const {
			return (twoSided || Vector3f::Dot(intr.normal, w) > 0) ? Lemit : Spectrum(0.f);
		}
	protected:
		const Spectrum Lemit;
		std::shared_ptr<Shape> shape;
		//Ãæ»ý
		const Float area; 
		const bool twoSided = false;
	};
}
