#include "homogeneousmedium.h"
#include "interaction.h"
#include <algorithm>

namespace AIR
{
	Spectrum HomogeneousMedium::Tr(const Ray& ray, Sampler& sampler) const
	{
		//����pbrt 11.1�Ĺ�ʽ��
		//Tr(p->p') = e^(-��[0,d]��t(p + t��, ��)dt)   //ͨ����΢�ַ��̵õ�
		//optical thicknes ��(p->p') = -��[0,d]��t(p + t��, ��)dt
		//����homogeneous medium��˥��ϵ����t��һ��������
		//��(p->p') = -��[0,d]��tdt = -��td
		//Tr(p->p') = e^(-��td)
		//����d = ray.tMax * ray.d.Length()
		return Exp(-sigma_t * std::min(ray.tMax * ray.d.Length(), MaxFloat));
	}

	Spectrum HomogeneousMedium::Sample(const Ray& ray, Sampler& sampler, 
		MemoryArena& arena, MediumInteraction* mi) const
	{
		//����˥��ϵ����ÿ����������һ�������������Ȳ���������Ӧ��channel
		//
		int channel = std::min((int)(sampler.Get1D() * Spectrum::nSamples), Spectrum::nSamples - 1);

		//����t�������Ǿ��룩
		//�ֲ���ָ���ֲ�
		//cdf = 1 - e^(-��t d)
		// d = -ln(1 - ��) / ��t
		float distance = -log(1 - sampler.Get1D()) / sigma_t[channel];

		//����ray.d���ǹ�һ������������distanceҪת��tMax�ķ�Χ��
		float t = std::min (distance / ray.d.Length(), ray.tMax);

		bool sampledMedium = t < ray.tMax;
		if (sampledMedium)
			*mi = MediumInteraction(ray(t), -ray.d, ray.time, this,
				ARENA_ALLOC(arena, HenyeyGreenstein)(g));
		else
		{
			int a = 0;
		}

		// Compute the transmittance and sampling density
		//��ǰ�����������͸����Tr
		//Tr = e^(-��t * t)
		Spectrum Tr = Exp(-sigma_t * std::min(t, MaxFloat) * ray.d.Length());

		//��pdf����
		//pdf = cdf����
		//pdf(t) = ��t e^(-��t t)
		//����ÿ��channel��pdf��һ��������ȡƽ��ֵ
		//pdf(t) = 1/n��(��ti * e^(-��tit)) = 1/n��(��ti * Tr_i)

		//������Surface�ĸ�����
		//   1 - ��[0, t_max]pdf(t)dt
		// = 1 - ��[0, t_max]sigma_t * e^(-sigma_t * t)dt
		// = 1 - sigma_t * ��[0, t_max]e^(-sigma_t * t)dt
		// = 1 - (1 - e^(-sigma_t * t)) = e^(-sigma_t * t)
		Spectrum pdfAll = sampledMedium ? Tr * sigma_t : Tr;
		float pdf = 0;
		for (int i = 0; i < Spectrum::nSamples; ++i)
		{
			pdf += pdfAll[i];
		}
		pdf /= Spectrum::nSamples;
		if (pdf == 0) {
			//CHECK(Tr.IsBlack());
			pdf = 1;
		}

		//����ý��ʵĹ���Ҫ����˥��ϵ����s������ray.o�Ĺ�����Ҫ�ٳ���Tr
		//Lo = Li * ��s * Tr;
		Spectrum beta_med = Tr * sigma_s / pdf;
		//������㲻�ǽ���ʱ������Ҫ����˥��ϵ����s
		Spectrum beta_surf = Tr / pdf;
		return sampledMedium ? beta_med : beta_surf;
	}
}
