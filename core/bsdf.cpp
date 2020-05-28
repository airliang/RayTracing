﻿#include "bsdf.h"
#include "fresnelreflection.h"
#include "sampling.h"
#include "rng.h"

namespace AIR
{
Spectrum BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const {
    // Cosine-sample the hemisphere, flipping the direction if necessary
    *wi = CosineSampleHemisphere(u);
    if (wo.z < 0) 
        wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Spectrum BxDF::rho_hd(const Vector3f &wo, int nSamples,
                         const Point2f *samples) const
{
    //ρhd(wo) = ∫Hfr(wo, wi)|cosθi|dwi
    //蒙特卡洛估计：
    //F = 1/N∑fr(wo, wi)|cosθi|/p(ωi)
    Float pdf = 0;
    Vector3f wi;
    Spectrum F = 0.f;
    for (int i = 0; i < nSamples; ++i)
    {
        Spectrum f = Sample_f(wo, &wi, samples[i], &pdf, nullptr);
        if (pdf != 0)
        {
            F += f * AbsCosTheta(wi) / pdf;
        }
    }

    return F / nSamples;
}

Spectrum BxDF::rho_hh(int nSamples, const Point2f *samples1,
                         const Point2f *samples2) const
{
    //ρhh = 1/π∫[H]∫[H]fr(p, wo, wi)|cosθi|dwi|cosθo|dwo
    //蒙特卡洛估计：
    //F = 1/(πN)∑fr(wo, wi)|cosθi||cosθo|dwidwo/(p(wi)p(wo))

    Float pdfi = 0;
    Float pdfo = 0;
    Vector3f wi, wo;
    Spectrum F = 0.f;

    for (int i = 0; i < nSamples; ++i)
    {
        wo = UniformSampleHemisphere(samples1[i]);
        pdfo = UniformHemispherePdf();
        Spectrum f = Sample_f(wo, &wi, samples2[i], &pdfi);
        if (pdfi > 0)
        {
            F += f * AbsCosTheta(wo) * AbsCosTheta(wi) / pdfo * pdfi;
        }
    }

    return F / (nSamples * Pi);
}

Spectrum BSDF::f(const Vector3f &woW, const Vector3f &wiW,
	BxDFType flags) const {
	//ProfilePhase pp(Prof::BSDFEvaluation);
	//转成local坐标（统一z向上的左手坐标系）
	Vector3f wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
	if (wo.z == 0) 
		return 0.;
	bool reflect = Vector3f::Dot(wiW, ng) * Vector3f::Dot(woW, ng) > 0;
	Spectrum f(0.f);
	for (int i = 0; i < nBxDFs; ++i)
	{
		if (bxdfs[i]->MatchesFlags(flags) &&
			((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
			(!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
			f += bxdfs[i]->f(wo, wi);
	}
	return f;
}

Spectrum BSDF::Sample_f(const Vector3f& woWorld, Vector3f* wiWorld, const Point2f& u,
    Float* pdf, BxDFType type, BxDFType* sampledType) const
{
    //随机选择一个bxdf
    int matchingComps = NumComponents(type);
    if (matchingComps == 0)
    {
        return Spectrum(0.0f);
    }
    //计算随机bxdf的序号
    //u[0] = 1的时候就要取 min
    int comps = std::min((int)std::floor(u[0] * matchingComps), matchingComps - 1);

    BxDF* bxdf = nullptr;
    int count = comps;
    for (int i = 0; i < nBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(type) && count-- == 0)
        {
            bxdf = bxdfs[i];
            break;
        }
    }

    //remap the u[0] to [0,1)
    Point2f uRemapped = Point2f(std::min(u[0] * matchingComps - comps, OneMinusEpsilon), u[1]);

    //采样bxdf
    Vector3f wi;
    Vector3f wo = WorldToLocal(woWorld);

    if (sampledType != nullptr)
    {
        *sampledType = bxdf->type;
    }

    Spectrum f = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
    if (*pdf == 0)
    {
        return 0.0f;
    }
    *wiWorld = LocalToWorld(wi);

    //由于specular的pdf非0即1，所以这里不把specular的pdf算进去。
    //specular的pdf返回的是0
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
    {
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
                *pdf += bxdfs[i]->Pdf(wo, wi);
    }
    if (matchingComps > 1) 
        *pdf /= matchingComps;

    //由于specular的brdf函数(f)返回的是0，所以specular不在这里算
    if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1) {
        bool reflect = Vector3f::Dot(*wiWorld, ng) * Vector3f::Dot(woWorld, ng) > 0;
        f = 0.;
        for (int i = 0; i < nBxDFs; ++i)
            if (bxdfs[i]->MatchesFlags(type) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
                f += bxdfs[i]->f(wo, wi);
    }
    return f;
}

Spectrum BSDF::rho_hh(int nSamples, const Point2f *samples1,
	const Point2f *samples2, BxDFType flags) const {
	Spectrum ret(0.f);
	for (int i = 0; i < nBxDFs; ++i)
	{
		if (bxdfs[i]->MatchesFlags(flags))
			ret += bxdfs[i]->rho_hh(nSamples, samples1, samples2);
	}
	return ret;
}

Spectrum BSDF::rho_hd(const Vector3f &woWorld, int nSamples, const Point2f *samples,
	BxDFType flags) const {
	Vector3f wo = WorldToLocal(woWorld);
	Spectrum ret(0.f);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->MatchesFlags(flags))
			ret += bxdfs[i]->rho_hd(wo, nSamples, samples);
	return ret;
}

inline int BSDF::NumComponents(BxDFType flags) const 
{
	int num = 0;
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdfs[i]->MatchesFlags(flags)) 
			++num;
	return num;
}

Float BSDF::Pdf(const Vector3f& woWorld, const Vector3f& wiWorld,
    BxDFType flags) const
{
    if (nBxDFs == 0.f) 
        return 0.f;
    Vector3f wo = WorldToLocal(woWorld), wi = WorldToLocal(wiWorld);
    if (wo.z == 0) 
        return 0.;
    Float pdf = 0.f;
    int matchingComps = 0;
    for (int i = 0; i < nBxDFs; ++i)
    {
        if (bxdfs[i]->MatchesFlags(flags)) 
        {
            ++matchingComps;
            pdf += bxdfs[i]->Pdf(wo, wi);
        }
    }
    Float v = matchingComps > 0 ? pdf / matchingComps : 0.f;
    return v;
}

Float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const 
{
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}


//std::string SpecularReflection::ToString() const 
//{
//    return std::string("[ SpecularReflection R: ") + R.ToString() +
//           std::string(" fresnel: ") + fresnel->ToString() + std::string(" ]");
//}

Spectrum SpecularReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                      const Point2f &sample, Float *pdf,
                                      BxDFType *sampledType) const 
{
    // Compute perfect specular reflection direction
    //wi = -wo + 2 * Dot(wo, n) * n
    //n = (0, 0, 1)
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;

    //fr = Fr(wi) / |cosθi|
    //详细推导看：
    //http://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission.html
    //8.2.2节
    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
}

//std::string SpecularReflection::ToString() const {
//    return std::string("[ SpecularReflection R: ") + R.ToString() +
//           std::string(" fresnel: ") + fresnel->ToString() + std::string(" ]");
//}

Spectrum SpecularTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &sample, Float *pdf,
                                        BxDFType *sampledType) const {
    // Figure out which $\eta$ is incident and which is transmitted
    bool entering = CosTheta(wo) > 0;
    Float etaI = entering ? etaA : etaB;
    Float etaT = entering ? etaB : etaA;

    // Compute ray direction for specular transmission
    if (!Refract(wo, Vector3f::FaceForward(Vector3f(0, 0, 1), wo), etaI / etaT, wi))
        return 0;
    *pdf = 1;
    Spectrum ft = T * (Spectrum(1.) - fresnel.Evaluate(CosTheta(*wi)));
    // Account for non-symmetry with transmission to different medium
    //if (mode == TransportMode::Radiance) 
        ft *= (etaI * etaI) / (etaT * etaT);
    return ft / AbsCosTheta(*wi);
}

Spectrum FresnelSpecular::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      Float *pdf, BxDFType *sampledType) const
{
    //因为cosθo和cosθi相同，所以用wo
    //先算出Fresnel的反射率
    Float F = FrDielectric(CosTheta(wo), etaA, etaB);

    //小于反射率的随机数用反射，否则用折射
    if (sample.x < F)
    {
        //先算入射方向
        *wi = Vector3f(-wo.x, -wo.y, wo.z);

        //
        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_REFLECTION);
        *pdf = F;
        return F * R / AbsCosTheta(*wi);
    }
    else
    {
        //判断是否从正面入射
        bool entering = CosTheta(wo) > 0;
        Float etaI = entering ? etaA : etaB;
        Float etaT = entering ? etaB : etaA;

        //求出射方向
        if (!Refract(wo, Vector3f::FaceForward(Vector3f::forward, wo), etaI / etaT, wi))
        {
            return 0;
        }
        Spectrum ft = T * (1 - F);

        //为何这样写，还没搞明白？
        if (mode == TransportMode::Radiance)
            ft *= (etaI * etaI) / (etaT * etaT);

        if (sampledType)
            *sampledType = BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION);

        *pdf = 1 - F;
        return ft / AbsCosTheta(*wi);
    }
}

Spectrum LambertianReflection::f(const Vector3f &wo, const Vector3f &wi) const 
{
    return R * InvPi;
}

Spectrum OrenNayar::f(const Vector3f& wo, const Vector3f& wi) const 
{
	Float sinThetaI = SinTheta(wi);
	Float sinThetaO = SinTheta(wo);
	// Compute cosine term of Oren-Nayar model
	Float maxCos = 0;
	if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
		Float sinPhiI = SinPhi(wi), cosPhiI = CosPhi(wi);
		Float sinPhiO = SinPhi(wo), cosPhiO = CosPhi(wo);
		Float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
		maxCos = std::max((Float)0, dCos);
	}

	// Compute sine and tangent terms of Oren-Nayar model
	Float sinAlpha, tanBeta;
	if (AbsCosTheta(wi) > AbsCosTheta(wo)) {
		sinAlpha = sinThetaO;
		tanBeta = sinThetaI / AbsCosTheta(wi);
	}
	else {
		sinAlpha = sinThetaI;
		tanBeta = sinThetaO / AbsCosTheta(wo);
	}
	return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}

Spectrum MicrofacetReflection::f(const Vector3f &wo, const Vector3f &wi) const {
	Float cosThetaO = AbsCosTheta(wo), cosThetaI = AbsCosTheta(wi);
	Vector3f wh = wi + wo;
	// Handle degenerate cases for microfacet reflection
	if (cosThetaI == 0 || cosThetaO == 0) 
		return Spectrum(0.);
	if (wh.x == 0 && wh.y == 0 && wh.z == 0) return Spectrum(0.);
	wh = Vector3f::Normalize(wh);
	// For the Fresnel call, make sure that wh is in the same hemisphere
	// as the surface normal, so that TIR is handled correctly.
	Spectrum F = fresnel->Evaluate(Vector3f::Dot(wi, Vector3f::FaceForward(wh, Vector3f(0, 0, 1))));
	return R * distribution->D(wh) * distribution->G(wo, wi) * F /
		(4 * cosThetaI * cosThetaO);
}

Spectrum MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &u, Float *pdf,
                                        BxDFType *sampledType) const 
{
    // Sample microfacet orientation $\wh$ and reflected direction $\wi$
    if (wo.z == 0) 
        return 0.;
    //首先根据分布采样法线
    Vector3f wh = distribution->Sample_wh(wo, u);
    //再由法线计算入射光
    *wi = Reflect(wo, wh);
    if (!SameHemisphere(wo, *wi)) 
        return Spectrum(0.f);

    // Compute PDF of _wi_ for microfacet reflection
    //假设以ωo为法线
    //φi=φh，从发射来计算θi = 2θh
    //dωi = sinθidθidφi = sin2θh d2θh dφh
    // dωh   sinθhdθhdφh          1
    // ---- = ----------------- = --------
    // dωi   sin2θhd2θhdφi     4cosθh
    //推出：p(ωi) = p(ωh) / 4(ωo·ωh)
    *pdf = distribution->Pdf(wo, wh) / (4 * Vector3f::Dot(wo, wh));
    return f(wo, *wi);
}

Float MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) 
        return 0;
    Vector3f wh = Vector3f::Normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4 * Vector3f::Dot(wo, wh));
}

Spectrum MicrofacetTransmission::f(const Vector3f& wo, const Vector3f& wi) const
{
    //     D(wh)G(wo,wi)(1 - Fr(wo)) |wi·wh||wo·wh|
    //fr = ------------------------- ----------------
    //      ((wo·wh) + η(wi·wo))²  cosθo cosθh
    if (SameHemisphere(wo, wi)) 
        return 0;  // transmission only

    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
	Vector3f wh = Vector3f::Normalize(wo + wi * eta);
	if (wh.z < 0) 
        wh = -wh;

    
    Float dotWiWh = Vector3f::Dot(wi, wh);
    Float dotWoWh = Vector3f::Dot(wo, wh);
    Spectrum F = fresnel.Evaluate(dotWoWh);
    Float sqrtDenom = dotWoWh + eta * dotWiWh;
    return  distribution->D(wi) * distribution->G(wo, wi) * (Spectrum(1.0f) - F) *
        std::abs(dotWiWh) * std::abs(dotWoWh) / (sqrtDenom * sqrtDenom * CosTheta(wo) * CosTheta(wi));
}

Spectrum MicrofacetTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                          const Point2f &u, Float *pdf,
                                          BxDFType *sampledType) const 
{
    if (wo.z == 0) 
        return 0.;
    //在折射中，wh微表面法线的采样和反射一样
    Vector3f wh = distribution->Sample_wh(wo, u);
    //求折射系数比例
    Float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    //eta = ηi/ηt
    //wt = eta(-wi) + [eta(wi·wh) - cosθt]wh
    if (!Refract(wo, wh, eta, wi)) 
        return 0;
    //求出射wi的pdf
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float MicrofacetTransmission::Pdf(const Vector3f &wo,
                                  const Vector3f &wi) const 
{
    if (SameHemisphere(wo, wi)) 
        return 0;
    // Compute $\wh$ from $\wo$ and $\wi$ for microfacet transmission
    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = Vector3f::Normalize(wo + wi * eta);

    // Compute change of variables _dwh\_dwi_ for microfacet transmission
    Float sqrtDenom = Vector3f::Dot(wo, wh) + eta * Vector3f::Dot(wi, wh);
    Float dwh_dwi =
        std::abs((eta * eta * Vector3f::Dot(wi, wh)) / (sqrtDenom * sqrtDenom));
    return distribution->Pdf(wo, wh) * dwh_dwi;
}

Spectrum FresnelConductor::Evaluate(Float cosThetaI) const
{
    //导体没有折射，所以取cosThetaI的绝对值
    return FrConductor(std::abs(cosThetaI), etaI, etaT, k);
}

Spectrum FresnelDielectric::Evaluate(Float cosThetaI) const
{
    return FrDielectric(cosThetaI, etaI, etaT);
}

}