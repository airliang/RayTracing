#include "spotlight.h"


namespace AIR
{
	Spectrum SpotLight::Sample_Li(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, VisibilityTester* vis) const
	{
		*wi = Vector3f::Normalize(position - ref.interactPoint);
		*pdf = 1.0f;
		*vis = VisibilityTester(ref, Interaction(position, ref.time));

		return intensity * Falloff(-*wi) / Vector3f::DistanceSquare(position, ref.interactPoint);
	}

	Float SpotLight::Falloff(const Vector3f& w) const
	{
		Vector3f wInLight = Vector3f::Normalize(LightToWorld.WorldToObjectVector(w));
		Float cosTheta = wInLight.z;

		if (cosTheta > cosFalloffStart)
			return 1;

		if (cosTheta < cosTotalWidth)
			return 0;

		//在totalWidth与fallOff之间
		//呈线性衰减
		Float delta = (cosTheta - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);

		//为什么不直接delta * delta * delta * delta？
		//delta * delta * delta * delta = 
		//((delta * delta) * delta) * delta = 
		//((((delta * delta) * (1+ε)) * delta) * (1+ε)) * delta * (1+ε) = 
		//=
		//(delta² + delta²ε) * delta * (1+ε) * delta * (1+ε) = 
		//(delta³ + 2delta³ε + delta³ε²) * delta * (1+ε) = 
		//(delta^4 + 2delta^4ε + delta^4ε²)(1+ε) = 
		//delta^4 + delta^4ε + 2delta^4ε(1+ε) + delta^4ε²(1+ε)
		//所以误差是delta^4ε + 2delta^4ε(1+ε) + delta^4ε²(1+ε)

		//再来看看(delta * delta) * (delta * delta)的误差
		//(delta * delta) * (delta * delta) =
		//(((delta * delta) * (1+ε)) * ((delta * delta) * (1+ε))) * (1+ε) = 
		//(delta² * (1+ε)) * (delta² * (1+ε)) * (1+ε) = 
		//(delta² + delta²ε) * (delta² + delta²ε) * (1+ε) =
		//(delta^4 + 2delta^4ε² + delta^4ε²) * (1+ε) =
		//delta^4 + delta^4ε + 2delta^4ε²(1+ε) + delta^4ε²(1+ε)
		//两者误差一样
		return (delta * delta) * (delta * delta);
	}

	Spectrum SpotLight::Power() const
	{
		// ∫[0,2pi]∫[0,θ]sinθdθdφ
		//= ∫[0,2pi](1 - cosθ)dφ
		//= 2π(1 - cosθ)
		//由于从FalloffStart开始衰减，所以从中间值近似
		return 2.0f * Pi * (1.0f - 0.5f * (cosTotalWidth + cosFalloffStart)) * intensity;
	}
}
