
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <iostream>
#include "geometry.h"
#include "../RayTracing.h"
#include "transform.h"
#include "matrix.h"
#include "quaternion.h"
#include "sphere.h"
#include "geometryparam.h"

using namespace AIR;
using namespace std;

/*
// main program
int main(int argc, char *argv[]) {
	cout << "Hello CMake¡£" << endl;
	Vector3f forward = Vector3f::forward;
	forward = forward.Normalize();
	cout << "sizeof(matrix4x4)=" << sizeof(Matrix4x4) << endl;
	cout << "sizeof(quaternion)=" << sizeof(Quaternion) << endl;
	cout << "sizeof(matrix3x3)=" << sizeof(Matrix3x3) << endl;
	cout << "sizeof(Vector3f)=" << sizeof(Vector3f) << endl;
	cout << "sizeof(Vector2f)=" << sizeof(Vector2f) << endl;
	cout << "sizeof(Sphere)=" << sizeof(Sphere) << endl;
	cout << "sizeof(GeometryParam)=" << sizeof(GeometryParam) << endl;
	cout << "sizeof(Transform)=" << sizeof(Transform) << endl;

	Transform transform;
	Sphere sphere(1.0f, 0, Pi, 2.0f * Pi, &transform);
	Ray ray(Vector3f(0, 0, -2.0f), Vector3f::forward);
	SurfaceInteraction interaction;
	Float tHit;
	sphere.Intersect(ray, &tHit, &interaction);

	system("pause");
	return 0;
}

*/
