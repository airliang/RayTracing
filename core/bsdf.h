#pragma once
#include "interaction.h"
#include "spectrum.h"
#include "microfacet.h"

namespace AIR
{
// BSDF Inline Functions
inline Float CosTheta(const Vector3f &w) 
{ 
    return w.z; 
}
inline Float Cos2Theta(const Vector3f &w) 
{ 
    return w.z * w.z; 
}
inline Float AbsCosTheta(const Vector3f &w) 
{ 
    return std::abs(w.z); 
}
inline Float Sin2Theta(const Vector3f &w) 
{
    return std::max((Float)0, (Float)1 - Cos2Theta(w));
}

inline Float SinTheta(const Vector3f &w) 
{ 
    return std::sqrt(Sin2Theta(w)); 
}

inline Float TanTheta(const Vector3f &w) 
{ 
    return SinTheta(w) / CosTheta(w); 
}

inline Float Tan2Theta(const Vector3f &w) 
{
    return Sin2Theta(w) / Cos2Theta(w);
}

inline Float CosPhi(const Vector3f &w) 
{
    Float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}

inline Float SinPhi(const Vector3f &w) {
    Float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}

inline Float Cos2Phi(const Vector3f &w) { return CosPhi(w) * CosPhi(w); }

inline Float Sin2Phi(const Vector3f &w) { return SinPhi(w) * SinPhi(w); }

inline Float CosDPhi(const Vector3f &wa, const Vector3f &wb) {
    return Clamp(
        (wa.x * wb.x + wa.y * wb.y) / std::sqrt((wa.x * wa.x + wa.y * wa.y) *
                                                (wb.x * wb.x + wb.y * wb.y)),
        -1, 1);
}

//ωo = ω⊥ + ω∥, ω∥ = cosθn = (n·ωo)n
//ω⊥ = ωo - ω∥ = ωo - (n·ωo)n
//ωr = -ω⊥ + ω∥ = -(ωo - (n·ωo)n) + (n·ωo)n = -ωo + 2(n·ωo)ω
inline Vector3f Reflect(const Vector3f &wo, const Vector3f &n) 
{
    return -wo + 2 * Vector3f::Dot(wo, n) * n;
}

inline bool Refract(const Vector3f &wi, const Vector3f &n, Float eta,
                    Vector3f *wt) {
    // Compute $\cos \theta_\roman{t}$ using Snell's law
    Float cosThetaI = Vector3f::Dot(n, wi);
    Float sin2ThetaI = std::max(Float(0), Float(1 - cosThetaI * cosThetaI));
    Float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1) return false;
    Float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp) 
{
    return w.z * wp.z > 0;
}

class Fresnel 
{
public:
    // Fresnel Interface
    virtual ~Fresnel();
    virtual Spectrum Evaluate(Float cosI) const = 0;
    //virtual std::string ToString() const = 0;
};

/*
inline std::ostream &operator<<(std::ostream &os, const Fresnel &f) 
{
    os << f.ToString();
    return os;
}
*/

class FresnelConductor : public Fresnel 
{
public:
    // FresnelConductor Public Methods
    Spectrum Evaluate(Float cosThetaI) const;
    FresnelConductor(const Spectrum &etaI, const Spectrum &etaT,
                     const Spectrum &k)
        : etaI(etaI), etaT(etaT), k(k) {}
    //std::string ToString() const;

private:
    Spectrum etaI, etaT, k;
};

class FresnelDielectric : public Fresnel 
{
public:
    // FresnelDielectric Public Methods
    Spectrum Evaluate(Float cosThetaI) const;
    FresnelDielectric(Float etaI, Float etaT) : etaI(etaI), etaT(etaT) {}
    //std::string ToString() const;

private:
    Float etaI, etaT;
};


// BSDF Declarations
enum BxDFType {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
               BSDF_TRANSMISSION,
};

class BxDF {
  public:
    // BxDF Interface
    virtual ~BxDF() {}
    BxDF(BxDFType type) : type(type) {}
    bool MatchesFlags(BxDFType t) const 
    { 
        return (type & t) == type; 
    }

    //返回BxDF在确定入射和出射方向下的bxdf值（比例）
    //wo 出射方向
    //wi 入射方向
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const = 0;

    //在未知入射方向的情况下，返回出射方向下的BxDF的值（比例）
    //wo出射方向
    //wi 入射方向（随机的或是specular的入射）
    //sample 样本值，用于采样wi的
    //pdf 返回的wo方向的概率密度函数值
    //sampledType bxdf类型
    virtual Spectrum Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;

    //该函数返回的是对wo方向有贡献的半球内入射方向的bxdf的积分值ρhd
	//实际上是返回总共出射的radiance的光量分布(归一化的)
    //ρhd = ∫[H]f(p, wo, wi)|cosθi|dwi
    //wo 出射方向
    //nSamples 样本数量
    //samples 样本列表，用于采样多个wi的
    virtual Spectrum rho_hd(const Vector3f &wo, int nSamples,
                         const Point2f *samples) const;
    
    //返回的是半球内所有出射方向的ρhd的积分再除以π
    //理解为半球内的平均反射率
    //ρhd = 1/π∫[H]∫[H]f(p, wo, wi)|cosθi|dwi|cosθo|dwo
    virtual Spectrum rho_hh(int nSamples, const Point2f *samples1,
                         const Point2f *samples2) const;
    virtual Float Pdf(const Vector3f &wo, const Vector3f &wi) const;
    //virtual std::string ToString() const = 0;

    // BxDF Public Data
    const BxDFType type;
};

//这里切线空间还是z向上，x向右，y向屏幕的左手坐标系

class BSDF 
{
public:
    // BSDF Public Methods
    BSDF(const Interaction &si, Float eta = 1)
        : eta(eta),
          //ns(si.shading.n),
		  geometryNormal(si.normal),
          //ss(Vector3f::Normalize(si.shading.dpdu)),
          //ts(Vector3f::Cross(ns, ss)) 
          ns(si.normal),
          ss(si.dpdu),
          ts(Vector3f::Cross(ns, ss)) {}
    void Add(BxDF *b) {
        //CHECK_LT(nBxDFs, MaxBxDFs);
        bxdfs[nBxDFs++] = b;
    }
    int NumComponents(BxDFType flags = BSDF_ALL) const;
    Vector3f WorldToLocal(const Vector3f &v) const {
        return Vector3f(Vector3f::Dot(v, ss), Vector3f::Dot(v, ts), Vector3f::Dot(v, ns));
    }
    Vector3f LocalToWorld(const Vector3f &v) const {
		return Vector3f(ss.x * v.x + ts.x * v.y + ns.x * v.z,
			ss.y * v.x + ts.y * v.y + ns.y * v.z,
			ss.z * v.x + ts.z * v.y + ns.z * v.z);
    }
    Spectrum f(const Vector3f &woW, const Vector3f &wiW,
               BxDFType flags = BSDF_ALL) const;
    Spectrum rho_hh(int nSamples, const Point2f *samples1, const Point2f *samples2,
                 BxDFType flags = BSDF_ALL) const;
    Spectrum rho_hd(const Vector3f &wo, int nSamples, const Point2f *samples,
                 BxDFType flags = BSDF_ALL) const;
    Spectrum Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType type = BSDF_ALL,
                      BxDFType *sampledType = nullptr) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi,
              BxDFType flags = BSDF_ALL) const;
    //std::string ToString() const;

    // BSDF Public Data
    const Float eta;

private:
    // BSDF Private Methods
    ~BSDF() {}

    // BSDF Private Data 暂时不要shading的normal和tangent
    //const Vector3f ns, ng;
    //const Vector3f ss, ts;
    //is geometry's normal tangent not shading
	const Vector3f geometryNormal;   
    const Vector3f ns;   //shading normal (z)
    const Vector3f ss;   //shading s (x)
    const Vector3f ts;   //shading tangent = cross(n, s) (y)
    int nBxDFs = 0;
    static constexpr int MaxBxDFs = 8;
    BxDF *bxdfs[MaxBxDFs];
    //friend class MixMaterial;
};

class SpecularReflection : public BxDF 
{
public:
    // SpecularReflection Public Methods
    SpecularReflection(const Spectrum &R, Fresnel *fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
          R(R),
          fresnel(fresnel) {}
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const 
    {
        return Spectrum(0.f);
    }
    Spectrum Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      Float *pdf, BxDFType *sampledType) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const 
    { 
        return 0; 
    }
    //std::string ToString() const;

  private:
    // SpecularReflection Private Data
    const Spectrum R;
    const Fresnel *fresnel;
};


class SpecularTransmission : public BxDF 
{
public:
    // SpecularTransmission Public Methods
    SpecularTransmission(const Spectrum &T, Float etaA, Float etaB)//, TransportMode mode)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
          T(T),
          etaA(etaA),
          etaB(etaB),
          fresnel(etaA, etaB)
          //mode(mode) 
          {}
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const {
        return Spectrum(0.f);
    }
    Spectrum Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample,
                      Float *pdf, BxDFType *sampledType) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const 
    { 
        return 0; 
    }
    //std::string ToString() const;

private:
    // SpecularTransmission Private Data
    const Spectrum T;
    const Float etaA, etaB;
    const FresnelDielectric fresnel;
    //const TransportMode mode;
};

class LambertianReflection : public BxDF {
public:
    // LambertianReflection Public Methods
    LambertianReflection(const Spectrum &R)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R) 
        {}

	//ρhd = ∫[H]f(p, wo, wi) |cosθi| dwi = R，该函数返回的是R
	//上面公式的f(p, wo, wi) = kR，因为每个立体角的反射是一个常量R，
	//所以f一定是一个R的缩放值
	//ρhd = ∫[H]kR|cosθi|dwi = kR∫[0,2π]∫[0,π/2]|cosθ|sinθdθdφ
	//     = kR∫[0,2π]∫[0,π/2](1/2)sin2θdθdφ
	//     = kR∫[0,2π](1/4)-cos2θ|[0,π/2]dφ
	//     = kR/2∫[0,2π]dφ = kRπ= R
	//k = 1 / π
	//所以f(p, wo, wi) = kR = R / π
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    Spectrum rho_hd(const Vector3f &, int, const Point2f *) const 
    { 
        return R; 
    }
    Spectrum rho_hh(int, const Point2f *, const Point2f *) const 
    { 
        return R; 
    }
    //std::string ToString() const;

private:
    // LambertianReflection Private Data
    const Spectrum R;
};

//微表面模型Torrance–Sparrow Model
class MicrofacetReflection : public BxDF 
{
public:
    // MicrofacetReflection Public Methods
    MicrofacetReflection(const Spectrum &R,
                         MicrofacetDistribution *distribution, Fresnel *fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_GLOSSY)),
          R(R),
          distribution(distribution),
          fresnel(fresnel) {}

	//               D(ωh) G(ωo,ωi) Fr(ωo)
	// f(ωo,ωi) = ---------------------------
	//                   4 cosθo cosθi
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    Spectrum Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType *sampledType) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const;
    //std::string ToString() const;

private:
    // MicrofacetReflection Private Data
    const Spectrum R;
    const MicrofacetDistribution *distribution;
    const Fresnel *fresnel;
};

class MicrofacetTransmission : public BxDF 
{
public:
    // MicrofacetTransmission Public Methods
    MicrofacetTransmission(const Spectrum &T,
                           MicrofacetDistribution *distribution, Float etaA,
                           Float etaB, TransportMode mode)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)),
          T(T),
          distribution(distribution),
          etaA(etaA),
          etaB(etaB),
          fresnel(etaA, etaB),
          mode(mode) {}
    Spectrum f(const Vector3f &wo, const Vector3f &wi) const;
    Spectrum Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                      Float *pdf, BxDFType *sampledType) const;
    Float Pdf(const Vector3f &wo, const Vector3f &wi) const;
    //std::string ToString() const;

private:
    // MicrofacetTransmission Private Data
    const Spectrum T;
    const MicrofacetDistribution *distribution;
    const Float etaA, etaB;
    const FresnelDielectric fresnel;
    const TransportMode mode;
};

}
