#include "shape.h"

namespace AIR
{
	Interaction Shape::Sample(const Interaction& ref, const Point2f& u,
		Float* pdf) const
	{
        Interaction intr = Sample(u, pdf);
        //这里假设整个面积可见，先采样面积上的一点，
        //这个时候pdf = 1/A
        Vector3f wi = intr.p - ref.p;
        if (wi.LengthSquared() == 0)
            *pdf = 0;
        else 
        {
            wi = Normalize(wi);
            // Convert from area measure, as returned by the Sample() call
            // above, to solid angle measure.
            //dwi/dA = cosθ/r²
            //∫1/AdA = 1      dA = r²dwi/cosθ
            //∫1/A r²/cosθdwi = 1;
            //∴ p(wi) = r²/(Acosθ)
            //pdf *= 是和1/A相乘，看上面注释
            *pdf *= Vector3f::DistanceSquare(ref.p, intr.p) / Vector3f::AbsDot(intr.n, -wi);
            if (std::isinf(*pdf)) 
                *pdf = 0.f;
        }
        return intr;
	}
}
