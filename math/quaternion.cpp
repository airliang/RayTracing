


// core/quaternion.cpp*
#include "quaternion.h"
#include "matrix.h"

namespace AIR {

// Quaternion Method Definitions
Matrix4x4 Quaternion::ToMatrix() const {
    Float xx = v.x * v.x, yy = v.y * v.y, zz = v.z * v.z;
    Float xy = v.x * v.y, xz = v.x * v.z, yz = v.y * v.z;
    Float wx = v.x * w, wy = v.y * w, wz = v.z * w;

    Matrix4x4 m;
    m._M[0][0] = 1 - 2 * (yy + zz);
    m._M[0][1] = 2 * (xy + wz);
    m._M[0][2] = 2 * (xz - wy);
    m._M[1][0] = 2 * (xy - wz);
    m._M[1][1] = 1 - 2 * (xx + zz);
    m._M[1][2] = 2 * (yz + wx);
    m._M[2][0] = 2 * (xz + wy);
    m._M[2][1] = 2 * (yz - wx);
    m._M[2][2] = 1 - 2 * (xx + yy);

    // Transpose since we are left-handed.  Ugh.
    return Matrix4x4::Transposed(m);
}

Matrix3x3 Quaternion::ToMatrix3x3() const {
	Float xx = v.x * v.x, yy = v.y * v.y, zz = v.z * v.z;
	Float xy = v.x * v.y, xz = v.x * v.z, yz = v.y * v.z;
	Float wx = v.x * w, wy = v.y * w, wz = v.z * w;

	Matrix3x3 m;
	m._M[0][0] = 1 - 2 * (yy + zz);
	m._M[0][1] = 2 * (xy + wz);
	m._M[0][2] = 2 * (xz - wy);
	m._M[1][0] = 2 * (xy - wz);
	m._M[1][1] = 1 - 2 * (xx + zz);
	m._M[1][2] = 2 * (yz + wx);
	m._M[2][0] = 2 * (xz + wy);
	m._M[2][1] = 2 * (yz - wx);
	m._M[2][2] = 1 - 2 * (xx + yy);

	// Transpose since we are left-handed.  Ugh.
	return m;
}

Quaternion::Quaternion(const Matrix4x4 &m) {

    Float trace = m._M[0][0] + m._M[1][1] + m._M[2][2];
    if (trace > 0.f) {
        // Compute w from matrix trace, then xyz
        // 4w^2 = m[0][0] + m[1][1] + m[2][2] + m[3][3] (but m[3][3] == 1)
        Float s = std::sqrt(trace + 1.0f);
        w = s / 2.0f;
        s = 0.5f / s;
        v.x = (m._M[2][1] - m._M[1][2]) * s;
        v.y = (m._M[0][2] - m._M[2][0]) * s;
        v.z = (m._M[1][0] - m._M[0][1]) * s;
    } else {
        // Compute largest of $x$, $y$, or $z$, then remaining components
        const int nxt[3] = {1, 2, 0};
        Float q[3];
        int i = 0;
        if (m._M[1][1] > m._M[0][0]) i = 1;
        if (m._M[2][2] > m._M[i][i]) i = 2;
        int j = nxt[i];
        int k = nxt[j];
        Float s = std::sqrt((m._M[i][i] - (m._M[j][j] + m._M[k][k])) + 1.0f);
        q[i] = s * 0.5f;
        if (s != 0.f) s = 0.5f / s;
        w = (m._M[k][j] - m._M[j][k]) * s;
        q[j] = (m._M[j][i] + m._M[i][j]) * s;
        q[k] = (m._M[k][i] + m._M[i][k]) * s;
        v.x = q[0];
        v.y = q[1];
        v.z = q[2];
    }
}

Quaternion::Quaternion(const Matrix3x3 &m) {

	Float trace = m._M[0][0] + m._M[1][1] + m._M[2][2];
	if (trace > 0.f) {
		// Compute w from matrix trace, then xyz
		// 4w^2 = m[0][0] + m[1][1] + m[2][2] + m[3][3] (but m[3][3] == 1)
		Float s = std::sqrt(trace + 1.0f);
		w = s / 2.0f;
		s = 0.5f / s;
		v.x = (m._M[2][1] - m._M[1][2]) * s;
		v.y = (m._M[0][2] - m._M[2][0]) * s;
		v.z = (m._M[1][0] - m._M[0][1]) * s;
	}
	else {
		// Compute largest of $x$, $y$, or $z$, then remaining components
		const int nxt[3] = { 1, 2, 0 };
		Float q[3];
		int i = 0;
		if (m._M[1][1] > m._M[0][0]) i = 1;
		if (m._M[2][2] > m._M[i][i]) i = 2;
		int j = nxt[i];
		int k = nxt[j];
		Float s = std::sqrt((m._M[i][i] - (m._M[j][j] + m._M[k][k])) + 1.0f);
		q[i] = s * 0.5f;
		if (s != 0.f) s = 0.5f / s;
		w = (m._M[k][j] - m._M[j][k]) * s;
		q[j] = (m._M[j][i] + m._M[i][j]) * s;
		q[k] = (m._M[k][i] + m._M[i][k]) * s;
		v.x = q[0];
		v.y = q[1];
		v.z = q[2];
	}
}

Quaternion Slerp(Float t, const Quaternion &q1, const Quaternion &q2) {
    Float cosTheta = Quaternion::Dot(q1, q2);
    if (cosTheta > .9995f)
        return Quaternion::Normalize((1 - t) * q1 + t * q2);
    else {
        Float theta = std::acos(Clamp(cosTheta, -1, 1));
        Float thetap = theta * t;
        Quaternion qperp = Quaternion::Normalize(q2 - q1 * cosTheta);
        return q1 * std::cos(thetap) + qperp * std::sin(thetap);
    }
}

} 
