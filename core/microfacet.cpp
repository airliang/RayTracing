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
}