#pragma once
#include "geometry.h"

namespace AIR
{
//微表面分布函数类
class MicrofacetDistribution {
public:
    // MicrofacetDistribution Public Methods
    virtual ~MicrofacetDistribution()
    {

    }
    //返回wh方向的微分面积
    virtual Float D(const Vector3f &wh) const = 0;

    //背向的w方向和总共可见微表面的比例
    //Λ(ω) = A-(ω) / (A+(ω) - A-(ω)) = A-(ω) / cosθ
    virtual Float Lambda(const Vector3f &w) const = 0;

    //返回总共可见的微表面和面向w方向的微表面的比例
    Float G1(const Vector3f &w) const 
    {
        //    if (Dot(w, wh) * CosTheta(w) < 0.) return 0.;
        return 1 / (1 + Lambda(w));
    }

    //返回的是wo和wi两个方向都可见的比例
    //注意，G1(ωo)和G1(ωi)是有依赖关系的，即互不独立
    //当ωo = ωi时，G(ωo, ωi) = G1(ωo) = G1(ωi)
    //所以当ωo和ωi越接近时，两者的关系越紧密
    virtual Float G(const Vector3f &wo, const Vector3f &wi) const 
    {
        return 1 / (1 + Lambda(wo) + Lambda(wi));
    }
    virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const = 0;
    
    //返回的是投影到宏观表面dA的概率密度
    //D是微表面的分布函数，D(ωh)n·h是投影到dA的分布
    //∫[H]D(ωh)n·h = 1
    Float Pdf(const Vector3f &wo, const Vector3f &wh) const;
    //virtual std::string ToString() const = 0;

protected:
    // MicrofacetDistribution Protected Methods
    MicrofacetDistribution(bool sampleVisibleArea)
        : sampleVisibleArea(sampleVisibleArea) {}

    // MicrofacetDistribution Protected Data
    //默认是true,
    //true的时候采样可见的微表面法线
    //false的时候只是用于测试，采样整个分布
    const bool sampleVisibleArea;
};

class BeckmannDistribution : public MicrofacetDistribution 
{
public:
    // BeckmannDistribution Public Methods
    static Float RoughnessToAlpha(Float roughness) 
    {
        roughness = std::max(roughness, (Float)1e-3);
        Float x = std::log(roughness);
        return 1.62142f + 0.819955f * x + 0.1734f * x * x +
               0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
    }
    BeckmannDistribution(Float alphax, Float alphay, bool samplevis = true)
        : MicrofacetDistribution(samplevis), alphax(alphax), alphay(alphay) {}

    //按各向异性的公式：
    //         e^(-tan²θh(cos²φh/αx² + sin²φh/αy²))
    // D(wh) = -----------------------------------------
    //          παxαy (cosθh)^4
    Float D(const Vector3f &wh) const;

    //采样微表面法线
    Vector3f Sample_wh(const Vector3f &wo, const Point2f &u) const;
    //std::string ToString() const;

private:
    // BeckmannDistribution Private Methods
    //Λ(w) = 1/2(erf(a) - 1 + e^(-a²)/a·sqrt(π))
    //a = 1/(αtanθ)
    Float Lambda(const Vector3f &w) const;

    // BeckmannDistribution Private Data
    const Float alphax, alphay;
};

}