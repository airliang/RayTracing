#pragma once
#include "light.h"
#include "mipmap.h"
#include "sampling.h"

namespace AIR
{
	class Scene;
	class InfiniteAreaLight : public Light
	{
	public:
		InfiniteAreaLight(const Transform& LightToWorld,
			const Spectrum& L, int nSamples, const std::string& texmap);

		virtual Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
			Vector3f* wi, Float* pdf, VisibilityTester* vis) const;

		Float Pdf_Li(const Interaction&, const Vector3f&) const;

		//virtual Spectrum LiEscape(const RayDifferential& r) const;

		void Preprocess(const Scene& scene);

		Spectrum Power() const;

		Spectrum LiEscape(const RayDifferential& r) const;
	private:
		std::unique_ptr<Mipmap<RGBSpectrum>> Lmap;

		Point3f worldCenter;
		Float worldRadius;
		std::unique_ptr<Distribution2D> distribution;
	};
}
