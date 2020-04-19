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

		//���ظ�shape�����
		virtual Float Area() const = 0;

		//����shape�ϵ�һ����
		//u �����������0-1
		//pdf �õ���shape�ϵ�pdfֵ
		virtual Interaction Sample(const Point2f& u, Float* pdf) const = 0;

		//��shape���������һ���pdf
		virtual Float Pdf(const Interaction&) const 
		{ 
			return 1 / Area(); 
		}

		//����shape�ϵ�һ�㣬���������ref��Ӧ�ĵ�ɼ�
		//ref ���������Ľ�������ref�ɼ�
		//u �����������0-1
		//pdf �õ���shape�ϵ�pdfֵ
		virtual Interaction Sample(const Interaction& ref, const Point2f& u,
			Float* pdf) const;

		//ר��Ϊlight�ṩ��pdf
		//����ֻ�пɼ�������ڲ�����
		//�����ܶȺ����������ת��������Ƿ�Χ��
		//�ú�����ʾĳ����ref������wi��pdf��ʲô
		//���ڲ���shape�������pdf��֪������ǵ�pdf��Ҫ�����ת��ȥ
		virtual Float Pdf(const Interaction& ref, const Vector3f& wi) const;

		//ray is in object space
		//�ĺ���ֻ�ж�shape�Ƿ��ray���ཻ
		virtual bool IntersectP(const Ray &ray) const
		{
			Float tHit = ray.tMax;
			Interaction isect;
			return false;
			//return Intersect(ray, &tHit, &isect);
		}

		Transform* mTransform;

		//�ж������Ƿ��ཻ
		//r ��������ϵ�µ�����
		//tHit �ཻ������ߵ�����ľ���
		//isect �ཻ�󷵻صĽ�����Ϣ
		virtual bool Intersect(const Ray &r, Float *tHit, Interaction *isect) const = 0;
	protected:
		
	};
}
