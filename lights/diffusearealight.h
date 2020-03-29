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

		//evaluate the area light’s emitted radiance
		Spectrum L(const Interaction& intr, const Vector3f& w) const {
			return (twoSided || Vector3f::Dot(intr.normal, w) > 0) ? Lemit : Spectrum(0.f);
		}
	protected:
	    //就是radiance，diffuse是各个方向都相同的radiance
		const Spectrum Lemit;
		std::shared_ptr<Shape> shape;
		//���
		const Float area; 
		const bool twoSided = false;
	};
}
