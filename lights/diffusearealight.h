#pragma once
#include "light.h"

namespace AIR
{
	class Shape;
	class Transform;
	class DiffuseAreaLight : public AreaLight
	{
	public:
		DiffuseAreaLight(const Transform& LightToWorld,
			const MediumInterface& mediumInterface,
			const Spectrum& Lemit, int nSamples, const std::shared_ptr<Shape>& shape);

		Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Spectrum Power() const;

		Float Pdf_Li(const Interaction&, const Vector3f&) const;
		//evaluate the area light’s emitted radiance
		Spectrum L(const Interaction& intr, const Vector3f& w) const {
			return (twoSided || Vector3f::Dot(intr.normal, w) > 0) ? Lemit : Spectrum(0.f);
		}
	protected:
	    //就是radiance，diffuse是各个方向都相同的radiance
		const Spectrum Lemit;
		std::shared_ptr<Shape> shape;
		//面积
		const Float area; 
		const bool twoSided = false;
	};
}
