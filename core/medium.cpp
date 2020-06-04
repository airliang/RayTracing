#include "medium.h"


namespace AIR
{
	Float HenyeyGreenstein::p(const Vector3f& wo, const Vector3f& wi) const 
	{
		//ProfilePhase _(Prof::PhaseFuncEvaluation);
		return PhaseHG(Vector3f::Dot(wo, wi), g);
	}

	Float HenyeyGreenstein::Sample_p(const Vector3f& wo, Vector3f* wi, const Point2f& sample) const
	{
		//pHG就是ω的概率密度函数
		//                      1 - g²
		//p(ω) = pHG(cosθ) = -----------------------------
		//              4π(1 + g² + 2gcosθ)^(3/2)
		//根据概率密度函数转换
		//p(θ,φ) = sinθp(ω)
		//边际概率密度求p(θ)
		//p(θ) = ∫[0, 2π]p(θ,φ)dφ
		//              sinθ(1 - g²)
		//      = -------------------------
		//        2(1 + g² + 2gcosθ)^(3/2)
		//求p(θ)的cdf
		//∫[0,θ]p(θ)dθ = u
		//解得：
		//              1 - g²
		//sqrtTerm = -------------
		//            1 - g + 2gu
		//cosθ = sqrtTerm * sqrtTerm - 1 - g²
		//φ = 2πv

		Float cosTheta;
		if (std::abs(g) < 1e-3)
		{
			//p(ω) = 1/4π
			//p(θ,φ) = sinθp(ω)
			//p(θ) = 1/4π∫[0, 2π]sinθdφ = sinθ/2
			//∫[0, θ]p(θ)dθ = u
			//1 - 1/2 cosθ = u
			cosTheta = 1 - 2 * sample[0];
		}
		else 
		{
			Float sqrTerm = (1 - g * g) / (1 - g + 2 * g * sample[0]);

			//phaseHG(cosθ) = phaseHG(-cosθ)
			cosTheta = (1 + g * g - sqrTerm * sqrTerm) / (2 * g);
		}

		Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
		Float phi = 2 * Pi * sample[1];

		Vector3f v1, v2;
		//构建坐标系
		CoordinateSystem(wo, &v1, &v2);
		*wi = SphericalDirection(sinTheta, cosTheta, phi, v1, v2, -wo);
		return PhaseHG(-cosTheta, g);
	}
}
