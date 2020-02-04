#include "matrix.h"

namespace AIR
{
	Matrix3x3 Matrix3x3::IDENTITY = Matrix3x3(1.0f, 0, 0,
		0, 1.0f, 0,
		0, 0, 1.0f);

	Matrix4x4 Matrix4x4::IDENTITY = Matrix4x4
	(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
}
