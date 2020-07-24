#include "bssrdf.h"
#include "fresnelreflection.h"
#include "scene.h"
#include "robject.h"

namespace AIR
{
	Float FresnelMoment1(Float eta) 
	{
		Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
			eta5 = eta4 * eta;
		if (eta < 1)
			return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 +
			2.49277f * eta4 - 0.68441f * eta5;
		else
			return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 -
			1.27198f * eta4 + 0.12746f * eta5;
	}

	Float FresnelMoment2(Float eta) 
	{
		Float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
			eta5 = eta4 * eta;
		if (eta < 1) {
			return 0.27614f - 0.87350f * eta + 1.12077f * eta2 - 0.65095f * eta3 +
				0.07883f * eta4 + 0.04860f * eta5;
		}
		else {
			Float r_eta = 1 / eta, r_eta2 = r_eta * r_eta, r_eta3 = r_eta2 * r_eta;
			return -547.033f + 45.3087f * r_eta3 - 218.725f * r_eta2 +
				458.843f * r_eta + 404.557f * eta - 189.519f * eta2 +
				54.9327f * eta3 - 9.00603f * eta4 + 0.63942f * eta5;
		}
	}

	BSSRDF::BSSRDF(const SurfaceInteraction& po, Float eta) :
		po(po), eta(eta)
	{

	}

	SeparableBSSRDF::SeparableBSSRDF(const SurfaceInteraction& po,
		Float eta, const Material* material, TransportMode mode) 
		: material(material), mode(mode), BSSRDF(po, eta)
	{

	}

	//S ≈ (1 - Fr) * Sp * Sw
	Spectrum SeparableBSSRDF::S(const SurfaceInteraction& pi, const Vector3f& wi) const
	{
		Float Fr = FrDielectric(CosTheta(po.wo), 1, eta);
		return (1 - Fr) * Sp(pi) * Sw(wi);
	}

	//既然sw是描述方向的散射分布，散射和入射光方向有关系
	//光线进入物体会被立即散射，散射方向在物体的半球内
	//假设是均匀散射，类似漫反射，那么分布公式是：
	//          1 - Fr(η,cosθi)
	//Sw(wi) = ------------------
	//               cπ
	//c是一个归一化的项
	//∫[H²]Sw(w)cosθdw = 1
	//解得：
	//c = ∫[0,2π]∫[0,π/2](1 - Fr(η,cosθi)) / π sinθcosθdθdφ
	//  = 1 - 2∫[0,π/2]Fr(η,cosθi)sinθcosθdθ
	Spectrum SeparableBSSRDF::Sw(const Vector3f& wi) const
	{
		Float c = 1 - 2 * FresnelMoment1(1 / eta);
		return (1 - FrDielectric(CosTheta(wi), 1, eta)) / (c * Pi);
	}

	//采样S函数
	Spectrum SeparableBSSRDF::Sample_S(const Scene& scene, Float u1, const Point2f& u2,
		MemoryArena& arena, SurfaceInteraction* si,
		Float* pdf) const
	{
		Spectrum Sp = Sample_Sp(scene, u1, u2, arena, si, pdf);

		if (!Sp.IsBlack())
		{
			si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
			si->bsdf->Add(ARENA_ALLOC(arena, SeparableBSSRDFAdapter)(this));
			//入射点中的wo没用，随便给一个初始值
			si->wo = si->shading.n;
		}

		return Sp;
	}

	Spectrum SeparableBSSRDF::Sample_Sp(const Scene& scene, Float u1, const Point2f& u2,
		MemoryArena& arena, SurfaceInteraction* si,
		Float* pdf) const
	{
		//以出射点为中心，确定一个球，通过r计算一个pTarget作为射线的原点，
		//确定射线的方向（以po的法线为z方向的三个轴随机选择），然后和primitive求交
		

		//随机确定射线的方向
		//po法线的反方向占0.5，其余两方向0.25
		
		Vector3f vx, vy, vz;
		if (u1 < 0.5f)
		{
			vz = ns;
			vx = ss;
			vy = ts;
			//把u1归一化回0 - 1
			u1 *= 2;
		}
		else if (u1 < 0.75f)
		{
			vz = ss;
			vx = ts;
			vy = ns;
			u1 = (u1 - .5f) * 4;
		}
		else
		{
			vz = ss;
			vx = ts;
			vy = ns;
			u1 = (u1 - 0.75f) * 4;
		}

		//随机取一个spectrum的channel
		int ch = Clamp((int)(u1 * Spectrum::nSamples),
			0, Spectrum::nSamples - 1);
		u1 = u1 * Spectrum::nSamples - ch;

		//上面是属于前置条件的确定
		//现在开始采用r和rMax

		//Sr是一个基于平面上的分布函数
		Float r = Sample_Sr(ch, u2[0]);
		if (r < 0)
			return Spectrum(0.f);
		Float phi = 2 * Pi * u2[1];

		//rMax覆盖0.999的能量
		Float rMax = Sample_Sr(ch, 0.999f);
		if (r > rMax)
			return Spectrum(0.f);

		//通过r和phi求出pTarget
		Float l = 2.0f * sqrt(rMax * rMax - r * r);

		Interaction base;
		base.interactPoint = po.interactPoint + r * (vx * std::cos(phi) + vy * std::sin(phi)) -
			l * vz * 0.5f;
		base.time = po.time;
		Point3f pTarget = base.interactPoint + l * vz;

		//以pTarget为原点 -vz为direction构建ray
		Ray ray(pTarget, -vz);

		struct IntersectionChain 
		{
			SurfaceInteraction si;
			IntersectionChain* next = nullptr;
		};
		IntersectionChain* chain = ARENA_ALLOC(arena, IntersectionChain)();

		IntersectionChain* ptr = chain;
		int nFound = 0;
		while (true) 
		{
			Ray r = base.SpawnRayTo(pTarget);
			if (r.d == Vector3f::zero || !scene.Intersect(r, &ptr->si))
				break;

			base = ptr->si;
			if (ptr->si.primitive->GetMaterial() == material)
			{
				//如果交点是当前的bssrdf的primitive
				IntersectionChain* next = ARENA_ALLOC(arena, IntersectionChain)();
				ptr->next = next;
				ptr = next;
				nFound++;
			}
		}

		if (nFound == 0)
		{
			return Spectrum(0.f);
		}

		int nPointIndex = Clamp((int)(u1 * nFound),
			0, nFound - 1);

		while (nPointIndex-- > 0) 
			chain = chain->next;
		*si = chain->si;

		// Compute sample PDF and return the spatial BSSRDF term $\Sp$
		//这里为何要÷nFound，不太理解
		*pdf = Pdf_Sp(*si) / nFound;
		return Sp(*si);
	}

	Float SeparableBSSRDF::Pdf_Sp(const SurfaceInteraction& pi) const
	{
		Vector3f d = pi.interactPoint - po.interactPoint;
		Vector3f dLocal(Vector3f::Dot(ss, d), Vector3f::Dot(ts, d), Vector3f::Dot(ns, d));
		Vector3f nLocal(Vector3f::Dot(ss, pi.normal), Vector3f::Dot(ts, pi.normal), Vector3f::Dot(ns, pi.normal));

		//dLocal投影到三个轴的距离
		Float rProj[3] = { std::sqrt(dLocal.y * dLocal.y + dLocal.z * dLocal.z),
				   std::sqrt(dLocal.z * dLocal.z + dLocal.x * dLocal.x),
				   std::sqrt(dLocal.x * dLocal.x + dLocal.y * dLocal.y) };

		//下面为何要乘以std::abs(nLocal[axis])？
		Float pdf = 0, axisProb[3] = { .25f, .25f, .5f };
		Float chProb = 1 / (Float)Spectrum::nSamples;
		for (int axis = 0; axis < 3; ++axis)
			for (int ch = 0; ch < Spectrum::nSamples; ++ch)
				pdf += Pdf_Sr(ch, rProj[axis]) * std::abs(nLocal[axis]) * chProb *
				axisProb[axis];
		return pdf;
	}

	Spectrum TabulatedBSSRDF::Sr(Float r) const
	{
		//Sr(η,g,ρ,σt,r) = σt²Sr(η,g,ρ,1,rOptical)
		Spectrum Sr(0.f);
		//计算每个通道的Sr
		for (int i = 0; i < Spectrum::nSamples; ++i)
		{
			Float rOptical = r * sigma_t[i];

			//求出用于插值的weights，插值后面补充
			int rhoOffset, radiusOffset;
			Float rhoWeights[4], radiusWeights[4];



			//利用weight进行查表并做spline interpolation
			Float sr = 0;

			Sr[i] = sr;
		}

		Sr *= sigma_t * sigma_t;
		return Sr;
	}

	Float TabulatedBSSRDF::Sample_Sr(int ch, Float sample) const
	{
		//σt和σa都是0
		if (sigma_t[ch] == 0)
			return -1;

		//利用table做插值
	}

	Float TabulatedBSSRDF::Pdf_Sr(int ch, Float r) const
	{
		Float rOptical = r * sigma_t[ch];

		Float sr = 0, rhoEff = 0;
		if (rOptical != 0) sr /= 2 * Pi * rOptical;
		return std::max((Float)0, sr * sigma_t[ch] * sigma_t[ch] / rhoEff);
	}


	inline double C1(const double n) 
	{
		double r;
		if (n > 1.0) {
			r = -9.23372 + n * (22.2272 + n * (-20.9292 + n * (10.2291 + n * (-2.54396 + 0.254913 * n))));
		}
		else {
			r = 0.919317 + n * (-3.4793 + n * (6.75335 + n * (-7.80989 + n * (4.98554 - 1.36881 * n))));
		}
		return r / 2.0;
	}
	inline double C2(const double n) 
	{
		double r = -1641.1 + n * (1213.67 + n * (-568.556 + n * (164.798 + n * (-27.0181 + 1.91826 * n))));
		r += (((135.926 / n) - 656.175) / n + 1376.53) / n;
		return r / 3.0;
	}


	Spectrum SimpleBSSRDF::Sample_S(const Scene& scene, Float u1, const Point2f& u2,
		MemoryArena& arena, SurfaceInteraction* si,
		Float* pdf) const
	{
		
		//首先采样入射点si
		Spectrum Sp = Sample_Sp(scene, u1, u2, arena, si, pdf);

		if (!Sp.IsBlack())
		{
			si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
			si->bsdf->Add(ARENA_ALLOC(arena, SeparableBSSRDFAdapter)(this));
			//入射点中的wo没用，随便给一个初始值
			si->wo = si->shading.n;
		}

		return Bssrdf(*si, po.interactPoint, po.shading.n, po.wo);
	}

	Spectrum SimpleBSSRDF::Sample_S(const Scene& scene, Float u1, const Point2f& u2,
		const Point2f& u3, MemoryArena& arena, SurfaceInteraction* si,
		Float* pdf) const
	{

		//首先采样入射点si
		Spectrum Sp = Sample_Sp(scene, u1, u2, arena, si, pdf);

		if (!Sp.IsBlack())
		{
			si->bsdf = ARENA_ALLOC(arena, BSDF)(*si);
			si->bsdf->Add(ARENA_ALLOC(arena, SeparableBSSRDFAdapter)(this));
			//入射点中的wo没用，随便给一个初始值
			si->wo = si->shading.n;
		}

		return Bssrdf(*si, po.interactPoint, po.shading.n, po.wo);
	}

	Spectrum SimpleBSSRDF::Sp_d(const Vector3f& xoxi, const Vector3f& wr, Float r) const
	{
		const Spectrum s_tr_r = sigma_tr * r;
		const Spectrum s_tr_r_one = Spectrum(1.0) + s_tr_r;
		const Float x_dot_w = Vector3f::Dot(xoxi, wr); //x.dot(w);
		const Float r_sqr = r * r;

		const Float Cp_norm = 1.0f / (1.0f - 2.0f * C1(1.0f / eta));
		const Float Cp = (1.0f - 2.0f * C1(eta)) / 4.0f;
		const Float Ce = (1.0f - 3.0f * C2(eta)) / 2.0f;

		const Spectrum t0 = Cp_norm * (1.0f / (4.0f * Pi * Pi)) * Exp(-s_tr_r) / (r * r_sqr);
		const Spectrum t1 = Spectrum(r_sqr) / D + 3.0f * s_tr_r_one * x_dot_w;
		const Spectrum t2 = 3.0 * D * s_tr_r_one * Vector3f::Dot(wr, po.normal);//w.dot(n);
		const Spectrum t3 = (s_tr_r_one + 3.0 * D * (3.0 * s_tr_r_one + s_tr_r * s_tr_r) / r_sqr * x_dot_w) * Vector3f::Dot(xoxi, po.normal);

		return t0 * (Cp * t1 - Ce * (t2 - t3));
	}

	Spectrum SimpleBSSRDF::Bssrdf(const SurfaceInteraction& pi,
		const Vector3f wi, const Vector3f no, const Vector3f wo) const
	{
		const Spectrum de = Spectrum(2.131f) * D / Sqrt(albedo_p);

		const Float Cp = (1.0f - 2.0f * C1(eta)) / 4.0f;
		const Float Ce = (1.0f - 3.0f * C2(eta)) / 2.0f;
		const Float A = (1.0f - Ce) / (2.0f * Cp);
		// distance
		//入射点和出射点的距离
		const Vector3f xoxi = po.interactPoint - pi.interactPoint;
		const Float r = xoxi.Length();

		// modified normal
		const Vector3f ni_s = (Vector3f::Normalize(xoxi)) % (Vector3f::Normalize(pi.normal % xoxi));

		// directions of ray sources
		const Float nnt = 1.0f / eta, ddn = Vector3f::Dot(-wi, pi.normal); // -wi.dot(ni);
		//这个应该是折射方向
		const Vector3f wr = Vector3f::Normalize(wi * -nnt - pi.normal * (ddn * nnt + sqrt(1.0 - nnt * nnt * (1.0 - ddn * ddn))));
		//折射方向在内部的法线反射
		const Vector3f wv = wr - ni_s * (2.0 * Vector3f::Dot(wr, ni_s)); 

		// distance to real sources
		const Spectrum cos_beta = -Sqrt(Spectrum(r * r - Vector3f::Dot(xoxi, wr) * Vector3f::Dot(xoxi, wr)) / (Spectrum(r * r) + de * de));
		Spectrum dr;
		const Float mu0 = Vector3f::Dot(-no, wr); //-no.dot(wr);
		if (mu0 > 0.0) {
			dr = Sqrt((D * mu0) * ((D * mu0) - de * cos_beta * 2.0) + r * r);
		}
		else {
			dr = Sqrt(Spectrum(1.0f) / (3.0 * sigma_t * 3.0 * sigma_t) + r * r);
		}

		// distance to virtual source
		Spectrum AtimesDe = A * de;
		Vector3f AtimesDevec;
		AtimesDevec.x = AtimesDe[0];
		AtimesDevec.y = AtimesDe[1];
		AtimesDevec.z = AtimesDe[2];


		const Vector3f xoxv = po.interactPoint - (pi.interactPoint + (AtimesDevec * 2.0f) * ni_s[0]);
		const Float dv = xoxv.Length();

		// BSSRDF
		const Spectrum result = Sp_d(xoxi, wr, dr[0]) - Sp_d(xoxv, wv, dv);

		// clamping to zero
		return result;
	}
}
