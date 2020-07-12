#pragma once
#include "interaction.h"

namespace AIR
{
	//BSSRDF scattering function:
	//Lo(po, wo) = ∫[A]∫[H] S(po, wo, pi, wi)Li(pi, wi)|cosθi|dwidA
	class BSSRDF
	{
	public:
		BSSRDF(const SurfaceInteraction& po, Float eta);
		virtual ~BSSRDF()
		{

		}

		//BSSRDF scattering function S(po, wo, pi, wi)
		//po, wo在类成员const SurfaceInteraction& po中了
		//@param pi 入射光的与表面交点
		//@param wi 入射光的方向
		virtual Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi) = 0;
	protected:
		//出射的交点
		const SurfaceInteraction& po;
		//medium的折射率
		Float eta;
	};

	//普通的sphere和plane，做subsurface light transport就很困难，
	//更何况复杂形状的模型，SeparableBSSRDF就是为了支持复杂形状
	//To retain the ability to support general Shapes
	//
	class SeparableBSSRDF : public BSSRDF
	{
	public:
		SeparableBSSRDF(const SurfaceInteraction& po, Float eta,
			const Material* material, TransportMode mode)
		{

		}

		//S(po, wo, pi, wi) ≈ (1 - Fr(cosθo))Sp(po, pi)Sw(wi)
		//
		Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi);

		Spectrum Sw(const Vector3f& w) const;

		//profile term
		//初步看是用于模拟散射的空间分布，例如
		//pi到po有能有多少radiance到达
		Spectrum Sp(const SurfaceInteraction& pi) const 
		{
			return Sr(Vector3f::Distance(po.p, pi.p));
		}

		//由于次表面散射在物体里模拟困难
		//一般用距离去模拟两个点之间的到达分布
		//@param d 距离
		virtual Spectrum Sr(Float d) const = 0;
	}

	class TabulatedBSSRDF : public SeparableBSSRDF
	{
	public:
		Spectrum Sr(Float d) const;
	};
}
