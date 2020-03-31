#include "infinitearealight.h"
#include "scene.h"
#include "parallelism.h"
#include "imageio.h"

namespace AIR
{
	InfiniteAreaLight::InfiniteAreaLight(const Transform& LightToWorld,
		const Spectrum& L, int nSamples, const std::string& texmap)
		: Light((int)LightFlags::Infinite, LightToWorld, nSamples)
	{
        // Read texel data from _texmap_ and initialize _Lmap_
        Point2i resolution;
        std::unique_ptr<RGBSpectrum[]> texels(nullptr);
        if (texmap != "") {
            texels = ReadImage(texmap, &resolution);
            if (texels)
                for (int i = 0; i < resolution.x * resolution.y; ++i)
                    texels[i] *= L.ToRGBSpectrum();
        }
        if (!texels) 
        {
            resolution.x = resolution.y = 1;
            texels = std::unique_ptr<RGBSpectrum[]>(new RGBSpectrum[1]);
            texels[0] = L.ToRGBSpectrum();
        }
        Lmap.reset(new Mipmap<RGBSpectrum>(resolution, texels.get()));

        // Initialize sampling PDFs for infinite area light

        // Compute scalar-valued image _img_ from environment map
        int width = 2 * Lmap->Width(), height = 2 * Lmap->Height();
        std::unique_ptr<Float[]> img(new Float[width * height]);
        float fwidth = 0.5f / std::min(width, height);
        ParallelFor(
            [&](int64_t v) {
            Float vp = (v + .5f) / (Float)height;
            //θ = pi * v
            Float sinTheta = std::sin(Pi * (v + .5f) / height);
            for (int u = 0; u < width; ++u) {
                Float up = (u + .5f) / (Float)width;
                img[u + v * width] = Lmap->Lookup(Point2f(up, vp), fwidth).y();
                //乘以sinTheta能让采样出来的radiance更均匀
                img[u + v * width] *= sinTheta;
            }
        },
            height, 32);

        // Compute sampling distributions for rows and columns of image
        distribution.reset(new Distribution2D(img.get(), width, height));
	}


	Spectrum InfiniteAreaLight::Sample_Li(const Interaction& ref, const Point2f& u,
		Vector3f* wi, Float* pdf, VisibilityTester* vis) const
	{
        //先把均匀分布的uv，转到distribution的uv
        Float mapPdf;
        Point2f uv = distribution->SampleContinuous(u, &mapPdf);
        if (mapPdf == 0)
        {
            return Spectrum(0.0f);
        }

        //compute the sample wi
        Float theta = uv[1] * Pi, phi = uv[0] * 2 * Pi;
        Float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
        Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
        *wi = LightToWorld.ObjectToWorldVector(Vector3f(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta));

        //compute the pdf of wi
        //θ = πv
        //φ = 2πu
        //jocabian = |∂θ/∂u ∂θ/∂v| = |0  π| = 2π²
        //           |∂φ/∂u ∂φ/∂v|   |2π 0|
        //p(θ,φ) = p(u, v) / 2π²
        //p(w) = p(θ,φ)/sinθ = p(u, v) / (2π²sinθ)
        if (sinTheta == 0)
            *pdf = 0;
        else
            *pdf = mapPdf / (2.0f * Pi * sinTheta);

        *vis = VisibilityTester(ref, Interaction(ref.p + *wi * (2 * worldRadius),
            ref.time));

        return Spectrum(Lmap->Lookup(uv), SpectrumType::Illuminant);
	}

	void InfiniteAreaLight::Preprocess(const Scene& scene)
	{
		scene.WorldBound().BoundingSphere(&worldCenter, &worldRadius);
	}

    Float InfiniteAreaLight::Pdf_Li(const Interaction&,
        const Vector3f& w) const 
    {
        Vector3f wi = LightToWorld.WorldToObjectVector(w);
        Float theta = SphericalTheta(wi), phi = SphericalPhi(wi);
        Float sinTheta = std::sin(theta);
        if (sinTheta == 0) 
            return 0;

        //u = φ/2π
        //v = θ/π
        //p(w) = p(θ,φ)/sinθ = p(u, v) / (2π²sinθ)
        //把p(w)传进distribution求pdf
        return distribution->Pdf(Point2f(phi * Inv2Pi, theta * InvPi)) /
            (2 * Pi * Pi * sinTheta);
    }

    Spectrum InfiniteAreaLight::Power() const
    {
        return Spectrum(worldRadius * worldRadius * Pi * Lmap->Lookup(Point2f(.5f, .5f), .5f),
            SpectrumType::Illuminant);
    }

    Spectrum InfiniteAreaLight::LiEscape(const RayDifferential& r) const
    {
        Vector3f w = Vector3f::Normalize(LightToWorld.ObjectToWorldVector(ray.d));
        Point2f st(SphericalPhi(w) * Inv2Pi,
            SphericalTheta(w) * InvPi);
        return Spectrum(Lmap->Lookup(st), SpectrumType::Illuminant);
    }
}
