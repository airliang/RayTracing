#pragma once
#include "../RayTracing.h"
#include "geometry.h"
#include <vector>

namespace AIR
{
	struct Distribution1D
	{
		//f 分段函数每段的值
		//n 分段函数的长度，即有多少段
		//每段的长度相等
		Distribution1D(const Float* f, int n) : func(f, f + n)
			, cdf(n + 1)
		{
			//计算func的积分
			//c = ∫f(x)dx = ∑[0,N-1]f(xi)Δ
			//如果分段函数的x长度是1，那么Δ = 1/n
			//c = ∑[0,N-1]f(xi)/n
			Float c = 0;
			for (int i = 0; i < n; ++i)
			{
				c += func[i] / n;
			}

			//计算cdf
			//pdf p(x)的积分是1，所以p(x) = f(x)/c
			cdf[0] = 0;
			Float inverseC = 1.0f / c;
			for (int i = 1; i < n + 1; ++i)
			{
				cdf[i] = cdf[i - 1] + func[i] * inverseC / n;
			}
		}

		//采样对应的随机变量
		//先求u落在哪个cdf上
		//然后根据该cdf的斜率求出对应的值
		//off 第几个cdf
		Float SampleContinuous(Float u, Float* pdf, int* off = nullptr) const;

		//采样离散的随机
		int SampleDiscrete(Float u, Float* pdf = nullptr, Float* uRemapped = nullptr) const;

		Float DiscretePDF(int index) const 
		{
			return funcInt != 0 ? func[index] / (funcInt * Count()) : 0;
		}

		int Count() const
		{
			return func.size();
		}

		//分段函数，每一段的函数值是一个常数
		std::vector<Float> func;
		//分段函数对应的概率累积函数
		//从0开始，到最后是1，所以cdf的长度是func.size + 1
		std::vector<Float> cdf;

		//分段函数的积分
		Float funcInt;
	};

	//把这个Distribution2D理解成一个函数f(x,y)
	//那么他的概率密度函数是p(x,y)服从均匀分布
	//f(x,y)的积分是If
	//If = ∫∫f(u,v)dudv = 1/(nu*nv)∑[0,nu-1]∑[0,nv-1]f(ui,vi)
	//p(u,v) = f(u,v) / If
	//p(v) = ∫[0,v]p(u,v)du = 1/nu * ∑[0,nu-1]f(ui,v) / If
	//     
	//由于If是一个固定值，所以构建p(v)并没有把 /If放到里面计算
	//p(u|v) = p(u,v) / p(v) = f(u,v) / (1/nu * ∑[0,nu-1]f(ui,v))
	//这里把If给约掉了
	struct Distribution2D
	{
		//func 2D函数
		//nu nv 长度
		Distribution2D(const Float* func, int nu, int nv)
		{
			//求func的积分
			Float If = 0;
			for (int u = 0; u < nu; u++)
			{
				for (int v = 0; v < nv; ++v)
				{
					If += func[u * v];
				}
			}
		}

		//可以理解成nv个关于v的分段函数
		std::vector<Float> pdfV;
	};

	//均匀采样单位半球上的立体角
	//u 0-1的随机变量
	Vector3f UniformSampleHemisphere(const Point2f& u)
	{
		//∫[hemisphere]p(ω)dω = 1
		//p(ω)是均匀分布的
		//求出：p(ω) = 1/(2π)
		//x = rsinθcosφ, y = rsinθsinφ，z = rcosθ
		//       sinθcosφ rcosθcosφ -rsinθsinφ   ∂x/∂(r,θ,φ)
		// JT =  sinθsinφ  cosθsinφ  rsinθcosφ
		//       cosθ      -rsinθ      0 
		//行列式|JT| = r²sinθ
		//p(x,y,z) = p(r,θ,φ)/(r²sinθ)，由于取的是单位球表面的xyz
		//r恒等于1  
		//p(x,y,z) = p(1,θ,φ)/sinθ = p(θ,φ)/sinθ
		//p(ω) = p(θ,φ)/sinθ

	    //p(θ,φ) = sinθp(ω) = sinθ/2π
		//先计算p(θ)边际概率密度
		//p(θ) = ∫[0, 2π]p(θ,φ)dφ = sinθ/2π[0, 2π] = sinθ
		//p(φ|θ) = p(θ,φ)/p(θ) = 1/2π

		//计算概率累积函数
		//P(θ) = ∫[0, θ]p(θ)dθ = 1 - cosθ= 1 - u.x   => u.x = cosθ
		//P(φ|θ) = ∫[0, φ]p(φ|θ)dφ = φ/2π = u.y      => φ = 2πu.y 
		Float z = u.x;
		Float sintheta = std::sqrt(1 - u.x * u.x);
		Float phi = 2 * Pi * u.y;
		Float cosPhi = std::cos(phi);
		
		return Vector3f(sintheta * cosPhi, sintheta * std::sin(phi), z);
	}

	Float UniformHemispherePdf() 
	{
		return Inv2Pi;
	}

	//均匀采样单位球上的立体角
	Vector3f UniformSampleSphere(const Point2f& u)
	{
		//ξ1 = 2 * u.x - 1
		//ξ2 = 2 * u.y - 1
		//∫[0,2π]∫[0,π]p(ω)sinθdθdφ = 1
		//∫[0,2π]2p(ω)dφ = 1
		//p(ω) = 1 / (4π)
		//p(θ,φ) = sinθp(ω) = sinθ/4π
		//先计算p(θ)边际概率密度
		//p(θ) = ∫[0, 2π]p(θ,φ)dφ = sinθ/2
		//p(φ|θ) = p(θ,φ)/p(θ) = 1/2π

		//计算概率累积函数
		//P(θ) = ∫[0, θ]p(θ)dθ = (1 - cosθ)/2 = u.x    => cosθ = 1 - 2u.x
		//P(φ|θ) = ∫[0, φ]p(φ|θ)dφ = φ/2π = u.y      => φ = 2πu.y 

		Float z = 1 - 2 * u.x;
		Float sintheta = std::sqrt(std::max(0.0f, (1.0f - z * z)));
		Float phi = Pi * 2.0f * u.y;
		Float cosPhi = std::cos(phi);

		return Vector3f(sintheta * cosPhi, sintheta * std::sin(phi), z);
	}

	Float UniformSpherePdf()
	{
		return Inv4Pi;
	}

	//均匀采样一个圆盘
	//u [0,1]之间的均匀随机变量
	//return 随机的r和θ
	//p(x,y) = 1/π  (单位圆盘的面积是π)
	//求p(r,θ)
	//极坐标：x = rcosθ, y = rsinθ
	// Jt = ∂x/∂r ∂x/∂θ = cosθ -rsinθ
	//      ∂y/∂r ∂y/∂θ   sinθ  rcosθ
	//Jt的行列式|Jt| = rcos²θ + rsin²θ = r
	//p(r,θ) = |Jt|p(x,y) = r/π
	Point2f UniformSampleDisk(const Point2f& u)
	{
		//求边际概率密度p(r) = ∫[0,2π]p(r,θ)dθ = 1/π(r)[0, 2π] = 2r
		//p(θ) = p(r,θ)/p(r) = 1/2π

		//P(r) = ∫[0,r]p(r)dr = r² = u.x
		//P(θ) = ∫[0,θ]p(θ)dθ = θ/2π = u.y

		Float r = std::sqrt(u.x);
		Float theta = Pi * 2.0f * u.y;
		return r * Point2f(std::cos(theta), std::sin(theta));
	}

	//同心圆盘采样
	Point2f ConcentricSampleDisk(const Point2f& u)
	{
		//mapping u to [-1,1]
		Point2f u1(u.x * 2.0f - 1, u.y * 2.0f - 1);

		if (u1.x == 0 && u1.y == 0)
			return Point2f::zero;

		//r = x
		//θ = y/x * π/4
		//最后返回x,y
		//x = rcosθ, y = rsinθ
		Float theta, r;
		if (std::abs(u1.x) > std::abs(u1.y))
		{
			r = u1.x;
			theta = u1.y / u1.x * PiOver4;
		}
		else
		{
			//这里要反过来看，就是把视野选择90度
			r = u1.y;
			theta = PiOver2 - u1.x / u1.y * PiOver4;
		}
		return r * Point2f(std::cos(theta), std::sin(theta));
	}

	//
	//由于radiance是以cosθ作为权重，所以越接近顶点的radiance贡献越大
	//如果希望收敛更快，pdf最好是和贡献差不多
	//可以近似地认为：p(ω)∝cosθ
	//假设p(ω) = kcosθ
	//∫[Hemisphere]p(ω)dω = ∫[Hemisphere]kcosθsinθdθdφ
	//= ∫[Hemisphere]ksinθdsinθdφ
    //= ∫[0, 2π]k/2dφ = kπ= 1
	// k = 1/π
	//p(ω) = cosθ/π
	//根据∫p(ω)dω = ∫p(θ, φ)dθdφ => p(ω)sinθ = p(θ,φ)
	//p(θ,φ) = cosθsinθ/π
	inline Vector3f CosineSampleHemisphere(const Point2f& u)
	{
		//先采样单位圆盘的点
		//得到r φ
		Point2f rphi = ConcentricSampleDisk(u);

		//把p(r,φ)转到p(θ,φ)
		//r = sinθ
		//Jt = ∂r/∂θ  ∂r/∂φ  =  cosθ 0
		//     ∂φ/∂θ ∂φ/∂φ     0    1
		//p(r,φ) = p(θ,φ)/|Jt| = sinθ/π = r/π

		//由于随机变量r,φ所对应的随机变量θ,φ的pdf p(θ,φ)刚好满足
		//p(ω) = cosθ/π，所以直接拿r,φ做随机变量就能满足p(ω)的概率密度函数
		Float z = std::sqrt(1.0f - rphi.x * rphi.x - rphi.y * rphi.y);
		return Vector3f(rphi.x, rphi.y, z);
	}

	//采样三角形上的一点
	//u 0-1的均匀随机变量
	//三角形的重心坐标是u,v,w，由于w = 1 - u -v
	//直接用(u,v)就能表示整个三角形内的所有点
	Point2f UniformSampleTriangle(const Point2f& u)
	{
		//三角形面积是1/2，p(u,v) = 1/(1/2) = 2
		//p(u) = ∫[0, 1-u]p(u,v)dv = 2(1 - u)
		//p(v|u) = p(u,v)/p(u) = 1/(1 - u)

		//P(u) = ∫[0,u]p(u')du' = 1 - (1 - u)² = 1 - ξ1  = 1 - u.x
		//(1 - u)² = u.x
		//P(v|u) = ∫[0,v]p(v'|u)dv' = v/(1 - u) = ξ2 = u.y

		//u = 1 - sqrt(u.x)
		//v = u.y(1 - u) = u.ysqrt(u.x)

		Float u1 = std::sqrt(u.x);
		return Point2f(1.0f - u1, u1 * u.y);
	}
}
