#include "fresnelreflection.h"

namespace AIR
{
    Float FrDielectric(Float cosThetaI, Float etaI, Float etaT)
    {
        //首先判断是否从正方向进入
        cosThetaI = Clamp(cosThetaI, -1, 1);
        bool entering = cosThetaI > 0;
        if (!entering)
        {
            std::swap(etaI, etaT);
            cosThetaI = std::abs(cosThetaI);
        }

        //先求出cosThetaT
        //snell's law
        //ηisinθi = ηtsinθt
        //ηisqrt(1 - cosθi²) = ηtsinθt
        Float sinThetaT = etaI * sqrt(
            std::max(0.0f, 1.0f- cosThetaI * cosThetaI)) / etaT;

        //如果sinThetaT >= 1，说明入射光是全反射，因为入射光角度大于critcal angle。
        if (sinThetaT >= 1)
        {
            return 1;
        }

        Float cosThetaT = sqrt(std::max(0.0f, 1.0f - sinThetaT));

        //反射率公式：
        //Fr = 1/2(r1² + r2²)
        //r1 = (ηtcosθi - ηicosθt) / (ηtcosθi + ηicosθt)
        //r2 = (ηtcosθi - ηtcosθt) / (ηicosθi + ηtcosθt)

        Float R1 = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
        Float R2 = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
        return (R1 * R1 + R2 * R2) * 0.5f;
    }


Spectrum FrConductor(Float cosThetaI, const Spectrum &etaI,
                    const Spectrum &etaT, const Spectrum &k)
{
    cosThetaI = Clamp(cosThetaI, -1, 1);
    Spectrum eta = etaT / etaI;
    Spectrum etak = k / etaI;

    Float cosThetaI2 = cosThetaI * cosThetaI;
    Float sinThetaI2 = 1. - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etak2 = etak * etak;

    Spectrum t0 = eta2 - etak2 - sinThetaI2;
    Spectrum a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
    Spectrum t1 = a2plusb2 + cosThetaI2;
    Spectrum a = Sqrt(0.5f * (a2plusb2 + t0));
    Spectrum t2 = (Float)2 * cosThetaI * a;
    Spectrum Rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5 * (Rp + Rs);
}
}
