#pragma once
#include "transform.h"
#include "spectrum.h"
#include "sampler.h"

namespace AIR
{
    class VisibilityTester;

    enum class LightFlags : int 
    {
        DeltaPosition = 1, DeltaDirection = 2, Area = 4, Infinite = 8
    };


	class Light
	{
	public:
        virtual ~Light() {}
        Light(int flags, const Transform& LightToWorld,
            /*const MediumInterface& mediumInterface, */int nSamples = 1)
            : flags(flags), nSamples(std::max(1, nSamples)),
            //mediumInterface(mediumInterface), 
            LightToWorld(LightToWorld),
        {
        
        }

        bool IsDeltaLight() const
        {
            return flags & (int)LightFlags::DeltaPosition || flags & (int)LightFlags::DeltaDirection;
        }

        //returns incident radiance from the light at a point ref.p and also 
        //returns the direction vector wi that gives the direction from which radiance is arriving.
        //ref 相交点
        //u  样本点(0-1)，采样area light要用到
        //wi 来自光源的入射光的方向
        //pdf 该光照样本概率密度函数值
        virtual Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
            Vector3f* wi, Float* pdf, VisibilityTester* vis) const = 0;

        virtual Spectrum Power() const = 0;
    public:
        const int flags;
        const int nSamples;
    protected:
        Transform LightToWorld;

	};

    class VisibilityTester 
    {
    public:
        VisibilityTester() {}
        // VisibilityTester Public Methods
        VisibilityTester(const Interaction& p0, const Interaction& p1)
            : p0(p0), p1(p1) {}
        const Interaction& P0() const { return p0; }
        const Interaction& P1() const { return p1; }
        bool Unoccluded(const Scene& scene) const;
        Spectrum Tr(const Scene& scene, Sampler& sampler) const;

    private:
        Interaction p0, p1;
    };
}
