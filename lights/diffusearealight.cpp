#include "diffusearealight.h"

namespace AIR
{
	Spectrum DiffuseAreaLight::Sample_Li(const Interaction &ref,
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
}
