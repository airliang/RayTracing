#pragma once 
#include "geometry.h"
#include "quaternion.h"
//#include "SIMD.h"

namespace AIR
{
	
	class Matrix3x3
	{
	public:
		Matrix3x3()
		{
			memset(_M, 0, 9 * sizeof(float));
		}
		Matrix3x3(float mat[3][3])
		{
			memcpy(_M, mat, 9 * sizeof(float));
		}

		Matrix3x3(float m00, float m01, float m02,
				  float m10, float m11, float m12,
				  float m20, float m21, float m22)
		{
			_M[0][0] = m00;
			_M[0][1] = m01;
			_M[0][2] = m02;

			_M[1][0] = m10;
			_M[1][1] = m11;
			_M[1][2] = m12;

			_M[2][0] = m20;
			_M[2][1] = m21;
			_M[2][2] = m22;
		}

		inline Matrix3x3& operator = (const Matrix3x3 &other)
		{
			memcpy(&(_M), &(other._M), 9 * sizeof(float));
			return *this;
		}

		Vector3f GetRow(size_t i) const
		{
			return Vector3f(_M[i][0], _M[i][1], _M[i][2]);
		}
		void Identity()
		{
			_11 = 1.0f, _12 = 0.0f, _13 = 0.0f;
			_21 = 0.0f, _22 = 1.0f, _23 = 0.0f;
			_31 = 0.0f, _32 = 0.0f, _33 = 1.0f;
		}
		void SetRow(size_t i, float x, float y, float z)
		{
			_M[i][0] = x;
			_M[i][1] = y;
			_M[i][2] = z;
		}

		Vector3f GetColumn(size_t i) const
		{
			return Vector3f(_M[0][i], _M[1][i], _M[2][i]);
		}

		float Determinant() const
		{
			return
				+ _M[0][0] * (_M[1][1] * _M[2][2] - _M[2][1] * _M[1][2])
				- _M[1][0] * (_M[0][1] * _M[2][2] - _M[2][1] * _M[0][2])
				+ _M[2][0] * (_M[0][1] * _M[1][2] - _M[1][1] * _M[0][2]);
		}
		static Matrix3x3 Inverse(const Matrix3x3& mat)
		{
			//assert(this != &output);
			Matrix3x3 output;
			float s = mat.Determinant();
			if (s == 0) return output;

			s = 1.0f / s;
			output._11 = s * ((mat._22 * mat._33) - (mat._23 * mat._32));
			output._12 = s * ((mat._32 * mat._13) - (mat._33 * mat._12));
			output._13 = s * ((mat._12 * mat._23) - (mat._13 * mat._22));
			output._21 = s * ((mat._23 * mat._31) - (mat._21 * mat._33));
			output._22 = s * ((mat._33 * mat._11) - (mat._31 * mat._13));
			output._23 = s * ((mat._13 * mat._21) - (mat._11 * mat._23));
			output._31 = s * ((mat._21 * mat._32) - (mat._22 * mat._31));
			output._32 = s * ((mat._31 * mat._12) - (mat._32 * mat._11));
			output._33 = s * ((mat._11 * mat._22) - (mat._12 * mat._21));
			return output;
		}
		
		//void Decompose(Vector3f& scale, Quaternion& rotation)
		//{
		//	
		//}

		inline const float* operator[] (size_t iRow) const
		{
			return _M[iRow];
		}

		inline float* operator[] (size_t iRow)
		{
			return _M[iRow];
		}

	public:
		union
		{
			float _M[3][3];
			struct
			{
				float _11, _12, _13;
				float _21, _22, _23;
				float _31, _32, _33;
			};
		};

		static Matrix3x3 IDENTITY;
	};

	typedef Matrix3x3 Matrix3f;
	

	template <typename T>
	inline Vector3<T> MultiplyVector(const Matrix3f &mat, const Vector3f &vec)
	{
		return Vector3<T>(
			mat._M[0][0] * vec.x + mat._M[0][1] * vec.y + mat._M[0][2] * vec.z,
			mat._M[1][0] * vec.x + mat._M[1][1] * vec.y + mat._M[1][2] * vec.z,
			mat._M[2][0] * vec.x + mat._M[2][0] * vec.y + mat._M[2][0] * vec.z);
	}
	

	inline Matrix3x3 operator *(const Matrix3x3 &m1, const Matrix3x3 &m2)
	{
		Matrix3x3 retvalue;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				retvalue._M[i][j] = m1._M[i][0] * m2._M[0][j] + m1._M[i][1] * m2._M[1][j] + m1._M[i][2] * m2._M[2][j];
			}
		}
		return retvalue;
	}


    class Matrix4x4
    {
    public:
        Matrix4x4()
        {
            memset(M, 0, 16 * sizeof(float));
        }

		Matrix4x4(float mat[4][4])
		{
			memcpy(_M, mat, 16 * sizeof(float));
		}

        Matrix4x4(float m00, float m01, float m02, float m03, 
                       float m10, float m11, float m12, float m13,
                       float m20, float m21, float m22, float m23, 
                       float m30, float m31, float m32, float m33)
        {
            M[0] = m00;
            M[1] = m01;
            M[2] = m02;
            M[3] = m03;
            M[4] = m10;
            M[5] = m11;
            M[6] = m12;
            M[7] = m13;
            M[8] = m20;
            M[9] = m21;
            M[10] = m22;
            M[11] = m23;
            M[12] = m30;
            M[13] = m31;
            M[14] = m32;
            M[15] = m33;
        }

        Matrix4x4(const Matrix4x4& other)// : m_bIdentity(false)
        {
			memcpy( &(this->M), &(other.M), sizeof(float) * 16 );
        }

        static Matrix4x4 Identity()
        {
			Matrix4x4 r;
            memset(r.M, 0, 16 * sizeof(float));
            r.M[0] = r.M[5] = r.M[10] = r.M[15] = (float)1;
            //m_bIdentity = true;
            //m_bIdentity=true;
            return r;
        }

        //! Simple operator for directly accessing every element of the matrix.
        inline float& operator()(const int row, const int col) 
        { 
            //m_bIdentity = false; 
            return M[ row * 4 + col ]; 
        }

        //! Simple operator for directly accessing every element of the matrix.
        inline const float& operator()(const int row, const int col) const 
        { 
            return M[row * 4 + col];
        }

        //! Simple operator for linearly accessing every element of the matrix.
        float& operator[](int index) 
        { 
            //m_bIdentity = false; 
            return M[index]; 
        }

        //! Simple operator for linearly accessing every element of the matrix.
        inline const float& operator[](int index) const 
        { 
            return M[index]; 
        }

        //! Sets this matrix equal to the other matrix.
        inline Matrix4x4& operator = (const Matrix4x4 &other)
        {
            if (this == &other)
                return *this;
            memcpy( &(M), &(other.M), 16 * sizeof(float) );
            //m_bIdentity = other.m_bIdentity;
            return *this;
        }

        //! Sets all elements of this matrix to the value.
        inline Matrix4x4& operator = (const float& scalar)
        {
            for (int i = 0; i < 16; ++i)
                M[i] = scalar;
            //m_bIdentity = false;
            return *this;
        }

        //! Add another matrix.
        Matrix4x4 operator + (const Matrix4x4& other) const
        {
            Matrix4x4 temp;

            temp[0] = M[0] + other[0];
            temp[1] = M[1] + other[1];
            temp[2] = M[2] + other[2];
            temp[3] = M[3] + other[3];
            temp[4] = M[4] + other[4];
            temp[5] = M[5] + other[5];
            temp[6] = M[6] + other[6];
            temp[7] = M[7] + other[7];
            temp[8] = M[8] + other[8];
            temp[9] = M[9] + other[9];
            temp[10] = M[10] + other[10];
            temp[11] = M[11] + other[11];
            temp[12] = M[12] + other[12];
            temp[13] = M[13] + other[13];
            temp[14] = M[14] + other[14];
            temp[15] = M[15] + other[15];

            return temp;
        }



        //! Add another matrix.
        Matrix4x4& operator += (const Matrix4x4& other)
        {
            M[0] += other[0];
            M[1] += other[1];
            M[2] += other[2];
            M[3] += other[3];
            M[4] += other[4];
            M[5] += other[5];
            M[6] += other[6];
            M[7] += other[7];
            M[8] += other[8];
            M[9] += other[9];
            M[10] += other[10];
            M[11] += other[11];
            M[12] += other[12];
            M[13] += other[13];
            M[14] += other[14];
            M[15] += other[15];

            return *this;
        }

        //! Subtract another matrix.
        Matrix4x4 operator - (const Matrix4x4& other) const
        {
            Matrix4x4 temp; // ( EM4CONST_NOTHING );

            temp[0] = M[0] - other[0];
            temp[1] = M[1] - other[1];
            temp[2] = M[2] - other[2];
            temp[3] = M[3] - other[3];
            temp[4] = M[4] - other[4];
            temp[5] = M[5] - other[5];
            temp[6] = M[6] - other[6];
            temp[7] = M[7] - other[7];
            temp[8] = M[8] - other[8];
            temp[9] = M[9] - other[9];
            temp[10] = M[10] - other[10];
            temp[11] = M[11] - other[11];
            temp[12] = M[12] - other[12];
            temp[13] = M[13] - other[13];
            temp[14] = M[14] - other[14];
            temp[15] = M[15] - other[15];

            return temp;
        }

        //! Subtract another matrix.
        Matrix4x4& operator -= (const Matrix4x4& other)
        {
            M[0] -= other[0];
            M[1] -= other[1];
            M[2] -= other[2];
            M[3] -= other[3];
            M[4] -= other[4];
            M[5] -= other[5];
            M[6] -= other[6];
            M[7] -= other[7];
            M[8] -= other[8];
            M[9] -= other[9];
            M[10] -= other[10];
            M[11] -= other[11];
            M[12] -= other[12];
            M[13] -= other[13];
            M[14] -= other[14];
            M[15] -= other[15];

            return *this;
        }

        //! Multiply by another matrix.
        Matrix4x4 operator * (const Matrix4x4& m2) const
        {
			Matrix4x4 r;
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					r._M[i][j] = _M[i][0] * m2._M[0][j] + _M[i][1] * m2._M[1][j] +
					_M[i][2] * m2._M[2][j] + _M[i][3] * m2._M[3][j];
			return r;
        }

        

        //! Set the translation of the current matrix. Will erase any previous values.
        Matrix4x4& SetTranslation( const Vector3f& translation )
        {
            M[3] = translation.x;
            M[7] = translation.y;
            M[11] = translation.z;

			M[15] = 1.0f;/////////
            //m_bIdentity=false;
            return *this;
        }

        Matrix4x4& SetTranslation( float x, float y, float z )
        {
            M[3] = x;
            M[7] = y;
            M[11] = z;

            M[15] = 1.0f;/////////
            //m_bIdentity=false;
            return *this;
        }

        //! Gets the current translation
        inline Vector3f GetTranslation() const
        {
            return Vector3f(M[3], M[7], M[11]);
        }

		static  Matrix4x4 GetScaleMatrix(const Vector3f& s)
		{
			return Matrix4x4(
				s.x, 0.0f, 0.0f, 0.0f, 
				0.0f,    s.y, 0.0f, 0.0f,
				0.0f, 0.0f,    s.z, 0.0f, 
				0.0f, 0.0f, 0.0f, 1.0f
				);
		}

		static  Matrix4x4 GetScaleMatrix(Float x, Float y, Float z)
		{
			return Matrix4x4(
				x, 0.0f, 0.0f, 0.0f,
				0.0f, y, 0.0f, 0.0f,
				0.0f, 0.0f, z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}

		static Matrix4x4 GetTranslateMatrix(Float x, Float y, Float z)
		{
			return Matrix4x4(
				0.0f, 0.0f, 0.0f, x,
				0.0f, 0.0f, 0.0f, y,
				0.0f, 0.0f, 0.0f, z,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}


        static Matrix4x4 Inverse(const Matrix4x4& m)
        {
            //if (m_bIdentity)
            //    return true;
			int indxc[4], indxr[4];
			int ipiv[4] = { 0, 0, 0, 0 };
			Float minv[4][4];
			memcpy(minv, m._M, 4 * 4 * sizeof(Float));
			for (int i = 0; i < 4; i++) {
				int irow = 0, icol = 0;
				Float big = 0.f;
				// Choose pivot
				for (int j = 0; j < 4; j++) {
					if (ipiv[j] != 1) {
						for (int k = 0; k < 4; k++) {
							if (ipiv[k] == 0) {
								if (std::abs(minv[j][k]) >= big) {
									big = Float(std::abs(minv[j][k]));
									irow = j;
									icol = k;
								}
							}
							else if (ipiv[k] > 1)
							{
								//Error("Singular matrix in MatrixInvert");
							}
								
						}
					}
				}
				++ipiv[icol];
				// Swap rows _irow_ and _icol_ for pivot
				if (irow != icol) {
					for (int k = 0; k < 4; ++k) std::swap(minv[irow][k], minv[icol][k]);
				}
				indxr[i] = irow;
				indxc[i] = icol;
				if (minv[icol][icol] == 0.f)
				{
					//Error("Singular matrix in MatrixInvert");
				}

				// Set $_M[icol][icol]$ to one by scaling row _icol_ appropriately
				Float pivinv = 1. / minv[icol][icol];
				minv[icol][icol] = 1.;
				for (int j = 0; j < 4; j++) minv[icol][j] *= pivinv;

				// Subtract this row from others to zero out their columns
				for (int j = 0; j < 4; j++) {
					if (j != icol) {
						Float save = minv[j][icol];
						minv[j][icol] = 0;
						for (int k = 0; k < 4; k++) minv[j][k] -= minv[icol][k] * save;
					}
				}
			}
			// Swap columns to reflect permutation
			for (int j = 3; j >= 0; j--) {
				if (indxr[j] != indxc[j]) {
					for (int k = 0; k < 4; k++)
						std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
				}
			}
			return Matrix4x4(minv);
        }

        //! Gets transposed matrix
        static Matrix4x4 Transposed(const Matrix4x4& mat)
        {
            Matrix4x4 t;
			t[0] = mat.M[0];
			t[1] = mat.M[4];
			t[2] = mat.M[8];
			t[3] = mat.M[12];
			
			t[4] = mat.M[1];
			t[5] = mat.M[5];
			t[6] = mat.M[9];
			t[7] = mat.M[13];
			
			t[8] = mat.M[2];
			t[9] = mat.M[6];
			t[10] = mat.M[10];
			t[11] = mat.M[14];
			
			t[12] = mat.M[3];
			t[13] = mat.M[7];
			t[14] = mat.M[11];
			t[15] = mat.M[15];
            return t;
        }

		static Matrix4x4 RotateX(Float theta) {
			Float sinTheta = std::sin(Radians(theta));
			Float cosTheta = std::cos(Radians(theta));
			Matrix4x4 _M(1, 0, 0, 0, 0, cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0,
				0, 0, 0, 1);
			return _M;
		}

		static Matrix4x4 RotateY(Float theta) {
			Float sinTheta = std::sin(Radians(theta));
			Float cosTheta = std::cos(Radians(theta));
			Matrix4x4 _M(cosTheta, 0, sinTheta, 0, 0, 1, 0, 0, -sinTheta, 0, cosTheta, 0,
				0, 0, 0, 1);
			return _M;
		}

		static Matrix4x4 RotateZ(Float theta) {
			Float sinTheta = std::sin(Radians(theta));
			Float cosTheta = std::cos(Radians(theta));
			Matrix4x4 _M(cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0, 0, 0, 1, 0,
				0, 0, 0, 1);
			return _M;
		}

		static Matrix4x4 Rotate(Float theta, const Vector3f &axis) {
			Vector3f a = Vector3f::Normalize(axis);
			Float sinTheta = std::sin(Radians(theta));
			Float cosTheta = std::cos(Radians(theta));
			Matrix4x4 _M;
			// Compute rotation of first basis vector
			_M._M[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
			_M._M[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
			_M._M[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
			_M._M[0][3] = 0;

			// Compute rotations of second and third basis vectors
			_M._M[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
			_M._M[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
			_M._M[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
			_M._M[1][3] = 0;

			_M._M[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
			_M._M[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
			_M._M[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
			_M._M[2][3] = 0;
			return _M;
		}

		static Matrix4x4 LookAt(const Vector3f &pos, const Vector3f &look, const Vector3f &up) {
			Matrix4x4 cameraToWorld;
			// Initialize fourth column of viewing matrix
			cameraToWorld._M[0][3] = pos.x;
			cameraToWorld._M[1][3] = pos.y;
			cameraToWorld._M[2][3] = pos.z;
			cameraToWorld._M[3][3] = 1;

			// Initialize first three columns of viewing matrix
			Vector3f dir = Vector3f::Normalize(look - pos);
			if (Vector3f::Cross(Vector3f::Normalize(up), dir).Length() == 0) {
				//Error(
				//	"\"up\" vector (%f, %f, %f) and viewing direction (%f, %f, %f) "
				//	"passed to LookAt are pointing in the same direction.  Using "
				//	"the identity transformation.",
				//	up.x, up.y, up.z, dir.x, dir.y, dir.z);
				return Matrix4x4();//Matrix4x4::ZERO;
			}
			Vector3f right = Vector3f::Normalize(Vector3f::Cross(Vector3f::Normalize(up), dir));
			Vector3f newUp = Vector3f::Cross(dir, right);
			cameraToWorld._M[0][0] = right.x;
			cameraToWorld._M[1][0] = right.y;
			cameraToWorld._M[2][0] = right.z;
			cameraToWorld._M[3][0] = 0.;
			cameraToWorld._M[0][1] = newUp.x;
			cameraToWorld._M[1][1] = newUp.y;
			cameraToWorld._M[2][1] = newUp.z;
			cameraToWorld._M[3][1] = 0.;
			cameraToWorld._M[0][2] = dir.x;
			cameraToWorld._M[1][2] = dir.y;
			cameraToWorld._M[2][2] = dir.z;
			cameraToWorld._M[3][2] = 0.;
			return cameraToWorld;
		}

		static Matrix4x4 Mul(const Matrix4x4 &m1, const Matrix4x4 &m2)
		{
			Matrix4x4 r;
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					r._M[i][j] = m1._M[i][0] * m2._M[0][j] + m1._M[i][1] * m2._M[1][j] +
					m1._M[i][2] * m2._M[2][j] + m1._M[i][3] * m2._M[3][j];
			return r;
		}

		float Determinant()
		{
			float SubFactor00 = _M[2][2] * _M[3][3] - _M[3][2] * _M[2][3];
			float SubFactor01 = _M[2][1] * _M[3][3] - _M[3][1] * _M[2][3];
			float SubFactor02 = _M[2][1] * _M[3][2] - _M[3][1] * _M[2][2];
			float SubFactor03 = _M[2][0] * _M[3][3] - _M[3][0] * _M[2][3];
			float SubFactor04 = _M[2][0] * _M[3][2] - _M[3][0] * _M[2][2];
			float SubFactor05 = _M[2][0] * _M[3][1] - _M[3][0] * _M[2][1];

			
			float DetCof0 = +(_M[1][1] * SubFactor00 - _M[1][2] * SubFactor01 + _M[1][3] * SubFactor02);
			float DetCof1 = -(_M[1][0] * SubFactor00 - _M[1][2] * SubFactor03 + _M[1][3] * SubFactor04);
			float DetCof2 = +(_M[1][0] * SubFactor01 - _M[1][1] * SubFactor03 + _M[1][3] * SubFactor05);
			float DetCof3 = -(_M[1][0] * SubFactor02 - _M[1][1] * SubFactor04 + _M[1][2] * SubFactor05);

			return
				_M[0][0] * DetCof0 + _M[0][1] * DetCof1 +
				_M[0][2] * DetCof2 + _M[0][3] * DetCof3;
		}

		void Decompose(Vector3f& translate, Vector3f& scale, Quaternion& rotation)
		{
			translate.x = _M[0][3];
			translate.y = _M[1][3];
			translate.z = _M[2][3];
		}

		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float			_M[4][4];
			float			M[16];
		};

        //bool m_bIdentity;

    public:
		static Matrix4x4 IDENTITY;
		static const Matrix4x4 ZERO;
    };

    typedef Matrix4x4 Matrix4f;

	//Matrix4x4 Matrix4x4::ZERO;

	template <typename T>
	inline Vector3<T> PointMultiply(const Vector3<T> &point, const Matrix4f &mat)
	{
		return Vector3<T>(
			(float)point.x * mat[0] + (float)point.y * mat[4] + (float)point.z * mat[8] + mat[12],
			(float)point.x * mat[1] + (float)point.y * mat[5] + (float)point.z * mat[9] + mat[13],
			(float)point.x * mat[2] + (float)point.y * mat[6] + (float)point.z * mat[10] + mat[14]);
	}

	template <typename T>
	inline Vector3<T> MultiplyPoint(const Matrix4f &mat, const Vector3<T> &point)
	{
		T xp = mat._M[0][0] * point.x + mat._M[0][1] * point.y + mat._M[0][2] * point.z + mat._M[0][3];
		T yp = mat._M[1][0] * point.x + mat._M[1][1] * point.y + mat._M[1][2] * point.z + mat._M[1][3];
		T zp = mat._M[2][0] * point.x + mat._M[2][1] * point.y + mat._M[2][2] * point.z + mat._M[2][3];
		T wp = mat._M[3][0] * point.x + mat._M[3][1] * point.y + mat._M[3][2] * point.z + mat._M[3][3];

		if (wp == 1.)
		{
			return Vector3<T>(xp, yp, zp);
		}
		else
		{
			return Vector3<T>(xp, yp, zp) / wp;
		}
	}

	template <typename T>
	inline Vector3<T> MultiplyVector(const Matrix4f &mat, const Vector3<T> &vec)
	{
		return Vector3<T>(
			mat._M[0][0] * vec.x + mat._M[0][1] * vec.y + mat._M[0][2] * vec.z,
			mat._M[1][0] * vec.x + mat._M[1][1] * vec.y + mat._M[1][2] * vec.z,
			mat._M[2][0] * vec.x + mat._M[2][0] * vec.y + mat._M[2][2] * vec.z);
	}

};



