#pragma once
#include "geometry.h"
#include "interaction.h"
#include "transform.h"

namespace AIR
{

	class Shape
	{
	public:
		enum ShapeType
		{
			shape_sphere,
			shape_cylinder,
		};
	public:
		Shape(Transform* pTransform) : mTransform(pTransform)
		{

		}
		virtual ~Shape()
		{

		}

		virtual Bounds3f ObjectBound() const = 0;

		//返回该shape的面积
		virtual Float Area() const = 0;

		//采样shape上的一个点
		//u 均匀随机变量0-1
		//pdf 该点在shape上的pdf值
		virtual Interaction Sample(const Point2f& u, Float* pdf) const = 0;

		//在shape上任意采样一点的pdf
		virtual Float Pdf(const Interaction&) const 
		{ 
			return 1 / Area(); 
		}

		//采样shape上的一点，但这点必须和ref对应的点可见
		//ref 采样出来的结果必须和ref可见
		//u 均匀随机变量0-1
		//pdf 该点在shape上的pdf值
		virtual Interaction Sample(const Interaction& ref, const Point2f& u,
			Float* pdf) const;

		//专门为light提供的pdf
		//由于只有可见立体角内采样，
		//概率密度函数将从面积转换到立体角范围内
		//该函数表示某交点ref，方向wi的pdf是什么
		//由于采样shape的面积的pdf已知，立体角的pdf需要从面积转过去
		virtual Float Pdf(const Interaction& ref, const Vector3f& wi) const;

		//ray is in object space
		//函数只判断shape是否和ray有相交
		virtual bool IntersectP(const Ray &ray) const
		{
			return Intersect(ray, nullptr, nullptr);
		}

		//解释一下这个Transform为何用指针，
		//假如有一个triangleMesh，有上千个Triangle，
		//这些Triangle都是共享一个Transform的
		Transform* mTransform;

		//判断射线是否相交
		//r 世界坐标系下的射线
		//tHit 相交后的射线到交点的距离
		//isect 相交后返回的交点信息
		virtual bool Intersect(const Ray &r, Float *tHit, Interaction *isect) const = 0;
	protected:
		
	};
}
