#pragma once

#include "geometry.h"
//#include "matrix.h"

namespace AIR 
{
	class Matrix4x4;
	class Matrix3x3;
// Quaternion Declarations
struct Quaternion 
{
    // Quaternion Public Methods
    Quaternion() : x(0), y(0), z(0), w(1) {}

	Quaternion(const Quaternion &q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

    Quaternion &operator += (const Quaternion &q) {
        x += q.x;
		y += q.y;
		z += q.z;
        w += q.w;
        return *this;
    }

	Quaternion &operator=(const Quaternion &q) {
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
		return *this;
	}
    friend Quaternion operator + (const Quaternion &q1, const Quaternion &q2) {
        Quaternion ret = q1;
        return ret += q2;
    }
    Quaternion &operator-=(const Quaternion &q) {
        x -= q.x;
		y -= q.y;
		z -= q.z;
        w -= q.w;
        return *this;
    }
    Quaternion operator-() const {
        Quaternion ret;
        ret.x = -x;
		ret.y = -y;
		ret.z = -z;
        ret.w = -w;
        return ret;
    }
    friend Quaternion operator-(const Quaternion &q1, const Quaternion &q2) {
        Quaternion ret = q1;
        return ret -= q2;
    }
    Quaternion &operator*=(Float f) {
        x *= f;
		y *= f;
		z *= f;
        w *= f;
        return *this;
    }
    Quaternion operator*(Float f) const {
        Quaternion ret = *this;
        ret.x *= f;
		ret.y *= f;
		ret.z *= f;
        ret.w *= f;
        return ret;
    }
    Quaternion &operator/=(Float f) {
		Float invF = 1.0f / f;
        x *= invF;
		y *= invF;
		z *= invF;
        w *= invF;
        return *this;
    }
    Quaternion operator/(Float f) const {
        Quaternion ret = *this;
		Float invF = 1.0f / f;
		ret.x *= invF;
		ret.y *= invF;
		ret.z *= invF;
		ret.w *= invF;
        return ret;
    }
    Matrix4x4 ToMatrix() const;
	Matrix3x3 ToMatrix3x3() const;
    Quaternion(const Matrix4x4 &m);
	Quaternion(const Matrix3x3 &m);

	static Float Dot(const Quaternion &q1, const Quaternion &q2)
	{
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	static Quaternion Normalize(const Quaternion &q) {
		return q / std::sqrt(Dot(q, q));
	}

    // Quaternion Public Data
	union 
	{
		struct  
		{
			float x, y, z;
		};
		Vector3f v;
	};
    
    float w;
};

Quaternion Slerp(Float t, const Quaternion &q1, const Quaternion &q2);

// Quaternion Inline Functions
inline Quaternion operator*(Float f, const Quaternion &q) { return q * f; }





} 

