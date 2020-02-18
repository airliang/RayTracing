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
}