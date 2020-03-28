#pragma once

#include "Transform.h"

namespace AIR
{
	void Transform::SetPosition(const Vector3f& position)
	{
		mat.SetTranslation(position);
		matInv = Matrix4x4::Inverse(mat);
		mPosition = position;
		mMatrixDirty = true;
	}

	void Transform::SetScale(const Vector3f& scale)
	{
		//Vector3f x = Vector3f(mat._M[0][0], mat._M[1][0], mat._M[2][0]).Normalize() * scale.x;
		//Vector3f y = Vector3f(mat._M[0][1], mat._M[1][1], mat._M[2][1]).Normalize() * scale.y;
		//Vector3f z = Vector3f(mat._M[0][2], mat._M[1][2], mat._M[2][2]).Normalize() * scale.z;
		//mat._M[0][0] = x.x;
		//mat._M[1][0] = x.y;
		//mat._M[2][0] = x.z;
		//mat._M[0][1] = y.x;
		//mat._M[1][1] = y.y;
		//mat._M[2][1] = y.z;
		//mat._M[0][2] = z.x;
		//mat._M[1][2] = z.y;
		//mat._M[2][2] = z.z;
		//matInv = Matrix4x4::Inverse(mat);
		mScale = scale;
		mMatrixDirty = true;
	}

	void Transform::SetRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
		mMatrixDirty = true;
	}

	const Matrix4x4& Transform::LocalToWorld() const
	{
		if (mMatrixDirty)
		{
			Matrix4x4 scale = Matrix4x4::GetScaleMatrix(mScale);
			Matrix4x4 rotation = mRotation.ToMatrix();
			mat = Matrix4x4::Mul(rotation, scale);
			mat.SetTranslation(mPosition);
			matInv = Matrix4x4::Inverse(mat);
		}
		return mat;
	}

	const Matrix4x4& Transform::WorldToLocal() const
	{
		if (mMatrixDirty)
		{
			Matrix4x4 scale = Matrix4x4::GetScaleMatrix(mScale);
			Matrix4x4 rotation = mRotation.ToMatrix();
			mat = Matrix4x4::Mul(rotation, scale);
			mat.SetTranslation(mPosition);
			matInv = Matrix4x4::Inverse(mat);
			mMatrixDirty = false;
		}
		return matInv;
	}

	Vector3f Transform::ObjectToWorldPoint(const Vector3f& point, Vector3f* absError) const
	{
		const Matrix4x4& localToWorld = LocalToWorld();
		return TransformPoint(localToWorld, point, absError);
	}

	Vector3f Transform::ObjectToWorldVector(const Vector3f& vec, Vector3f* absError) const
	{
		const Matrix4x4& localToWorld = LocalToWorld();
		
		return TransformVector(localToWorld, vec, absError);
	}

	Vector3f Transform::ObjectToWorldNormal(const Vector3f& normal) const
	{
		Float x = normal.x, y = normal.y, z = normal.z;
		const Matrix4x4& inv = WorldToLocal();
		return Vector3f(inv._M[0][0] * x + inv._M[1][0] * y + inv._M[2][0] * z,
			inv._M[0][1] * x + inv._M[1][1] * y + inv._M[2][1] * z,
			inv._M[0][2] * x + inv._M[1][2] * y + inv._M[2][2] * z);
	}

	Ray Transform::ObjectToWorldRay(const Ray& ray) const
	{
		Vector3f oError;
		Vector3f o = ObjectToWorldPoint(ray.o, &oError);
		Vector3f d = ObjectToWorldVector(ray.d);
		Float tMax = ray.tMax;
		Float lengthSquared = d.LengthSquared();
		if (lengthSquared > 0) 
		{
			Float dt = Vector3f::Dot(Vector3f::Abs(d), oError) / lengthSquared;
			o += d * dt;
			tMax -= dt;
		}
		return Ray(o, d, tMax, ray.time);
	}

	Ray Transform::WorldToObjectRay(const Ray& ray, Vector3f* oError, Vector3f* dError) const
	{
		//Vector3f oError;
		Vector3f o = WorldToObjectPoint(ray.o, oError);
		Vector3f d = WorldToObjectVector(ray.d, dError);
		Float tMax = ray.tMax;
		Float lengthSquared = d.LengthSquared();
		if (lengthSquared > 0)
		{
			//误差的处理：把oError投影到abs(d)，取abs(d)是为了求最大误差，
			//d全是正数，dot后误差才最大
			//然后归一化？按道理应该是除以length
			Float dt = Vector3f::Dot(Vector3f::Abs(d), *oError) / lengthSquared;
			o += d * dt;
			tMax -= dt;
		}

		return Ray(o, d, tMax, ray.time);
	}

	Vector3f Transform::WorldToObjectPoint(const Vector3f& point, Vector3f* absError /* = nullptr */) const
	{
		const Matrix4x4& worldToLocal = WorldToLocal();
		Vector3f p = TransformPoint(worldToLocal, point, absError);

		return p;
	}

	Vector3f Transform::WorldToObjectVector(const Vector3f& vec, Vector3f* absError /* = nullptr */) const
	{
		const Matrix4x4& worldToLocal = WorldToLocal();
		Vector3f v = TransformVector(worldToLocal, vec, absError);

		return v;
	}

	Vector3f Transform::WorldToObjectNormal(const Vector3f& normal) const
	{
		const Matrix4x4& localToWorld = LocalToWorld();
		Float x = normal.x, y = normal.y, z = normal.z;
		return Vector3f(localToWorld._M[0][0] * x + localToWorld._M[1][0] * y + localToWorld._M[2][0] * z,
			localToWorld._M[0][1] * x + localToWorld._M[1][1] * y + localToWorld._M[2][1] * z,
			localToWorld._M[0][2] * x + localToWorld._M[1][2] * y + localToWorld._M[2][2] * z);
	}

	Interaction Transform::ObjectToWorldInteraction(const Interaction& isect) const
	{
		const Matrix4x4& localToWorld = LocalToWorld();
		Interaction ret;
		ret.interactPoint = TransformPoint(localToWorld, isect.interactPoint, isect.pError, &ret.pError);
		ret.normal = ObjectToWorldNormal(isect.normal);
		ret.wo = TransformVector(localToWorld, isect.wo);
		ret.time = isect.time;
		ret.uv = isect.uv;
		ret.dpdu = TransformVector(localToWorld, isect.dpdu);
		ret.dpdv = TransformVector(localToWorld, isect.dpdv);
		ret.dndu = TransformVector(localToWorld, isect.dndu);
		ret.dndv = TransformVector(localToWorld, isect.dndv);

		ret.dudx = isect.dudx;
		ret.dvdx = isect.dvdx;
		ret.dudy = isect.dudy;
		ret.dvdy = isect.dvdy;
		ret.dpdx = TransformVector(localToWorld, isect.dpdx);
		ret.dpdy = TransformVector(localToWorld, isect.dpdy);

		return ret;
	}

	Vector3f Transform::TransformPoint(const Matrix4x4& mat, const Vector3f& point, Vector3f* absError) const
	{
		
		Float x = point.x;
		Float y = point.y;
		Float z = point.z;
		// Compute absolute error for transformed point
		if (absError != nullptr)
		{
			Float xAbsSum = (std::abs(mat._M[0][0] * x) + std::abs(mat._M[0][1] * y) +
				std::abs(mat._M[0][2] * z) + std::abs(mat._M[0][3]));
			Float yAbsSum = (std::abs(mat._M[1][0] * x) + std::abs(mat._M[1][1] * y) +
				std::abs(mat._M[1][2] * z) + std::abs(mat._M[1][3]));
			Float zAbsSum = (std::abs(mat._M[2][0] * x) + std::abs(mat._M[2][1] * y) +
				std::abs(mat._M[2][2] * z) + std::abs(mat._M[2][3]));
			*absError = gamma(3) * Vector3f(xAbsSum, yAbsSum, zAbsSum);
		}
		Vector3f p = MultiplyPoint(mat, point);
		return p;
	}

	Vector3f Transform::TransformPoint(const Matrix4x4& mat, const Vector3f& point, const Vector3f& ptError, Vector3f* absError) const
	{
		Float x = point.x;
		Float y = point.y;
		Float z = point.z;
		// Compute absolute error for transformed point
		if (absError != nullptr)
		{
			absError->x =
				(gamma(3) + 1.0f) *
				(std::abs(mat._M[0][0]) * ptError.x + std::abs(mat._M[0][1]) * ptError.y +
					std::abs(mat._M[0][2]) * ptError.z) +
				gamma(3) * (std::abs(mat._M[0][0] * x) + std::abs(mat._M[0][1] * y) +
					std::abs(mat._M[0][2] * z) + std::abs(mat._M[0][3]));
			absError->y =
				(gamma(3) + 1.0f) *
				(std::abs(mat._M[1][0]) * ptError.x + std::abs(mat._M[1][1]) * ptError.y +
					std::abs(mat._M[1][2]) * ptError.z) +
				gamma(3) * (std::abs(mat._M[1][0] * x) + std::abs(mat._M[1][1] * y) +
					std::abs(mat._M[1][2] * z) + std::abs(mat._M[1][3]));
			absError->z =
				(gamma(3) + 1.0f) *
				(std::abs(mat._M[2][0]) * ptError.x + std::abs(mat._M[2][1]) * ptError.y +
					std::abs(mat._M[2][2]) * ptError.z) +
				gamma(3) * (std::abs(mat._M[2][0] * x) + std::abs(mat._M[2][1] * y) +
					std::abs(mat._M[2][2] * z) + std::abs(mat._M[2][3]));
		}
		Vector3f p = MultiplyPoint(mat, point);
		return p;
	}

	Vector3f Transform::TransformVector(const Matrix4x4& mat, const Vector3f& vec, Vector3f* absError) const
	{
		if (absError != nullptr)
		{
			Float x = vec.x, y = vec.y, z = vec.z;
			absError->x =
				gamma(3) * (std::abs(mat._M[0][0] * vec.x) + std::abs(mat._M[0][1] * vec.y) +
					std::abs(mat._M[0][2] * vec.z));
			absError->y =
				gamma(3) * (std::abs(mat._M[1][0] * vec.x) + std::abs(mat._M[1][1] * vec.y) +
					std::abs(mat._M[1][2] * vec.z));
			absError->z =
				gamma(3) * (std::abs(mat._M[2][0] * vec.x) + std::abs(mat._M[2][1] * vec.y) +
					std::abs(mat._M[2][2] * vec.z));
		}

		Vector3f v = MultiplyVector(mat, vec);
		return v;
	}

	Bounds3f Transform::ObjectToWorldBound(const Bounds3f& bound) const
	{
		const Matrix4x4 localToWorld = LocalToWorld();
		Bounds3f ret(TransformPoint(localToWorld, bound.pMin));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMax.x, bound.pMin.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMin.x, bound.pMax.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMin.x, bound.pMin.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMin.x, bound.pMax.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMax.x, bound.pMax.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMax.x, bound.pMin.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(localToWorld, Vector3f(bound.pMax.x, bound.pMax.y, bound.pMax.z)));
		return ret;
	}

	Bounds3f Transform::WorldToObjectBound(const Bounds3f& bound) const
	{
		const Matrix4x4 worldToLocal = WorldToLocal();
		Bounds3f ret(TransformPoint(worldToLocal, bound.pMin));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMax.x, bound.pMin.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMin.x, bound.pMax.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMin.x, bound.pMin.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMin.x, bound.pMax.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMax.x, bound.pMax.y, bound.pMin.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMax.x, bound.pMin.y, bound.pMax.z)));
		ret = Union(ret, TransformPoint(worldToLocal, Vector3f(bound.pMax.x, bound.pMax.y, bound.pMax.z)));
		return ret;
	}
};


