#include "texture.h"
#include "robject.h"

namespace AIR
{
UVMapping2D::UVMapping2D(Float su, Float sv, Float du, Float dv) : su(su)
, sv(sv)
, du(du)
, dv(dv)
{

}

Point2f UVMapping2D::Map(const Interaction& si, Vector2f *dstdx, Vector2f* dstdy) const
{
    //u = u(x, y), s = s(u, v)
    //多元复合函数求导
    //∂s   ∂s ∂u   ∂s ∂v
    //-- = -- -- + -- --       1式
    //∂x   ∂u ∂x   ∂v ∂x

    //s = su·u + du,s对uv求偏导得到：
    //∂s/∂u = su, ∂s/∂v = 0，根据1式
    //∂s/∂x = su·dudx


    //∂t   ∂t ∂u   ∂t ∂v
    //-- = -- -- + -- --       2式
    //∂x   ∂u ∂x   ∂v ∂x
    //t = sv·v + dv
    //∂t/∂u = 0, ∂t/∂v = sv
    //∂t/∂x = sv·dvdx
    *dstdx = Vector2f(su * si.dudx, sv * si.dvdx);
    *dstdy = Vector2f(su * si.dudy, sv * si.dvdy);

    return Point2f(si.uv[0] * su + du, si.uv[1] * sv + dv);
}

Point2f SphericalMapping2D::Sphere(const Vector3f& p) const
{
    //返回p对应的球面坐标
    Vector3f vec = Vector3f::Normalize(MultiplyPoint(worldToTexture, p) - Vector3f::zero);
    Float theta = SphericalTheta(vec);
    Float phi = SphericalPhi(vec);
    return Point2f(theta * InvPi, phi * Inv2Pi);
}

Point2f SphericalMapping2D::Sphere(const Vector3f& p, const Matrix4f& w2t) const
{
	Vector3f vec = Vector3f::Normalize(MultiplyPoint(w2t, p) - Vector3f::zero);
	//Float theta = SphericalTheta(vec);
	//Float phi = SphericalPhi(vec);
	//由于我这里的球面定义不是按z向上的，所以不能用SphericalTheta和SphericalPhi
	//定义了y向上
	Float theta = SphericalThetaYup(vec);
	Float phi = SphericalPhiYup(vec);
	return Point2f(theta * InvPi, phi * Inv2Pi);
}

Point2f SphericalMapping2D::Map(const Interaction& si, Vector2f *dstdx, Vector2f* dstdy) const
{
    // ∂s   sphere(p + Δ∂p/∂x) - sphere(p)
    // -- = ------------------------------
    // ∂x                Δ
    Float delta = 0.1f;
    Point2f st = Sphere(si.interactPoint, si.primitive->GetTransform()->WorldToLocal());
    Point2f deltaX = Sphere(si.interactPoint + delta * si.dpdx, si.primitive->GetTransform()->WorldToLocal());
    Point2f deltaY = Sphere(si.interactPoint + delta * si.dpdy, si.primitive->GetTransform()->WorldToLocal());

    *dstdx = (deltaX - st) / delta;
    *dstdy = (deltaY - st) / delta;

    //xy变化时，t不可能超过一个半圆，所以必须做处理
    if ((*dstdx)[1] > .5)
        (*dstdx)[1] = 1 - (*dstdx)[1];
    else if ((*dstdx)[1] < -.5f)
        (*dstdx)[1] = -((*dstdx)[1] + 1);
    if ((*dstdy)[1] > .5)
        (*dstdy)[1] = 1 - (*dstdy)[1];
    else if ((*dstdy)[1] < -.5f)
        (*dstdy)[1] = -((*dstdy)[1] + 1);
    return st;
}

//       |` 1         x = 0
// L(x) =|  asin(πx)sin(πx/a) / (π²x²)  -a ≤ x < a
//       |, 0         
Float Lanczos(Float x, Float tau) 
{
    //a = 1.0f / tau
	x = std::abs(x);
	if (x < 1e-5f) 
        return 1;
	if (x > 1.f) 
        return 0;
	x *= Pi;
	Float s = std::sin(x * tau) / (x * tau);
	Float lanczos = std::sin(x) / x;
	return s * lanczos;
}

}
