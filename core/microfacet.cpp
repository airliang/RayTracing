#include "microfacet.h"
#include "bsdf.h"

namespace AIR
{

    Float MicrofacetDistribution::Pdf(const Vector3f &wo, const Vector3f &wh) const
    {
        if (sampleVisibleArea)
            return D(wh) * G1(wo) * Vector3f::AbsDot(wo, wh) / AbsCosTheta(wo);
        else
            return D(wh) * AbsCosTheta(wh);
    }
    
    Float BeckmannDistribution::D(const Vector3f &wh) const
    {
        Float tan2Theta = Tan2Theta(wh);
        if (std::isinf(tan2Theta)) 
            return 0.;
        const Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
        Float e =
            (Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) *
            tan2Theta;
        return 1 / (Pi * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
    }

    Vector3f BeckmannDistribution::Sample_wh(const Vector3f& wo, const Point2f& u) const
    {
        //p(φ) = 1/2π
        //P(φ) = ∫[0,φ']1/2πdφ = φ'/2π
        //u.y = φ'/2π
        //        2e^(-tan²θ/α²)sinθ
        //p(θ) = ---------------------
        //             α²cos³θ
        //推导请看：https://blog.csdn.net/air_liang1212/article/details/104965866
        //概率累积函数：
        //P(θ) = ∫[0,θ']p(θ)dθ  = 1 - e^(-tan²θ/α²)
        //反函数法：
        //u.x = 1 - e^(-tan²θ/α²)
        //-tan²θ/α² = log(1 - u.x)
        //tan²θ = -α²log(1 - u.x)
        Float theta;
        Float phi;
        Float tanTheta2;
        if (sampleVisibleArea)
        {

        }
        else
        {
            
            if (alphax == alphay)   //各向同性
            {
                Float logUx = std::log(1 - u.x);
                if (std::isinf(logUx))
                {
                    logUx = 0;
                }
                tanTheta2 = -alphax * alphax * logUx;
                phi = Pi * 2.0f * u.y;
            }
            else
            {
                //各向异性的情况
                //tan(φ) = αy/αx
                phi = std::atan(alphay / alphax *
                    std::tan(2 * Pi * u[1] + 0.5f * Pi));
                if (u[1] > 0.5f)
                    phi += Pi;
                //P(θ) = 1 - e^(-tan²θ/α²)
                //α² = cos²φ/αx² + sin²φ/αy² 代入上式得：
                //P(θ) = 1 - e^(-tan²θ/(cos²φ/αx² + sin²φ/αy²))
                //u.x = 1 - e^(-tan²θ/(cos²φ/αx² + sin²φ/αy²))
                //tan²θ = -(cos²φ/αx² + sin²φ/αy²)log(1 - u.x)
                Float logUx = std::log(1 - u.x);
                Float cosphi = std::cos(phi);
                
                Float sinphi = std::sin(phi);
                Float alphax2 = alphax * alphax;
                Float alphay2 = alphay * alphay;
                tanTheta2 = -logUx /
                    (cosphi * cosphi / alphax + sinphi * sinphi / alphay2);

            }
        }
        //tan² = sin²/cos² = (1 - cos²)/cos²
        Float cosTheta = 1 / std::sqrt(1 + tanTheta2);
        Float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
        Vector3f wh = SphericalDirection(sinTheta, cosTheta, phi);
        if (!SameHemisphere(wo, wh)) 
            wh = -wh;

        return wh;
    }

    Float BeckmannDistribution::Lambda(const Vector3f& w) const
    {
        Float absTanTheta = std::abs(TanTheta(w));
        if (std::isinf(absTanTheta)) 
            return 0.;
        Float alpha = std::sqrt(Cos2Phi(w) * alphax * alphax +
                Sin2Phi(w) * alphay * alphay);

        Float a = 1 / (alpha * absTanTheta);
        if (a >= 1.6f)
            return 0;
        return (1 - 1.259f * a + 0.396f * a * a) /
            (3.535f * a + 2.181f * a * a);
    }

	static void TrowbridgeReitzSample11(Float cosTheta, Float U1, Float U2,
		Float* slope_x, Float* slope_y) 
    {
		// special case (normal incidence)
		if (cosTheta > .9999) {
			Float r = sqrt(U1 / (1 - U1));
			Float phi = 6.28318530718 * U2;
			*slope_x = r * cos(phi);
			*slope_y = r * sin(phi);
			return;
		}

		Float sinTheta =
			std::sqrt(std::max((Float)0, (Float)1 - cosTheta * cosTheta));
		Float tanTheta = sinTheta / cosTheta;
		Float a = 1 / tanTheta;
		Float G1 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

		// sample slope_x
		Float A = 2 * U1 / G1 - 1;
		Float tmp = 1.f / (A * A - 1.f);
		if (tmp > 1e10) tmp = 1e10;
		Float B = tanTheta;
		Float D = std::sqrt(
			std::max(Float(B * B * tmp * tmp - (A * A - B * B) * tmp), Float(0)));
		Float slope_x_1 = B * tmp - D;
		Float slope_x_2 = B * tmp + D;
		*slope_x = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

		// sample slope_y
		Float S;
		if (U2 > 0.5f) {
			S = 1.f;
			U2 = 2.f * (U2 - .5f);
		}
		else {
			S = -1.f;
			U2 = 2.f * (.5f - U2);
		}
		Float z =
			(U2 * (U2 * (U2 * 0.27385f - 0.73369f) + 0.46341f)) /
			(U2 * (U2 * (U2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
		*slope_y = S * z * std::sqrt(1.f + *slope_x * *slope_x);

		//CHECK(!std::isinf(*slope_y));
		//CHECK(!std::isnan(*slope_y));
	}

	static Vector3f TrowbridgeReitzSample(const Vector3f& wi, Float alpha_x,
		Float alpha_y, Float U1, Float U2) {
		// 1. stretch wi
		Vector3f wiStretched =
			Normalize(Vector3f(alpha_x * wi.x, alpha_y * wi.y, wi.z));

		// 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
		Float slope_x, slope_y;
		TrowbridgeReitzSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

		// 3. rotate
		Float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
		slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
		slope_x = tmp;

		// 4. unstretch
		slope_x = alpha_x * slope_x;
		slope_y = alpha_y * slope_y;

		// 5. compute normal
		return Normalize(Vector3f(-slope_x, -slope_y, 1.));
	}

	Vector3f TrowbridgeReitzDistribution::Sample_wh(const Vector3f& wo,
		const Point2f& u) const {
		Vector3f wh;
		if (!sampleVisibleArea) {
			Float cosTheta = 0, phi = (2 * Pi) * u[1];
			if (alphax == alphay) {
				Float tanTheta2 = alphax * alphax * u[0] / (1.0f - u[0]);
				cosTheta = 1 / std::sqrt(1 + tanTheta2);
			}
			else {
				phi =
					std::atan(alphay / alphax * std::tan(2 * Pi * u[1] + .5f * Pi));
				if (u[1] > .5f) phi += Pi;
				Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
				const Float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
				const Float alpha2 =
					1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
				Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
				cosTheta = 1 / std::sqrt(1 + tanTheta2);
			}
			Float sinTheta =
				std::sqrt(std::max((Float)0., (Float)1. - cosTheta * cosTheta));
			wh = SphericalDirection(sinTheta, cosTheta, phi);
			if (!SameHemisphere(wo, wh)) wh = -wh;
		}
		else {
			bool flip = wo.z < 0;
			wh = TrowbridgeReitzSample(flip ? -wo : wo, alphax, alphay, u[0], u[1]);
			if (flip) wh = -wh;
		}
		return wh;
	}

	Float TrowbridgeReitzDistribution::D(const Vector3f& wh) const {
		Float tan2Theta = Tan2Theta(wh);
		if (std::isinf(tan2Theta)) return 0.;
		const Float cos4Theta = Cos2Theta(wh) * Cos2Theta(wh);
		Float e =
			(Cos2Phi(wh) / (alphax * alphax) + Sin2Phi(wh) / (alphay * alphay)) *
			tan2Theta;
		return 1 / (Pi * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
	}

	Float TrowbridgeReitzDistribution::Lambda(const Vector3f& w) const {
		Float absTanTheta = std::abs(TanTheta(w));
		if (std::isinf(absTanTheta)) return 0.;
		// Compute _alpha_ for direction _w_
		Float alpha =
			std::sqrt(Cos2Phi(w) * alphax * alphax + Sin2Phi(w) * alphay * alphay);
		Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
		return (-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2;
	}
}