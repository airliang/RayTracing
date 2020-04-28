#pragma once

#include "matrix.h"
#include "interaction.h"

namespace AIR
{

	class Transform
	{
	public:

		Transform() : mMatrixDirty(true)
		{
			mScale = Vector3f::one;
		}

		Transform(const Vector3f& position, const Quaternion& rotation, const Vector3f& scale) : 
			mPosition(position), mRotation(rotation), mScale(scale), mMatrixDirty(true)
		{

		}

		void SetPosition(const Vector3f& position);

		Vector3f Position() const
		{
			return mPosition;
		}

		void SetScale(const Vector3f& scale);

		Vector3f Scale() const
		{
			return mScale;
		}

		Quaternion Rotation() const
		{
			return mRotation;
		}
		
		void SetRotation(const Quaternion& rotation);

		const Matrix4x4& LocalToWorld() const;
		const Matrix4x4& WorldToLocal() const;

		Vector3f ObjectToWorldPoint(const Vector3f& point, Vector3f* absError = nullptr) const;
		Vector3f ObjectToWorldPoint(const Vector3f& point, const Vector3f& ptError, Vector3f* absError = nullptr) const;
		Vector3f ObjectToWorldVector(const Vector3f& vec, Vector3f* absError = nullptr) const;
		Vector3f ObjectToWorldNormal(const Vector3f& normal) const;
		Ray ObjectToWorldRay(const Ray& ray) const;
		Bounds3f ObjectToWorldBound(const Bounds3f& bound) const;
		//Vector3f ObjectToWorldPoint(const Vector3f& point, const Vector3f& ptError, Vector3f* absError) const;
		Interaction ObjectToWorldInteraction(const Interaction& isect) const;

		bool SwapsHandedness() const 
		{
			Float det =
				mat._M[0][0] * (mat._M[1][1] * mat._M[2][2] - mat._M[1][2] * mat._M[2][1]) -
				mat._M[0][1] * (mat._M[1][0] * mat._M[2][2] - mat._M[1][2] * mat._M[2][0]) +
				mat._M[0][2] * (mat._M[1][0] * mat._M[2][1] - mat._M[1][1] * mat._M[2][0]);
			return det < 0;
		}

		//世界坐标系下的ray转到object space下
		//ray world space下的ray
		//oError ray的o点转换后的误差
		//dError ray的d向量转换后的误差
		Ray WorldToObjectRay(const Ray& ray, Vector3f* oError, Vector3f* dError) const;
		Vector3f WorldToObjectPoint(const Vector3f& point, Vector3f* absError = nullptr) const;
		Vector3f WorldToObjectVector(const Vector3f& vec, Vector3f* absError = nullptr) const;
		Vector3f WorldToObjectNormal(const Vector3f& normal) const;
		Bounds3f WorldToObjectBound(const Bounds3f& bound) const;

		bool operator==(const Transform& t) const 
		{
			return t.mPosition == mPosition && t.mScale == mScale && t.mRotation == mRotation;
		}
		bool operator!=(const Transform& t) const 
		{
			return t.mPosition != mPosition && t.mScale != mScale && t.mRotation != mRotation;
		}

		static Transform MakeTransform(const Vector3f& position, const Quaternion& rotation, const Vector3f& scale);
	private:
		Vector3f TransformPoint(const Matrix4x4& mat, const Vector3f& point, Vector3f* absError = nullptr) const;
		Vector3f TransformPoint(const Matrix4x4& mat, const Vector3f& point, const Vector3f& ptError, Vector3f* absError = nullptr) const;
		Vector3f TransformVector(const Matrix4x4& mat, const Vector3f& vec, Vector3f* absError = nullptr) const;
	private:
		Vector3f mPosition;
		Vector3f mScale;
		Quaternion mRotation;

		mutable Matrix4x4 mat;
		mutable Matrix4x4 matInv;
		mutable bool mMatrixDirty;
	};
};


