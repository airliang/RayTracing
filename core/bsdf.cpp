#include "bsdf.h"

namespace AIR
{
Spectrum BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const {
    // Cosine-sample the hemisphere, flipping the direction if necessary
    *wi = CosineSampleHemisphere(u);
    if (wo.z < 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

Float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    return SameHemisphere(wo, wi) ? AbsCosTheta(wi) * InvPi : 0;
}


Spectrum SpecularReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                      const Point2f &sample, Float *pdf,
                                      BxDFType *sampledType) const 
{
    // Compute perfect specular reflection direction
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
    return fresnel->Evaluate(CosTheta(*wi)) * R / AbsCosTheta(*wi);
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
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1;
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
    if (!Refract(wo, Vector3f::Faceforward(Vector3f(0, 0, 1), wo), etaI / etaT, wi))
        return 0;
    *pdf = 1;
    Spectrum ft = T * (Spectrum(1.) - fresnel.Evaluate(CosTheta(*wi)));
    // Account for non-symmetry with transmission to different medium
    //if (mode == TransportMode::Radiance) 
        ft *= (etaI * etaI) / (etaT * etaT);
    return ft / AbsCosTheta(*wi);
}

Spectrum LambertianReflection::f(const Vector3f &wo, const Vector3f &wi) const 
{
    return R * InvPi;
}

Spectrum MicrofacetReflection::Sample_f(const Vector3f &wo, Vector3f *wi,
                                        const Point2f &u, Float *pdf,
                                        BxDFType *sampledType) const 
{
    // Sample microfacet orientation $\wh$ and reflected direction $\wi$
    if (wo.z == 0) 
        return 0.;
    Vector3f wh = distribution->Sample_wh(wo, u);
    *wi = Reflect(wo, wh);
    if (!SameHemisphere(wo, *wi)) 
        return Spectrum(0.f);

    // Compute PDF of _wi_ for microfacet reflection
    *pdf = distribution->Pdf(wo, wh) / (4 * Vector3f::Dot(wo, wh));
    return f(wo, *wi);
}

Float MicrofacetReflection::Pdf(const Vector3f &wo, const Vector3f &wi) const {
    if (!SameHemisphere(wo, wi)) 
        return 0;
    Vector3f wh = Vector3f::Normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4 * Vector3f::Dot(wo, wh));
}

Spectrum MicrofacetTransmission::Sample_f(const Vector3f &wo, Vector3f *wi,
                                          const Point2f &u, Float *pdf,
                                          BxDFType *sampledType) const 
{
    if (wo.z == 0) 
        return 0.;
    Vector3f wh = distribution->Sample_wh(wo, u);
    Float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    if (!Refract(wo, wh, eta, wi)) 
        return 0;
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

}