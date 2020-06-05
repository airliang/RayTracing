#include "diffusearealight.h"
#include "transform.h"
#include "shape.h"

namespace AIR
{
	DiffuseAreaLight::DiffuseAreaLight(const Transform& LightToWorld,
		const MediumInterface& mediumInterface,
		const Spectrum& Lemit, int nSamples, const std::shared_ptr<Shape>& shape) :
		AreaLight(LightToWorld, mediumInterface, nSamples)
		, Lemit(Lemit)
		, shape(shape)
		, area(shape->Area())
	{

	}

	Spectrum DiffuseAreaLight::Sample_Li(const Interaction&ref,
        const Point2f &u, Vector3f *wi, Float *pdf,
        VisibilityTester *vis) const
    {
        Interaction pShape = shape->Sample(ref, u, pdf);

        if (*pdf == 0 || (pShape.interactPoint - ref.interactPoint).LengthSquared() == 0) 
        {
            *pdf = 0;
            return 0.f;
        }
        *wi = Vector3f::Normalize(pShape.interactPoint - ref.interactPoint);
        *vis = VisibilityTester(ref, pShape);
        return L(pShape, -*wi);
    }

	Spectrum DiffuseAreaLight::Power() const 
    {
		return (twoSided ? 2 : 1) * Lemit * area * Pi;
	}

    Float DiffuseAreaLight::Pdf_Li(const Interaction& isect, const Vector3f& wi) const
    {
        //dw = dAcosθ/r²
        //∫1/AdA = ∫1/A r²/cosθdw
        //pw = 1/A r²/cosθ
        return shape->Pdf(isect, wi);
        Ray ray(isect.interactPoint, wi);
        Float tHit;
        SurfaceInteraction it;
        if (shape->Intersect(ray, &tHit, &it))
        {
            Float rSquare = Vector3f::DistanceSquare(isect.interactPoint, it.interactPoint);
            return rSquare / (Vector3f::AbsDot(it.normal, wi) * area);
        }
        return 0;
    }
}
