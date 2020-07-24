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
	//Lo(po, wo) = ��[A]��[H] S(po, wo, pi, wi)Li(pi, wi)|cos��i|dwidA
	class BSSRDF
	{
	public:
		BSSRDF(const SurfaceInteraction& po, Float eta);
		virtual ~BSSRDF()
		{

		}

		//BSSRDF scattering function S(po, wo, pi, wi)
		//po, wo�����Աconst SurfaceInteraction& po����
		//@param pi ����������潻��
		//@param wi �����ķ���
		virtual Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi) = 0;

		//����S����
		//@param scene ��ǰ����
		//@u1 ���ڲ���spectrum channel���ཻ�����index
		//@u2 ���ڲ���r�ͦգ���ɢ�����õ�
		//@param arena �������ڵ��ڴ������
		//@param si��u2�������������õ�
		//@pdf   ������si��pdf
		virtual Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const = 0;

		virtual Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const = 0;
	protected:
		//����Ľ���
		const SurfaceInteraction& po;
		//medium��������
		Float eta;
	};

	//��ͨ��sphere��plane����subsurface light transport�ͺ����ѣ�
	//���ο�������״��ģ�ͣ�SeparableBSSRDF����Ϊ��֧�ָ�����״
	//To retain the ability to support general Shapes
	//Separable�ĸ����Ƿ��룬����ʲô��
	//S�����ǣ�S(pi, po, wi, wo)
	//��ô������ǿռ�ͷ��򣬼���sp(pi, po) sw(wi, wo)ģ��
	//�����湫ʽģ�⣺
	//S(po, wo, pi, wi) = (1 - Fr) * sp(pi, po) * sw(wi, wo)
	class SeparableBSSRDF : public BSSRDF
	{
	public:
		SeparableBSSRDF(const SurfaceInteraction& po, Float eta,
			const Material* material, TransportMode mode);

		//S(po, wo, pi, wi) �� (1 - Fr(cos��o))Sp(po, pi)Sw(wi)
		//��ֳɿռ�ͷ������������
		Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi) const;

		//������ص�ɢ�亯��
		//���ط���w��ɢ��ֲ�
		//          1 - Fr(��,cos��i)
		//Sw(wi) = ------------------
		//                c��
		Spectrum Sw(const Vector3f& wi) const;

		//profile term
		//������������ģ��ɢ��Ŀռ�ֲ�������
		//pi��po�����ж���radiance����
		Spectrum Sp(const SurfaceInteraction& pi) const
		{
			return Sr(Vector3f::Distance(po.interactPoint, pi.interactPoint));
		}

		//����S����
		//@param scene ��ǰ����
		//@u1 ���ڲ���spectrum channel���ཻ�����index
		//@u2 ���ڲ���r�ͦգ���ɢ�����õ�
		//@param arena �������ڵ��ڴ������
		//@param si��u2�������������õ�
		//@pdf   ������si��pdf
		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Spectrum Sample_S(const Scene& scene, Float u1, const Point2f& u2,
			const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const
		{
			return Sample_S(scene, u1, u2, arena, si, pdf);
		}

		//����Sp����
		//����ͬSample_S����
		Spectrum Sample_Sp(const Scene& scene, Float u1, const Point2f& u2,
			MemoryArena& arena, SurfaceInteraction* si,
			Float* pdf) const;

		Float Pdf_Sp(const SurfaceInteraction& si) const;

		//���ڴα���ɢ����������ģ������
		//һ���þ���ȥģ��������֮��ĵ���ֲ�
		//@param d ����
		//��Ҫ��Sr�ǲ���һ����һ����ֵ��
		virtual Spectrum Sr(Float d) const = 0;

		virtual Float Pdf_Sr(int ch, Float distance) const = 0;

		//����Sr�е�distance
		//@param ch spectrum��sample index
		//@param sample ������� 0-1
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

		//�Ѿ���d�͵���ɢ��albedo����Ա���rho��Ϊ������
		//ͨ����ֵ�ķ�����table��õ�radiance�ķֲ�
		Spectrum Sr(Float d) const;
		Float Pdf_Sr(int ch, Float distance) const;
		Float Sample_Sr(int ch, Float sample) const;
	private:
		const BSSRDFTable& table;

		//scattering extinction
		Spectrum sigma_t;
		//scattering albedo����ʾ��ɢ��ı���
		Spectrum rho;


	};

	//����SeparableBSSRDF::Sw(wi).
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
		//@param xoxi ����㵽�����ķ���
		//@param wr   ������ڽ����ڲ������䷽��
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
