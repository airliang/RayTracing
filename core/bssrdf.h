#pragma once
#include "interaction.h"

namespace AIR
{
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
	protected:
		//����Ľ���
		const SurfaceInteraction& po;
		//medium��������
		Float eta;
	};

	//��ͨ��sphere��plane����subsurface light transport�ͺ����ѣ�
	//���ο�������״��ģ�ͣ�SeparableBSSRDF����Ϊ��֧�ָ�����״
	//To retain the ability to support general Shapes
	//
	class SeparableBSSRDF : public BSSRDF
	{
	public:
		SeparableBSSRDF(const SurfaceInteraction& po, Float eta,
			const Material* material, TransportMode mode)
		{

		}

		//S(po, wo, pi, wi) �� (1 - Fr(cos��o))Sp(po, pi)Sw(wi)
		//
		Spectrum S(const SurfaceInteraction& pi, const Vector3f& wi);

		Spectrum Sw(const Vector3f& w) const;

		//profile term
		//������������ģ��ɢ��Ŀռ�ֲ�������
		//pi��po�����ж���radiance����
		Spectrum Sp(const SurfaceInteraction& pi) const 
		{
			return Sr(Vector3f::Distance(po.p, pi.p));
		}

		//���ڴα���ɢ����������ģ������
		//һ���þ���ȥģ��������֮��ĵ���ֲ�
		//@param d ����
		virtual Spectrum Sr(Float d) const = 0;
	}

	class TabulatedBSSRDF : public SeparableBSSRDF
	{
	public:
		Spectrum Sr(Float d) const;
	};
}
