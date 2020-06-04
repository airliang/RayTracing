#pragma once
#include "transform.h"
#include "spectrum.h"
#include "sampler.h"
#include "interaction.h"
#include "medium.h"

namespace AIR
{
    class VisibilityTester;
    class Scene;

    enum class LightFlags : int 
    {
        DeltaPosition = 1, DeltaDirection = 2, Area = 4, Infinite = 8
    };


	class Light
	{
	public:
        virtual ~Light() {}
        Light(int flags, const Transform& LightToWorld,
            const MediumInterface& mediumInterface, int nSamples = 1)
            : flags(flags), nSamples(std::max(1, nSamples)),
            //mediumInterface(mediumInterface), 
            LightToWorld(LightToWorld)
        {
        
        }

        bool IsDeltaLight() const
        {
            return flags & (int)LightFlags::DeltaPosition || flags & (int)LightFlags::DeltaDirection;
        }

        //预处理light
        //例如distantlight需要预处理中算出worldRadius
        virtual void Preprocess(const Scene& scene) {}

        //returns incident radiance from the light at a point ref.p and also 
        //returns the direction vector wi that gives the direction from which radiance is arriving.
        //ref 相交点
        //u  样本点(0-1)，采样area light要用到
        //wi 来自光源的入射光的方向
        //pdf 详细解释：
        //既然有pdf，就存在随机变量和样本空间
        //现在需要确认随机变量是什么？随机变量就是方向wi，
        //那么样本空间就是光源到ref这个点的所有的方向。
        //随机变量和样本空间确认了，就可以理解u的作用了
        //在面积光中，需要把面积的pdf转为wi的pdf，u相当于随机变量[0-1]之间
        //在某些面积中，u均匀取[0-1]²在面积里不一定是均匀的
        //delta光源为何pdf = 1？delta光源的方向只有一个，
        //那么返回的Li不需要积分，蒙特卡洛中需要Li/pdf，pdf = 1就相当于积分结果
        virtual Spectrum Sample_Li(const Interaction& ref, const Point2f& u,
            Vector3f* wi, Float* pdf, VisibilityTester* vis) const = 0;

        //返回方向wi到ref的pdf值
        //ref 受光面的一点
        //wi  受光点ref到光源上一点的方向
        //wi方向是从ref指向光源
        virtual Float Pdf_Li(const Interaction& ref, const Vector3f& wi) const = 0;

        //表示发射的光线escapes the scene bounds
        //意思是物体表面的出来的一条ray，不和场景任何模型（和光照）有相交的话，
        //就要采样环境光
        virtual Spectrum LiEnviroment(const RayDifferential& r) const
        {
            return Spectrum(0.0f);
        }

        //专门为InfiniteAreaLight提供的一个接口
        //如果ray没和任何light和primitive相交，
        //那么现在就说明这个ray escape the scenebound
        //sceneBound就是InfiniteAreaLight
        virtual Spectrum LiEscape(const RayDifferential& r) const
        {
            return Spectrum(0.f);
        }

        //返回光源的功率
        virtual Spectrum Power() const = 0;
    public:
        const int flags;

        //The nSamples parameter is used for area light sources where it 
        //may be desirable to trace multiple shadow rays to the light to compute soft shadows; 
        //default value is 1.
        const int nSamples;

        const MediumInterface mediumInterface;
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
        //判断p0到p1是否被遮挡
        bool Unoccluded(const Scene& scene) const;
        //computes the beam transmittance, Equation (11.1), 
        //the fraction of radiance transmitted along the segment between the two points. 
        //光线和medium也会有相交，当和medium相交时，发生折射后的计算函数
        Spectrum Tr(const Scene& scene, Sampler& sampler) const;

    private:
        Interaction p0, p1;
    };

    class AreaLight : public Light
    {
    public:
        AreaLight(const Transform& LightToWorld, const MediumInterface& medium, int nSamples) :
            Light((int)LightFlags::Area, LightToWorld, medium, nSamples)
        {

        }

        //返回arealight一点上发射的radiance
        //intr arealight上的一点
        //w 发射的radiance的方向（方向是从intr出发还是到达intr？）
        virtual Spectrum L(const Interaction& intr, const Vector3f& w) const = 0;
    };
}
