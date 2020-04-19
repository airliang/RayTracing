#include "shape.h"

namespace AIR
{
	Interaction Shape::Sample(const Interaction& ref, const Point2f& u,
		Float* pdf) const
	{
        Interaction intr = Sample(u, pdf);
        //这里假设整个面积可见，先采样面积上的一点，
        //这个时候pdf = 1/A
        Vector3f wi = intr.interactPoint - ref.interactPoint;
        if (wi.LengthSquared() == 0)
            *pdf = 0;
        else 
        {
            wi = Vector3f::Normalize(wi);
            // Convert from area measure, as returned by the Sample() call
            // above, to solid angle measure.
            //dwi/dA = cosθ/r²
            //∫1/AdA = 1      dA = r²dwi/cosθ
            //∫1/A r²/cosθdwi = 1;
            //∴ p(wi) = r²/(Acosθ)
            //pdf *= 是和1/A相乘，看上面注释
            *pdf *= Vector3f::DistanceSquare(ref.interactPoint, intr.interactPoint) / Vector3f::AbsDot(intr.normal, -wi);
            if (std::isinf(*pdf)) 
                *pdf = 0.f;
        }
        return intr;
	}

    Float Shape::Pdf(const Interaction& ref, const Vector3f& wi) const
    {
		Ray ray = ref.SpawnRay(wi);
		Float tHit;
		Interaction isectLight;
		// Ignore any alpha textures used for trimming the shape when performing
		// this intersection. Hack for the "San Miguel" scene, where this is used
		// to make an invisible area light.
		if (!Intersect(ray, &tHit, &isectLight)) 
            return 0;

        //p_A是面积的pdf
        //pw是立体角的pdf
        //dw = dAcosθ/r²
        //∫p_AdA = ∫p_A dwr²/cosθ= ∫1/A r²/cosθ dw
        //pw = r²/(Acosθ)

        return Vector3f::DistanceSquare(isectLight.interactPoint, ref.interactPoint) / 
            (Area() * Vector3f::AbsDot(isectLight.normal, wi));
    }
}
