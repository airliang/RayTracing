#pragma once

#include "bsdf.h"

namespace AIR
{
	class Material;
	class Scene;

	struct BSSRDFTable 
	{
		// BSSRDFTable Public Data
		const int nRhoSamples, nRadiusSamples;
		std::unique_ptr<Float[]> rhoSamples, radiusSamples;
		std::unique_ptr<Float[]> profile;
		std::unique_ptr<Float[]> rhoEff;
		std::unique_ptr<Float[]> profileCDF;

		// BSSRDFTable Public Methods
		BSSRDFTable(int nRhoSamples, int nRadiusSamples);
		inline Float EvalProfile(int rhoIndex, int radiusIndex) const 
		{
			return profile[rhoIndex * nRadiusSamples + radiusIndex];
		}
	};

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

		//采样S函数
		//@param scene 当前场景
		//@u1 用于采样spectrum channel，相交入射点index
		//@u2 用于采样r和φ，即散射作用点
		//@param arena 作用域内的内存分配器
		//@param si，u2采样出来的作用点
		//@pdf   采样出si的pdf
		virtual Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const = 0;

		virtual Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const = 0;
	protected:
		//出射的交点
		const SurfaceInteraction& po;
		//medium的折射率
		Float eta;
	};

	//普通的sphere和plane，做subsurface light transport就很困难，
	//更何况复杂形状的模型，SeparableBSSRDF就是为了支持复杂形状
	//To retain the ability to support general Shapes
	//Separable的概念是分离，分离什么？
	//S函数是：S(pi, po, wi, wo)
	//那么分离的是空间和方向，即用sp(pi, po) sw(wi, wo)模拟
	//用下面公式模拟：
	//S(po, wo, pi, wi) = (1 - Fr) * sp(pi, po) * sw(wi, wo)
	class SeparableBSSRDF : public BSSRDF
	{
	public:
		SeparableBSSRDF(const SurfaceInteraction& po, Float eta,
			const Material* material, TransportMode mode);

		//S(po, wo, pi, wi) ≈ (1 - Fr(cosθo))Sp(po, pi)Sw(wi)
		//拆分成空间和方向的两个函数
		Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi) const;

		//方向相关的散射函数
		//返回方向w的散射分布
		//          1 - Fr(η,cosθi)
		//Sw(wi) = ------------------
		//                cπ
		Spectrum Sw(const Vector3f& wi) const;

		//profile term
		//初步看是用于模拟散射的空间分布，例如
		//pi到po有能有多少radiance到达
		Spectrum Sp(const SurfaceInteraction& pi) const
		{
			return Sr(Vector3f::Distance(po.interactPoint, pi.interactPoint));
		}

		//采样S函数
		//@param scene 当前场景
		//@u1 用于采样spectrum channel，相交入射点index
		//@u2 用于采样r和φ，即散射作用点
		//@param arena 作用域内的内存分配器
		//@param si，u2采样出来的作用点
		//@pdf   采样出si的pdf
		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const
		{
			return Sample_S(scene, u1, u2, arena, si, pdf);
		}

		//采样Sp函数
		//参数同Sample_S函数
		Spectrum Sample_Sp(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Float Pdf_Sp(const SurfaceInteraction& si) const;

		//由于次表面散射在物体里模拟困难
		//一般用距离去模拟两个点之间的到达分布
		//@param d 距离
		//重要：Sr是不是一个归一化的值？
		virtual Spectrum Sr(Float d) const = 0;

		virtual Float Pdf_Sr(int ch, Float distance) const = 0;

		//采用Sr中的distance
		//@param ch spectrum的sample index
		//@param sample 随机变量 0-1
		virtual Float Sample_Sr(int ch, Float sample) const = 0;

	private:
		// SeparableBSSRDF Private Data
		const Vector3f ns;
		const Vector3f ss, ts;
		const Material* material;
		const TransportMode mode;
	};

	class TabulatedBSSRDF : public SeparableBSSRDF
	{
	public:
		TabulatedBSSRDF(const SurfaceInteraction& po, const Material* material,
			TransportMode mode, Float eta, const Spectrum& sigma_a,
			const Spectrum& sigma_s, const BSSRDFTable& table)
			: SeparableBSSRDF(po, eta, material, mode), table(table) 
		{
			sigma_t = sigma_a + sigma_s;
			for (int c = 0; c < Spectrum::nSamples; ++c)
				rho[c] = sigma_t[c] != 0 ? (sigma_s[c] / sigma_t[c]) : 0;
		}

		//把距离d和单词散射albedo即成员里的rho作为参数，
		//通过插值的方法从table里得到radiance的分布
		Spectrum Sr(Float d) const;
		Float Pdf_Sr(int ch, Float distance) const;
		Float Sample_Sr(int ch, Float sample) const;
	private:
		const BSSRDFTable& table;

		//scattering extinction
		Spectrum sigma_t;
		//scattering albedo，表示有散射的比例
		Spectrum rho;


	};

	//代表SeparableBSSRDF::Sw(wi).
	class SeparableBSSRDFAdapter : public BxDF 
	{
	public:
		// SeparableBSSRDFAdapter Public Methods
		SeparableBSSRDFAdapter(const SeparableBSSRDF* bssrdf)
			: BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), bssrdf(bssrdf) {}
		Spectrum f(const Vector3f& wo, const Vector3f& wi) const 
		{
			Spectrum f = bssrdf->Sw(wi);
			// Update BSSRDF transmission term to account for adjoint light
			// transport
			//if (bssrdf->mode == TransportMode::Radiance)
			//	f *= bssrdf->eta * bssrdf->eta;
			return f;
		}
		std::string ToString() const { return "[ SeparableBSSRDFAdapter ]"; }

	private:
		const SeparableBSSRDF* bssrdf;
	};

	class SimpleBSSRDF : public SeparableBSSRDF
	{
	public:
		SimpleBSSRDF(const SurfaceInteraction& po, Float eta, const Material* material
			, const Spectrum& sigma_a, const Spectrum& sigma_s) : sigma_a(sigma_a),
			sigma_s(sigma_s), 
			SeparableBSSRDF(po, eta, material, TransportMode::Radiance) 
		{
			sigma_t = sigma_a + sigma_s;
			Spectrum g = Spectrum::black;
			sigma_sp = sigma_s * (Spectrum(1.0f) - g);
			sigma_tp = sigma_sp + sigma_a;
			D = Spectrum(1.0f) / (Spectrum(3.0f) * sigma_tp);
			sigma_tr = Sqrt(sigma_a / D);
			albedo_p = sigma_sp / sigma_tp;
		}
		Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi);

		
		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Spectrum Sr(Float d) const
		{
			return Spectrum::black;
		}

	private:
		Spectrum Bssrdf(const SurfaceInteraction& pi,
			const Vector3f xo, const Vector3f no, const Vector3f wo) const;

		//
		//@param xoxi 出射点到入射点的方向
		//@param wr   入射点在介质内部的折射方向
		//
		Spectrum Sp_d(const Vector3f& xoxi, const Vector3f& wr, Float r) const;


	private:
		Spectrum sigma_a, sigma_s;
		Spectrum sigma_t;
		Spectrum sigma_sp, sigma_tp;
		Spectrum albedo_p;
		Spectrum D;
		Spectrum sigma_tr;
	};
}
