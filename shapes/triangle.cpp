#include "triangle.h"
#include "sampling.h"
#include "interaction.h"

namespace AIR
{
    TriangleMesh::TriangleMesh(int nTriangles, const int* vertexIndices,
        int nVertices, const Point3f* P, const Vector3f* S, const Vector3f* N,
        const Point2f* UV) : nTriangles(nTriangles), 
        nVertices(nVertices),
        vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles)
    {
        p.reset(new Point3f[nVertices]);
        for (int i = 0; i < nVertices; ++i)
        {
            p[i] = P[i];
        }

        if (N != nullptr)
        {
            n.reset(new Vector3f[nVertices]);
            memcpy(n.get(), N, nVertices * sizeof(Vector3f));
        }

        if (S != nullptr)
        {
            s.reset(new Vector3f[nVertices]);
            memcpy(s.get(), S, nVertices * sizeof(Vector3f));
        }

        if (UV != nullptr)
        {
            uv.reset(new Point2f[nVertices]);
            memcpy(uv.get(), UV, nVertices * sizeof(Point2f));
        }
    }

    Triangle::Triangle(Transform* pTransform, const std::shared_ptr<TriangleMesh>& mesh, int triNumber) : Shape(pTransform)
        , mesh(mesh)
    {
        vIndices = &mesh->vertexIndices[triNumber * 3];
    }

    Bounds3f Triangle::ObjectBound() const
    {
        const Point3f& p0 = mesh->p[vIndices[0]];
        const Point3f& p1 = mesh->p[vIndices[1]];
        const Point3f& p2 = mesh->p[vIndices[2]];

        return Bounds3f::Union(Bounds3f(p0, p1), p2);
    }

    Bounds3f Triangle::WorldBound() const
    {
        const Point3f& p0 = mesh->p[vIndices[0]];
        const Point3f& p1 = mesh->p[vIndices[1]];
        const Point3f& p2 = mesh->p[vIndices[2]];

        return Bounds3f::Union(Bounds3f(mTransform->ObjectToWorldPoint(p0), mTransform->ObjectToWorldPoint(p1)), mTransform->ObjectToWorldPoint(p2));
    }

    bool Triangle::Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect) const
    {
		//ProfilePhase p(Prof::TriIntersect);
		//++nTests;
		// Get triangle vertices in _p0_, _p1_, and _p2_
		Point3f p0 = mTransform->ObjectToWorldPoint(mesh->p[vIndices[0]]);
		Point3f p1 = mTransform->ObjectToWorldPoint(mesh->p[vIndices[1]]);
		Point3f p2 = mTransform->ObjectToWorldPoint(mesh->p[vIndices[2]]);

		Vector3f n0 = mTransform->ObjectToWorldNormal(mesh->n[vIndices[0]]);
		Vector3f n1 = mTransform->ObjectToWorldNormal(mesh->n[vIndices[1]]);
		Vector3f n2 = mTransform->ObjectToWorldNormal(mesh->n[vIndices[2]]);

		// Perform ray--triangle intersection test

		// Transform triangle vertices to ray coordinate space

		// Translate vertices based on ray origin
		Point3f p0t = p0 - Vector3f(ray.o);
		Point3f p1t = p1 - Vector3f(ray.o);
		Point3f p2t = p2 - Vector3f(ray.o);

		// Permute components of triangle vertices and ray direction
		int kz = Vector3f::MaxDimension(Vector3f::Abs(ray.d));
		int kx = kz + 1;
		if (kx == 3) kx = 0;
		int ky = kx + 1;
		if (ky == 3) ky = 0;
		Vector3f d = Vector3f::Permute(ray.d, kx, ky, kz);
		p0t = Vector3f::Permute(p0t, kx, ky, kz);
		p1t = Vector3f::Permute(p1t, kx, ky, kz);
		p2t = Vector3f::Permute(p2t, kx, ky, kz);

		// Apply shear transformation to translated vertex positions
		Float Sx = -d.x / d.z;
		Float Sy = -d.y / d.z;
		Float Sz = 1.f / d.z;
		p0t.x += Sx * p0t.z;
		p0t.y += Sy * p0t.z;
		p1t.x += Sx * p1t.z;
		p1t.y += Sy * p1t.z;
		p2t.x += Sx * p2t.z;
		p2t.y += Sy * p2t.z;

		// Compute edge function coefficients _e0_, _e1_, and _e2_
		Float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
		Float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
		Float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

		// Fall back to double precision test at triangle edges
		if (sizeof(Float) == sizeof(float) &&
			(e0 == 0.0f || e1 == 0.0f || e2 == 0.0f)) 
		{
			double p2txp1ty = (double)p2t.x * (double)p1t.y;
			double p2typ1tx = (double)p2t.y * (double)p1t.x;
			e0 = (float)(p2typ1tx - p2txp1ty);
			double p0txp2ty = (double)p0t.x * (double)p2t.y;
			double p0typ2tx = (double)p0t.y * (double)p2t.x;
			e1 = (float)(p0typ2tx - p0txp2ty);
			double p1txp0ty = (double)p1t.x * (double)p0t.y;
			double p1typ0tx = (double)p1t.y * (double)p0t.x;
			e2 = (float)(p1typ0tx - p1txp0ty);
		}

		// Perform triangle edge and determinant tests
		if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
			return false;
		Float det = e0 + e1 + e2;
		if (det == 0) 
			return false;

		// Compute scaled hit distance to triangle and test against ray $t$ range
		p0t.z *= Sz;
		p1t.z *= Sz;
		p2t.z *= Sz;
		Float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
		if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
			return false;
		else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax* det))
			return false;

		// Compute barycentric coordinates and $t$ value for triangle intersection
		Float invDet = 1 / det;
		Float b0 = e0 * invDet;
		Float b1 = e1 * invDet;
		Float b2 = e2 * invDet;
		Float t = tScaled * invDet;

		// Ensure that computed triangle $t$ is conservatively greater than zero

		// Compute $\delta_z$ term for triangle $t$ error bounds
		Float maxZt = Vector3f::MaxComponent(Vector3f::Abs(Vector3f(p0t.z, p1t.z, p2t.z)));
		Float deltaZ = gamma(3) * maxZt;

		// Compute $\delta_x$ and $\delta_y$ terms for triangle $t$ error bounds
		Float maxXt = Vector3f::MaxComponent(Vector3f::Abs(Vector3f(p0t.x, p1t.x, p2t.x)));
		Float maxYt = Vector3f::MaxComponent(Vector3f::Abs(Vector3f(p0t.y, p1t.y, p2t.y)));
		Float deltaX = gamma(5) * (maxXt + maxZt);
		Float deltaY = gamma(5) * (maxYt + maxZt);

		// Compute $\delta_e$ term for triangle $t$ error bounds
		Float deltaE =
			2 * (gamma(2) * maxXt * maxYt + deltaY * maxXt + deltaX * maxYt);

		// Compute $\delta_t$ term for triangle $t$ error bounds and check _t_
		Float maxE = Vector3f::MaxComponent(Vector3f::Abs(Vector3f(e0, e1, e2)));
		Float deltaT = 3 *
			(gamma(3) * maxE * maxZt + deltaE * maxZt + deltaZ * maxE) *
			std::abs(invDet);
		if (t <= deltaT) 
			return false;

		// Compute triangle partial derivatives
		Vector3f dpdu, dpdv;
		Point2f uv[3];
		GetUVs(uv);

		// Compute deltas for triangle partial derivatives
		Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
		Vector3f dp02 = p0 - p2, dp12 = p1 - p2;
		Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		bool degenerateUV = std::abs(determinant) < 1e-8;
		if (!degenerateUV) 
		{
			Float invdet = 1 / determinant;
			dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
			dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
		}
		if (degenerateUV || Vector3f::Cross(dpdu, dpdv).LengthSquared() == 0) 
		{
			// Handle zero determinant for triangle partial derivative matrix
			Vector3f ng = Vector3f::Cross(p2 - p0, p1 - p0);
			if (ng.LengthSquared() == 0)
				// The triangle is actually degenerate; the intersection is
				// bogus.
				return false;

			CoordinateSystem(Vector3f::Normalize(ng), &dpdu, &dpdv);
		}

		// Compute error bounds for triangle intersection
		Float xAbsSum =
			(std::abs(b0 * p0.x) + std::abs(b1 * p1.x) + std::abs(b2 * p2.x));
		Float yAbsSum =
			(std::abs(b0 * p0.y) + std::abs(b1 * p1.y) + std::abs(b2 * p2.y));
		Float zAbsSum =
			(std::abs(b0 * p0.z) + std::abs(b1 * p1.z) + std::abs(b2 * p2.z));
		Vector3f pError = gamma(7) * Vector3f(xAbsSum, yAbsSum, zAbsSum);

		// Interpolate $(u,v)$ parametric coordinates and hit point
		Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
		Point2f uvHit = b0 * uv[0] + b1 * uv[1] + b2 * uv[2];

		// Test intersection against alpha texture, if present
		//if (testAlphaTexture && mesh->alphaMask) {
		//	SurfaceInteraction isectLocal(pHit, Vector3f(0, 0, 0), uvHit, -ray.d,
		//		dpdu, dpdv, Normal3f(0, 0, 0),
		//		Normal3f(0, 0, 0), ray.time, this);
		//	if (mesh->alphaMask->Evaluate(isectLocal) == 0) return false;
		//}

		// Fill in _SurfaceInteraction_ from triangle hit
		*isect = SurfaceInteraction(pHit, pError, uvHit, -ray.d, dpdu, dpdv,
			Vector3f(0, 0, 0), Vector3f(0, 0, 0), ray.time,
			this);

		// Override surface normal in _isect_ for triangle
		isect->normal = isect->shading.n = Vector3f::Normalize(Vector3f::Cross(dp02, dp12));
		//if (reverseOrientation ^ transformSwapsHandedness)
		//	isect->n = isect->shading.n = -isect->n;

		if (mesh->n || mesh->s) 
		{
			// Initialize _Triangle_ shading geometry

			// Compute shading normal _ns_ for triangle
			Vector3f ns;
			if (mesh->n) 
			{
				ns = (b0 * n0 + b1 * n1 + b2 * n2);
				if (ns.LengthSquared() > 0)
					ns = Vector3f::Normalize(ns);
				else
					ns = isect->normal;
			}
			else
				ns = isect->normal;

			// Compute shading tangent _ss_ for triangle
			Vector3f ss;
			if (mesh->s) 
			{
				Vector3f s0 = mTransform->ObjectToWorldVector(mesh->s[vIndices[0]]);
				Vector3f s1 = mTransform->ObjectToWorldVector(mesh->s[vIndices[1]]);
				Vector3f s2 = mTransform->ObjectToWorldVector(mesh->s[vIndices[2]]);

				ss = (b0 * s0 + b1 * s1 + b2 * s2);
				if (ss.LengthSquared() > 0)
					ss = Vector3f::Normalize(ss);
				else
					ss = Vector3f::Normalize(isect->dpdu);
			}
			else
				ss = Vector3f::Normalize(isect->dpdu);

			// Compute shading bitangent _ts_ for triangle and adjust _ss_
			Vector3f ts = Vector3f::Cross(ss, ns);
			if (ts.LengthSquared() > 0.f) 
			{
				ts = Vector3f::Normalize(ts);
				ss = Vector3f::Cross(ts, ns);
			}
			else
				CoordinateSystem((Vector3f)ns, &ss, &ts);

			// Compute $\dndu$ and $\dndv$ for triangle shading geometry
			Vector3f dndu, dndv;
			if (mesh->n) 
			{
				// Compute deltas for triangle partial derivatives of normal
				Vector2f duv02 = uv[0] - uv[2];
				Vector2f duv12 = uv[1] - uv[2];
				Vector3f dn1 = n0 - n2;
				Vector3f dn2 = n1 - n2;
				Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
				bool degenerateUV = std::abs(determinant) < 1e-8;
				if (degenerateUV) 
				{
					// We can still compute dndu and dndv, with respect to the
					// same arbitrary coordinate system we use to compute dpdu
					// and dpdv when this happens. It's important to do this
					// (rather than giving up) so that ray differentials for
					// rays reflected from triangles with degenerate
					// parameterizations are still reasonable.
					Vector3f dn = Vector3f::Cross(Vector3f(n2 - n0),
						Vector3f(n1 - n0));
					if (dn.LengthSquared() == 0)
						dndu = dndv = Vector3f(0, 0, 0);
					else 
					{
						Vector3f dnu, dnv;
						CoordinateSystem(dn, &dnu, &dnv);
						dndu = Vector3f(dnu);
						dndv = Vector3f(dnv);
					}
				}
				else 
				{
					Float invDet = 1 / determinant;
					dndu = (duv12[1] * dn1 - duv02[1] * dn2) * invDet;
					dndv = (-duv12[0] * dn1 + duv02[0] * dn2) * invDet;
				}
			}
			else
				dndu = dndv = Vector3f(0, 0, 0);
			//if (reverseOrientation) 
			//	ts = -ts;
			isect->SetGeometryShading(ss, ts, dndu, dndv, true);
		}

		*tHit = t;
		//++nHits;
		return true;
    }

    bool Triangle::IntersectMoller(const Ray& ray, Float* tHit, SurfaceInteraction* isect) const
    {
		//这里和pbrt有所不同，用的是Möller-Trumbore algorithm
		//

		const Point3f& p0 = mesh->p[vIndices[0]];
		const Point3f& p1 = mesh->p[vIndices[1]];
		const Point3f& p2 = mesh->p[vIndices[2]];
		Point3f A = mTransform->ObjectToWorldPoint(p0);
		Point3f B = mTransform->ObjectToWorldPoint(p1);
		Point3f C = mTransform->ObjectToWorldPoint(p2);

		//假设交点是P(先不管交点是否在三角形内)，都满足重心坐标：
		//P = (1 - u - v)A + uB + vC     (1)式
		//由于P = O + tD，P是射线上一点,代入(1)式
		//O + tD = A + u(B - A) + v(C - A)
		//O - A = -tD + u(B - A) + v(C - A)
		//上式写成矩阵形式：
		//[O - A] = [D B - A C - A][-t]
		//                         [u]
		//                         [v]
		//T = O - A, E1 = B - A, E2 = C - A
		//[T] = [D E1 E2][-t]   -t的负号移到D上  [T] = [-D E1 E2][t]
		//               [u]                                     [u]
		//               [v]                                     [v]
		//

		Vector3f E1 = B - A;
		Vector3f E2 = C - A;
		Vector3f T = ray.o - A;

		//通过Cramer's Rule来求t u v
		//t = |[T E1 E2]| / |[-D E1 E2]| 
		//u = |[-D T E2]| / |[-D E1 E2]|
		//v = |[-D E1 T]| / |[-D E1 E2]|

		//det = |[-D E1 E2]| = -(-D x E2)·E1   行列式性质：第一列乘以-1 = -1 乘以 行列式
		//    = (D x E2)·E1 = (E2 x E1)·D
		//可以看得出，如果E2xE1和D平衡，det = 0，即三角形的面和ray.d平衡，无相交
		Float det = Vector3f::Dot(Vector3f::Cross(ray.d, E2), E1);

		if (std::abs(det) <= 1e-8)  //射线和三角面平衡
		{
			return false;
		}

		Float invDet = 1.0f / det;

		//继续上面的注释：
		//t = |[T E1 E2]| / |[-D E1 E2]|   行列式性质：两列交换改变符号
		//  = -|[T E2 E1]| / |[-D E1 E2]|
		//  = (T x E1)·E2 / det
		Float t = Vector3f::Dot(Vector3f::Cross(T, E1), E2) * invDet;
		if (t < 0)
			return false;

		//u = |[-D T E2]| / |[-D E1 E2]|   行列式性质：第一列乘以-1 = -1 乘以 行列式
		//  = (-1) * -(D x E2)·T / det
		//  = (D x E2)·T / det
		Float u = Vector3f::Dot(Vector3f::Cross(ray.d, E2), T) * invDet;
		if (u < 0 || u > 1)
			return false;

		//v = (D x T)·E1 / det
		Float v = Vector3f::Dot(Vector3f::Cross(ray.d, T), E1) * invDet;
		if (v < 0 || v > 1)
			return false;

		//求dpdu，dpdv
		Vector3f dpdu, dpdv;
		Point2f uv[3];
		GetUVs(uv);
		//由于三角形的两点可以通过下面式子表示：
		//pi = p0 + ui∂p/∂u + vi∂p/∂v
		//p2 = p0 + (u2 - u0)∂p/∂u + (v2 - v0)∂p/∂v
		//p2 = p1 + (u2 - u1)∂p/∂u + (v2 - v1)∂p/∂v
		//取得矩阵：
		//[p2 - p0] = [u0 - u2  v0 - v2] [∂p/∂u]
		//[p2 - p1] = [u1 - u2  v1 - v2] [∂p/∂v]
		//[∂p/∂u] = [u0 - u2  v0 - v2]^(-1) = [p2 - p0]
		//[∂p/∂v]   [u1 - u2  v1 - v2]        [p2 - p1]
		Vector2f duv02 = uv[0] - uv[2], duv12 = uv[1] - uv[2];
		Vector3f dp02 = A - C, dp12 = B - C;
		Float determinant = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		bool degenerateUV = std::abs(determinant) < 1e-8;
		if (!degenerateUV)
		{
			Float invdet = 1 / determinant;
			dpdu = (duv12[1] * dp02 - duv02[1] * dp12) * invdet;
			dpdv = (-duv12[0] * dp02 + duv02[0] * dp12) * invdet;
		}
		if (degenerateUV || Vector3f::Cross(dpdu, dpdv).LengthSquared() == 0)
		{
			//方程组无解
			// Handle zero determinant for triangle partial derivative matrix
			Vector3f ng = Vector3f::Cross(C - A, B - A);
			if (ng.LengthSquared() == 0)
				// The triangle is actually degenerate; the intersection is
				// bogus.
				return false;

			CoordinateSystem(Vector3f::Normalize(ng), &dpdu, &dpdv);
		}
		Float w = 1.0f - u - v;
		Float xAbsSum =
			(std::abs(w * A.x) + std::abs(u * B.x) + std::abs(v * C.x));
		Float yAbsSum =
			(std::abs(w * A.y) + std::abs(u * B.y) + std::abs(v * C.y));
		Float zAbsSum =
			(std::abs(w * A.z) + std::abs(u * B.z) + std::abs(v * C.z));
		Vector3f pError = gamma(7) * Vector3f(xAbsSum, yAbsSum, zAbsSum);

		Point3f pHit = w * A + u * B + v * C;
		Point2f uvHit = w * uv[0] + u * uv[1] + v * uv[2];

		*isect = SurfaceInteraction(pHit, pError, uvHit, -ray.d, dpdu, dpdv,
			Vector3f(0, 0, 0), Vector3f(0, 0, 0), ray.time, this);

		//isect的normal由dpdu dpdv的纹理坐标cross求得，但实质上并不是由纹理坐标决定normal的
		//需要重新给他的normal赋值
		isect->normal = isect->shading.n = Vector3f::Normalize(Vector3f::Cross(dp02, dp12));

		//if (mesh->n)   //这步不需要，因为三角形的生成是保证了他的法线和顶点顺序一致的。
		//    isect->normal = Vector3f::FaceForward(isect->normal, isect->shading.n);
		if (mesh->n || mesh->s)
		{
			Vector3f ns = isect->normal;
			if (mesh->n)
			{
				//插值
				//mesh里的n是objectSpace下的
				Vector3f ns = w * mesh->n[vIndices[0]] + u * mesh->n[vIndices[1]] + v * mesh->n[vIndices[2]];
				if (ns.LengthSquared() > 0)
					ns = Vector3f::Normalize(mTransform->ObjectToWorldNormal(ns));
				else
					ns = isect->normal;
			}

			Vector3f ss = isect->dpdu;
			if (mesh->s)
			{
				//mesh里的s是objectSpace下的
				ss = w * mesh->s[vIndices[0]] + u * mesh->s[vIndices[1]] + v * mesh->s[vIndices[2]];
				if (ss.LengthSquared() > 0)
					ss = Vector3f::Normalize(mTransform->ObjectToWorldVector(ss));
				else
					ss = isect->dpdu;
			}

			Vector3f ts = Vector3f::Cross(ss, ns);

			if (ts.LengthSquared() > 0.f) {
				ts = Vector3f::Normalize(ts);
				ss = Vector3f::Cross(ts, ns);
			}
			else
				CoordinateSystem((Vector3f)ns, &ss, &ts);

			//计算dndu dndv
			//由于三角形的法线可以通过下面式子表示：
			//ni = n0 + ui∂n/∂u + vi∂n/∂v
			//n2 = n0 + (u2 - u0)∂n/∂u + (v2 - v0)∂p/∂v
			//n2 = n1 + (u2 - u1)∂n/∂u + (v2 - v1)∂p/∂v
			//取得矩阵：
			//[n0 - n2] = [u0 - u2  v0 - v2] [∂n/∂u]
			//[n1 - n2] = [u1 - u2  v1 - v2] [∂n/∂v]
			//[∂n/∂u] = [u0 - u2  v0 - v2]^(-1) = [p2 - p0]
			//[∂n/∂v]   [u1 - u2  v1 - v2]        [p2 - p1]
			Vector3f dndu, dndv;
			if (mesh->n)
			{
				Vector3f nw0 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[0]]));
				Vector3f nw1 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[1]]));
				Vector3f nw2 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[2]]));
				Vector3f dn02 = nw0 - nw1;
				Vector3f dn12 = nw1 - nw2;

				bool degenerateUV = std::abs(determinant) < 1e-8;
				if (degenerateUV) {
					// We can still compute dndu and dndv, with respect to the
					// same arbitrary coordinate system we use to compute dpdu
					// and dpdv when this happens. It's important to do this
					// (rather than giving up) so that ray differentials for
					// rays reflected from triangles with degenerate
					// parameterizations are still reasonable.
					Vector3f dn = Vector3f::Cross(nw2 - nw0, nw1 - nw0);
					if (dn.LengthSquared() == 0)
						dndu = dndv = Vector3f(0, 0, 0);
					else
					{
						Vector3f dnu, dnv;
						CoordinateSystem(dn, &dnu, &dnv);
						dndu = dnu;
						dndv = dnv;
					}
				}
				else
				{
					Float invDet = 1 / determinant;
					dndu = (duv12[1] * dn02 - duv02[1] * dn12) * invDet;
					dndv = (-duv12[0] * dn02 + duv02[0] * dn12) * invDet;
				}
			}
			else
				dndu = dndv = Vector3f::zero;

			isect->SetGeometryShading(ss, ts, dndu, dndv, true);
		}

		*tHit = t;
		return true;
    }

    bool Triangle::IntersectP(const Ray& ray) const
    {
        //Möller-Trumbore algorithm
        const Point3f& p0 = mesh->p[vIndices[0]];
        const Point3f& p1 = mesh->p[vIndices[1]];
        const Point3f& p2 = mesh->p[vIndices[2]];
        Point3f A = mTransform->ObjectToWorldPoint(p0);
        Point3f B = mTransform->ObjectToWorldPoint(p1);
        Point3f C = mTransform->ObjectToWorldPoint(p2);

        //假设交点是P(先不管交点是否在三角形内)，都满足重心坐标：
        //P = (1 - u - v)A + uB + vC     (1)式
        //由于P = O + tD，P是射线上一点,代入(1)式
        //O + tD = A + u(B - A) + v(C - A)
        //O - A = -tD + u(B - A) + v(C - A)
        //上式写成矩阵形式：
        //[O - A] = [D B - A C - A][-t]
        //                         [u]
        //                         [v]
        //T = O - A, E1 = B - A, E2 = C - A
        //[T] = [D E1 E2][-t]   -t的负号移到D上  [T] = [-D E1 E2][t]
        //               [u]                                     [u]
        //               [v]                                     [v]
        //

        Vector3f E1 = B - A;
        Vector3f E2 = C - A;
        Vector3f T = ray.o - A;

        //通过Cramer's Rule来求t u v
        //t = |[T E1 E2]| / |[-D E1 E2]| 
        //u = |[-D T E2]| / |[-D E1 E2]|
        //v = |[-D E1 T]| / |[-D E1 E2]|

        //det = |[-D E1 E2]| = -(-D x E2)·E1   行列式性质：第一列乘以-1 = -1 乘以 行列式
        //    = (D x E2)·E1 = (E2 x E1)·D
        //可以看得出，如果E2xE1和D平衡，det = 0，即三角形的面和ray.d平衡，无相交
        Float det = Vector3f::Dot(Vector3f::Cross(ray.d, E2), E1);

        if (std::abs(det) <= 1e-8)  //射线和三角面平衡
        {
            return false;
        }

        Float invDet = 1.0f / det;

        //继续上面的注释：
        //t = |[T E1 E2]| / |[-D E1 E2]|   行列式性质：两列交换改变符号
        //  = -|[T E2 E1]| / |[-D E1 E2]|
        //  = (T x E1)·E2 / det
        Float t = Vector3f::Dot(Vector3f::Cross(T, E1), E2) * invDet;
        if (t < 0)
            return false;

        //u = |[-D T E2]| / |[-D E1 E2]|   行列式性质：第一列乘以-1 = -1 乘以 行列式
        //  = (-1) * -(D x E2)·T / det
        //  = (D x E2)·T / det
        Float u = Vector3f::Dot(Vector3f::Cross(ray.d, E2), T) * invDet;
        if (u < 0 || u > 1)
            return false;

        //v = (D x T)·E1 / det
        Float v = Vector3f::Dot(Vector3f::Cross(ray.d, T), E1) * invDet;
        if (v < 0 || v > 1)
            return false;

        return true;
    }

    Interaction Triangle::Sample(const Point2f& u, Float* pdf) const
    {
        //均匀采样
        Point2f b = UniformSampleTriangle(u);

        //mesh里是objectspace下的数值
        const Point3f& p0 = mesh->p[vIndices[0]];
        const Point3f& p1 = mesh->p[vIndices[1]];
        const Point3f& p2 = mesh->p[vIndices[2]];
        Point3f p0w = mTransform->ObjectToWorldPoint(p0);
        Point3f p1w = mTransform->ObjectToWorldPoint(p1);
        Point3f p2w = mTransform->ObjectToWorldPoint(p2);

		Interaction it;
        Float w = 1.0f - b.x - b.y;
        //既然是均匀采样，那么u,v,w如何分配都不重要了，因为最后都是三角形内均匀分布的一点。
        it.interactPoint = p0w * w + p1w * b.x + p2w * b.y;

        if (mesh->n)
        {
            Vector3f nw0 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[0]]));
            Vector3f nw1 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[1]]));
            Vector3f nw2 = Vector3f::Normalize(mTransform->ObjectToWorldNormal(mesh->n[vIndices[2]]));

            it.normal = Vector3f::Normalize(w * nw0 +
                b.x * nw1 +
                b.y * nw2);
        }
        else
            it.normal = Vector3f::Normalize(Vector3f::Cross(p1w - p0w, p2w - p0w));

        Point3f pAbsSum = Vector3f::Abs(w * p0) + Vector3f::Abs(b[0] * p1w) +
            Vector3f::Abs(b[1] * p2w);
        it.pError = gamma(6) * Vector3f(pAbsSum);

        *pdf = 1.0f / Area();
        return it;
    }

    //Area = |Cross(a, b)| * 0.5;
    //|Cross(a, b)| = |a||b|sinθ
    Float Triangle::Area() const
    {
        const Point3f& p0 = mesh->p[vIndices[0]];
        const Point3f& p1 = mesh->p[vIndices[1]];
        const Point3f& p2 = mesh->p[vIndices[2]];
        Point3f p0w = mTransform->ObjectToWorldPoint(p0);
        Point3f p1w = mTransform->ObjectToWorldPoint(p1);
        Point3f p2w = mTransform->ObjectToWorldPoint(p2);
        return 0.5 * Vector3f::Cross(p1w - p0w, p2w - p0w).Length();
    }
}
