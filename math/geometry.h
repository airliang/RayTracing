#pragma once

#include "../RayTracing.h"
#include "mathdef.h"

namespace AIR
{
	class Ray;

	template <typename T>
	inline bool isNaN(const T x) {
		return std::isnan(x);
	}
	template <>
	inline bool isNaN(const int x) {
		return false;
	}

	template <typename T>
	class Vector2 {
	public:
		// Vector2 Public Methods
		Vector2() 
		{ 
			x = y = 0; 
		}
		Vector2(T xx, T yy) : x(xx), y(yy) {  }
		bool HasNaNs() const { return isNaN(x) || isNaN(y); }

		template <typename U>
		explicit Vector2(const Vector2<U> &p) {
			x = (T)p.x;
			y = (T)p.y;
			//DCHECK(!HasNaNs());
		}

#ifndef NDEBUG
		// The default versions of these are fine for release builds; for debug
		// we define them so that we can add the Assert checks.
		Vector2(const Vector2<T> &v) {
			 
			x = v.x;
			y = v.y;
		}
		Vector2<T> &operator=(const Vector2<T> &v) {
			 
			x = v.x;
			y = v.y;
			return *this;
		}
#endif  // !NDEBUG

		Vector2<T> operator+(const Vector2<T> &v) const {
			 
			return Vector2(x + v.x, y + v.y);
		}

		Vector2<T> &operator+=(const Vector2<T> &v) {
			 
			x += v.x;
			y += v.y;
			return *this;
		}
		Vector2<T> operator-(const Vector2<T> &v) const {
			 
			return Vector2(x - v.x, y - v.y);
		}

		Vector2<T> &operator-=(const Vector2<T> &v) {
			 
			x -= v.x;
			y -= v.y;
			return *this;
		}
		bool operator==(const Vector2<T> &v) const { return x == v.x && y == v.y; }
		bool operator!=(const Vector2<T> &v) const { return x != v.x || y != v.y; }
		template <typename U>
		Vector2<T> operator*(U f) const {
			return Vector2<T>(f * x, f * y);
		}

		template <typename U>
		Vector2<T> &operator *= (U f) {
			x *= f;
			y *= f;
			return *this;
		}
		template <typename U>
		Vector2<T> operator /(U f) const {

			Float inv = (Float)1 / f;
			return Vector2<T>(x * inv, y * inv);
		}

		template <typename U>
		Vector2<T> &operator /= (U f) {

			Float inv = (Float)1 / f;
			x *= inv;
			y *= inv;
			return *this;
		}
		Vector2<T> operator-() const { return Vector2<T>(-x, -y); }
		T operator[](int i) const {

			if (i == 0) return x;
			return y;
		}

		T &operator[](int i) {

			if (i == 0) return x;
			return y;
		}
		Float LengthSquared() const { return x * x + y * y; }
		Float Length() const { return std::sqrt(LengthSquared()); }

		static T Dot(const Vector2<T> &v1, const Vector2<T> &v2)
		{
			return v1.x * v2.x + v1.y * v2.y;
		}

		Vector2<T> Normalize()
		{
			float len = Length();
			x /= len;
			y /= len;
			return *this;
		}

		static Vector2<T> Normalize(const Vector2<T>& v)
		{
			return v / v.Length();
		}

		static T Distance(const Vector2<T>& v1, const Vector2<T>& v2)
		{
			(v1 - v2).Length();
		}

		static Vector2<T> Abs(const Vector2<T> &v)
		{
			return Vector2<T>(std::abs(v.x), std::abs(v.y));
		}

		static Vector2<T> Floor(const Vector2<T> &v)
		{
			return Vector2<T>(std::floor(v.x), std::floor(v.y));
		}

		static Vector2<T> Ceil(const Vector2<T> &v)
		{
			return Vector2<T>(std::ceil(v.x), std::ceil(v.y));
		}

		// Vector2 Public Data
		T x, y;

		static Vector2<T> one;
		static Vector2<T> zero;
	};

	template <typename T>
	class Vector3 {
	public:
		// Vector3 Public Methods
		T operator[](int i) const {
			//DCHECK(i >= 0 && i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}
		T &operator[](int i) {
			//DCHECK(i >= 0 && i <= 2);
			if (i == 0) return x;
			if (i == 1) return y;
			return z;
		}
		Vector3() { x = y = z = 0; }
		Vector3(T x, T y, T z) : x(x), y(y), z(z) {  }
		bool HasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z); }

#ifndef NDEBUG
		// The default versions of these are fine for release builds; for debug
		// we define them so that we can add the Assert checks.
		Vector3(const Vector3<T> &v) {
			x = v.x;
			y = v.y;
			z = v.z;
		}

		Vector3<T> &operator=(const Vector3<T> &v) {
			 
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
#endif  // !NDEBUG
		Vector3<T> operator+(const Vector3<T> &v) const {
			 
			return Vector3(x + v.x, y + v.y, z + v.z);
		}
		Vector3<T> &operator+=(const Vector3<T> &v) {
			 
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}
		Vector3<T> operator-(const Vector3<T> &v) const {
			 
			return Vector3(x - v.x, y - v.y, z - v.z);
		}
		Vector3<T> &operator-=(const Vector3<T> &v) {
			 
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}
		bool operator==(const Vector3<T> &v) const {
			return x == v.x && y == v.y && z == v.z;
		}
		bool operator!=(const Vector3<T> &v) const {
			return x != v.x || y != v.y || z != v.z;
		}
		template <typename U>
		Vector3<T> operator*(U s) const {
			return Vector3<T>(s * x, s * y, s * z);
		}
		template <typename U>
		Vector3<T> &operator*=(U s) {
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}
		template <typename U>
		Vector3<T> operator/(U f) const {
			Float inv = (Float)1 / f;
			return Vector3<T>(x * inv, y * inv, z * inv);
		}

		template <typename U>
		Vector3<T> &operator/=(U f) {
			Float inv = (Float)1 / f;
			x *= inv;
			y *= inv;
			z *= inv;
			return *this;
		}
		Vector3<T> operator-() const { return Vector3<T>(-x, -y, -z); }
		Float LengthSquared() const { return x * x + y * y + z * z; }
		Float Length() const { return std::sqrt(LengthSquared()); }

		static Vector3<T> Cross(const Vector3<T> &v1, const Vector3<T> &v2)
		{
			float v1x = v1.x, v1y = v1.y, v1z = v1.z;
			float v2x = v2.x, v2y = v2.y, v2z = v2.z;
			return Vector3<T>((v1y * v2z) - (v1z * v2y), (v1z * v2x) - (v1x * v2z),
				(v1x * v2y) - (v1y * v2x));
		}

		static T Dot(const Vector3<T> &v1, const Vector3<T> &v2)
		{
			return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
		}

		Vector3<T> Normalize()
		{
			float len = Length();
			x /= len;
			y /= len;
			z /= len;
			return *this;
		}

		static Vector3<T> Normalize(const Vector3<T>& v)
		{
			return v / v.Length();
		}

		static T Distance(const Vector3<T>& v1, const Vector3<T>& v2)
		{
			return (v1 - v2).Length();
		}

		static Vector3<T> Abs(const Vector3<T> &v) 
		{
			return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
		}

		static Vector3<T> Floor(const Vector3<T> &v)
		{
			return Vector3<T>(std::floor(v.x), std::floor(v.y), std::floor(v.z));
		}

		static Vector3<T> Ceil(const Vector3<T> &v)
		{
			return Vector3<T>(std::ceil(v.x), std::ceil(v.y), std::ceil(v.z));
		}

		
		// Vector3 Public Data
		T x, y, z;

		static Vector3<T> forward; 
		static Vector3<T> up;
		static Vector3<T> right; 
		static Vector3<T> one;
		static Vector3<T> zero;
	};

	typedef Vector2<Float> Vector2f;
	typedef Vector2<int> Vector2i;
	typedef Vector3<Float> Vector3f;
	typedef Vector3<int> Vector3i;

    template <>
	Vector3f Vector3f::forward = Vector3f(0, 0, 1.0f);
	template <>
	Vector3f Vector3f::up = Vector3<float>(0, 1.0f, 0);
	template <>
	Vector3f Vector3f::right = Vector3<float>(1.0f, 0, 0);
	template <>
	Vector3f Vector3f::one = Vector3<float>(1.0f, 1.0f, 1.0f);
	template <>
	Vector3f Vector3f::zero = Vector3<float>(0, 0, 0);
	template <>
	Vector2f Vector2f::one = Vector2f(1.0f, 1.0f);
	template <>
	Vector2f Vector2f::zero = Vector2f(0, 0);

	template <typename T>
	Vector2<T> Min(const Vector2<T> &pa, const Vector2<T> &pb) 
	{
		return Vector2<T>(std::min(pa.x, pb.x), std::min(pa.y, pb.y));
	}

	template <typename T>
	Vector2<T> Max(const Vector2<T> &pa, const Vector2<T> &pb) 
	{
		return Vector2<T>(std::max(pa.x, pb.x), std::max(pa.y, pb.y));
	}

	template <typename T, typename U>
	inline Vector2<T> operator*(U f, const Vector2<T> &v) 
	{
		return v * f;
	}

	template <typename T, typename U>
	inline Vector3<T> operator*(U f, const Vector3<T> &v) 
	{
		return v * f;
	}

	template <typename T>
	Vector3<T> Min(const Vector3<T> &p1, const Vector3<T> &p2) 
	{
		return Vector3<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
			std::min(p1.z, p2.z));
	}

	template <typename T>
	Vector3<T> Max(const Vector3<T> &p1, const Vector3<T> &p2) 
	{
		return Vector3<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
			std::max(p1.z, p2.z));
	}

	typedef Vector2f Point2f;
	typedef Vector2i Point2i;
	typedef Vector3f Point3f;

	class Ray
	{
	public:
		Ray() : tMax(Infinity), time(0.f) { }
		Ray(const Vector3f &o, const Vector3f &d, Float tMax = Infinity,
			Float time = 0.f)
			: o(o), d(d), tMax(tMax), time(time) 
		{ 
		
		}
		Vector3f operator()(Float t) const { return o + d * t; }

		bool HasNaNs() const {
			return (o.HasNaNs() || d.HasNaNs() || std::isnan(tMax));
		}
		//friend std::ostream& operator<<(std::ostream& os, const Ray &r) {
		//	os << "[o=" << r.o << ", d=" << r.d << ", tMax="
		//		<< r.tMax << ", time=" << r.time << "]";
		//	return os;
		//}

		Vector3f o;
		Vector3f d;
		mutable Float tMax;
		Float time;
	};

	class RayDifferential : public Ray 
	{
	public:
		RayDifferential() : hasDifferentials(false) 
		{
		}

		RayDifferential(const Vector3f &o, const Vector3f &d,
			Float tMax = Infinity, Float time = 0.f)
			: Ray(o, d, tMax, time) 
		{
			hasDifferentials = false;
		}
		RayDifferential(const Ray &ray) : Ray(ray) {
			hasDifferentials = false;
		}
		bool HasNaNs() const 
		{
			return Ray::HasNaNs() ||
				(hasDifferentials && (rxOrigin.HasNaNs() || ryOrigin.HasNaNs() ||
					rxDirection.HasNaNs() || ryDirection.HasNaNs()));
		}
		void ScaleDifferentials(Float s) 
		{
			rxOrigin = o + (rxOrigin - o) * s;
			ryOrigin = o + (ryOrigin - o) * s;
			rxDirection = d + (rxDirection - d) * s;
			ryDirection = d + (ryDirection - d) * s;
		}

		bool hasDifferentials;
		Vector3f rxOrigin, ryOrigin;
		Vector3f rxDirection, ryDirection;

	};

	template <typename T>
	class Bounds3 
	{
	public:
		// Bounds3 Public Methods
		Bounds3() {
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();
			pMin = Vector3<T>(maxNum, maxNum, maxNum);
			pMax = Vector3<T>(minNum, minNum, minNum);
		}

		explicit Bounds3(const Vector3<T> &p) : pMin(p), pMax(p) 
		{}

		Bounds3(const Vector3<T> &p1, const Vector3<T> &p2)
			: pMin(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
				std::min(p1.z, p2.z)),
			pMax(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
				std::max(p1.z, p2.z)) 
		{}

		const Vector3<T> &operator[](int i) const
		{
			return (i == 0) ? pMin : pMax;
		}

		Vector3<T> &operator[](int i)
		{
			return (i == 0) ? pMin : pMax;
		}

		bool operator==(const Bounds3<T> &b) const 
		{
			return b.pMin == pMin && b.pMax == pMax;
		}

		bool operator!=(const Bounds3<T> &b) const 
		{
			return b.pMin != pMin || b.pMax != pMax;
		}

		Vector3<T> Corner(int corner) const {
			//DCHECK(corner >= 0 && corner < 8);
			return Vector3<T>((*this)[(corner & 1)].x,
				(*this)[(corner & 2) ? 1 : 0].y,
				(*this)[(corner & 4) ? 1 : 0].z);
		}
		Vector3<T> Diagonal() const 
		{ 
			return pMax - pMin; 
		}

		T SurfaceArea() const {
			Vector3<T> d = Diagonal();
			return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
		}

		T Volume() const 
		{
			Vector3<T> d = Diagonal();
			return d.x * d.y * d.z;
		}

		int MaximumExtent() const 
		{
			Vector3<T> d = Diagonal();
			if (d.x > d.y && d.x > d.z)
				return 0;
			else if (d.y > d.z)
				return 1;
			else
				return 2;
		}

		Vector3<T> Lerp(const Vector3f &t) const 
		{
			return Vector3<T>(AIR::Lerp(t.x, pMin.x, pMax.x),
				AIR::Lerp(t.y, pMin.y, pMax.y),
				AIR::Lerp(t.z, pMin.z, pMax.z));
		}

		Vector3<T> Offset(const Vector3<T> &p) const
		{
			Vector3<T> o = p - pMin;
			if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
			if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
			if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
			return o;
		}

		bool static Inside(const Vector3<T> &p, const Bounds3<T> &b) {
			return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
				p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
		}

		void BoundingSphere(Vector3<T> *center, Float *radius) const {
			*center = (pMin + pMax) / 2;
			*radius = Inside(*center, *this) ? Distance(*center, pMax) : 0;
		}
		template <typename U>
		explicit operator Bounds3<U>() const 
		{
			return Bounds3<U>((Vector3<U>)pMin, (Vector3<U>)pMax);
		}

		bool IntersectP(const Ray &ray, Float *hitt0 = nullptr,
			Float *hitt1 = nullptr) const;

		inline bool IntersectP(const Ray &ray, const Vector3f &invDir,
			const int dirIsNeg[3]) const;


		static Bounds3<T> Union(const Bounds3<T> &b, const Vector3<T> &p) {
			Bounds3<T> ret;
			ret.pMin = Min(b.pMin, p);
			ret.pMax = Max(b.pMax, p);
			return ret;
		}


		static Bounds3<T> Union(const Bounds3<T> &b1, const Bounds3<T> &b2) {
			Bounds3<T> ret;
			ret.pMin = Min(b1.pMin, b2.pMin);
			ret.pMax = Max(b1.pMax, b2.pMax);
			return ret;
		}

	
		static Bounds3<T> Intersect(const Bounds3<T> &b1, const Bounds3<T> &b2) {
			// Important: assign to pMin/pMax directly and don't run the Bounds2()
			// constructor, since it takes min/max of the points passed to it.  In
			// turn, that breaks returning an invalid bound for the case where we
			// intersect non-overlapping bounds (as we'd like to happen).
			Bounds3<T> ret;
			ret.pMin = Max(b1.pMin, b2.pMin);
			ret.pMax = Min(b1.pMax, b2.pMax);
			return ret;
		}

		friend std::ostream &operator<<(std::ostream &os, const Bounds3<T> &b) {
			os << "[ " << b.pMin << " - " << b.pMax << " ]";
			return os;
		}

		// Bounds3 Public Data
		Vector3<T> pMin, pMax;
	};

	typedef Bounds3<Float> Bounds3f;

	template <typename T>
	inline bool Bounds3<T>::IntersectP(const Ray &ray, Float *hitt0,
		Float *hitt1) const 
	{
		Float t0 = 0, t1 = ray.tMax;
		for (int i = 0; i < 3; ++i) {
			// Update interval for _i_th bounding box slab
			Float invRayDir = 1 / ray.d[i];
			Float tNear = (pMin[i] - ray.o[i]) * invRayDir;
			Float tFar = (pMax[i] - ray.o[i]) * invRayDir;

			// Update parametric interval from slab intersection $t$ values
			if (tNear > tFar) std::swap(tNear, tFar);

			// Update _tFar_ to ensure robust ray--bounds intersection
			tFar *= 1 + 2 * gamma(3);
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) return false;
		}
		if (hitt0) *hitt0 = t0;
		if (hitt1) *hitt1 = t1;
		return true;
	}

	template <typename T>
	class Bounds2 
	{
	public:
		// Bounds2 Public Methods
		Bounds2() 
		{
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();
			pMin = Vector2<T>(maxNum, maxNum);
			pMax = Vector2<T>(minNum, minNum);
		}
		explicit Bounds2(const Vector2<T> &p) : pMin(p), pMax(p) {}

		Bounds2(const Vector2<T> &p1, const Vector2<T> &p2)
		{
			pMin = Vector2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
			pMax = Vector2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
		}

		template <typename U>
		explicit operator Bounds2<U>() const {
			return Bounds2<U>((Vector2<U>)pMin, (Vector2<U>)pMax);
		}

		Vector2<T> Diagonal() const 
		{ 
			return pMax - pMin; 
		}

		T Area() const 
		{
			Vector2<T> d = pMax - pMin;
			return (d.x * d.y);
		}

		int MaximumExtent() const 
		{
			Vector2<T> diag = Diagonal();
			if (diag.x > diag.y)
				return 0;
			else
				return 1;
		}

		inline const Vector2<T> &operator[](int i) const 
		{
			//DCHECK(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}

		inline Vector2<T> &operator[](int i) 
		{
			//DCHECK(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}

		bool operator==(const Bounds2<T> &b) const 
		{
			return b.pMin == pMin && b.pMax == pMax;
		}

		bool operator!=(const Bounds2<T> &b) const 
		{
			return b.pMin != pMin || b.pMax != pMax;
		}

		Vector2<T> Lerp(const Point2f &t) const 
		{
			return Vector2<T>(AIR::Lerp(t.x, pMin.x, pMax.x),
				AIR::Lerp(t.y, pMin.y, pMax.y));
		}

		Vector2<T> Offset(const Vector2<T> &p) const
		{
			Vector2<T> o = p - pMin;
			if (pMax.x > pMin.x) 
				o.x /= pMax.x - pMin.x;
			if (pMax.y > pMin.y) 
				o.y /= pMax.y - pMin.y;
			return o;
		}

		void BoundingSphere(Vector2<T> *c, Float *rad) const
		{
			*c = (pMin + pMax) / 2;
			*rad = Inside(*c, *this) ? Distance(*c, pMax) : 0;
		}

		static Bounds2<T> Intersect(const Bounds2<T> &b1, const Bounds2<T> &b2) 
		{
			// Important: assign to pMin/pMax directly and don't run the Bounds2()
			// constructor, since it takes min/max of the points passed to it.  In
			// turn, that breaks returning an invalid bound for the case where we
			// intersect non-overlapping bounds (as we'd like to happen).
			Bounds2<T> ret;
			ret.pMin = Max(b1.pMin, b2.pMin);
			ret.pMax = Min(b1.pMax, b2.pMax);
			return ret;
		}

		friend std::ostream &operator<<(std::ostream &os, const Bounds2<T> &b) 
		{
			os << "[ " << b.pMin << " - " << b.pMax << " ]";
			return os;
		}

		// Bounds2 Public Data
		Vector2<T> pMin, pMax;
	};

	typedef Bounds2<Float> Bounds2f;
	typedef Bounds2<int> Bounds2i;

	class Bounds2iIterator : public std::forward_iterator_tag 
	{
	public:
		Bounds2iIterator(const Bounds2i &b, const Point2i &pt)
			: p(pt), bounds(&b) {}
		Bounds2iIterator operator++() 
		{
			advance();
			return *this;
		}
		Bounds2iIterator operator++(int) 
		{
			Bounds2iIterator old = *this;
			advance();
			return old;
		}
		bool operator==(const Bounds2iIterator &bi) const 
		{
			return p == bi.p && bounds == bi.bounds;
		}
		bool operator!=(const Bounds2iIterator &bi) const 
		{
			return p != bi.p || bounds != bi.bounds;
		}

		Point2i operator*() const { return p; }

	private:
		void advance() 
		{
			++p.x;
			if (p.x == bounds->pMax.x) 
			{
				p.x = bounds->pMin.x;
				++p.y;
			}
		}
		Point2i p;
		const Bounds2i *bounds;
	};

	inline Bounds2iIterator begin(const Bounds2i &b) 
	{
		return Bounds2iIterator(b, b.pMin);
	}

	inline Bounds2iIterator end(const Bounds2i &b) 
	{
		// Normally, the ending point is at the minimum x value and one past
		// the last valid y value.
		Point2i pEnd(b.pMin.x, b.pMax.y);
		// However, if the bounds are degenerate, override the end point to
		// equal the start point so that any attempt to iterate over the bounds
		// exits out immediately.
		if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y)
			pEnd = b.pMin;
		return Bounds2iIterator(b, pEnd);
	}

	template <typename T>
	inline bool Bounds3<T>::IntersectP(const Ray &ray, const Vector3f &invDir,
		const int dirIsNeg[3]) const 
	{
		const Bounds3f &bounds = *this;
		// Check for ray intersection against $x$ and $y$ slabs
		Float tMin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
		Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
		Float tyMin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
		Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;

		// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
		tMax *= 1 + 2 * gamma(3);
		tyMax *= 1 + 2 * gamma(3);
		if (tMin > tyMax || tyMin > tMax) return false;
		if (tyMin > tMin) tMin = tyMin;
		if (tyMax < tMax) tMax = tyMax;

		// Check for ray intersection against $z$ slab
		Float tzMin = (bounds[dirIsNeg[2]].z - ray.o.z) * invDir.z;
		Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;

		// Update _tzMax_ to ensure robust bounds intersection
		tzMax *= 1 + 2 * gamma(3);
		if (tMin > tzMax || tzMin > tMax) 
			return false;
		if (tzMin > tMin) tMin = tzMin;
		if (tzMax < tMax) tMax = tzMax;
		return (tMin < ray.tMax) && (tMax > 0);
	}

	template <typename T>
	Bounds3<T> Union(const Bounds3<T> &b, const Vector3<T> &p) {
		Bounds3<T> ret;
		ret.pMin = Min(b.pMin, p);
		ret.pMax = Max(b.pMax, p);
		return ret;
	}

	template <typename T>
	Bounds3<T> Union(const Bounds3<T> &b1, const Bounds3<T> &b2) {
		Bounds3<T> ret;
		ret.pMin = Min(b1.pMin, b2.pMin);
		ret.pMax = Max(b1.pMax, b2.pMax);
		return ret;
	}

	template <typename T>
	Bounds3<T> Intersect(const Bounds3<T> &b1, const Bounds3<T> &b2) {
		// Important: assign to pMin/pMax directly and don't run the Bounds2()
		// constructor, since it takes min/max of the points passed to it.  In
		// turn, that breaks returning an invalid bound for the case where we
		// intersect non-overlapping bounds (as we'd like to happen).
		Bounds3<T> ret;
		ret.pMin = Max(b1.pMin, b2.pMin);
		ret.pMax = Min(b1.pMax, b2.pMax);
		return ret;
	}

	

	//y is up,so is different from pbrt
	inline Vector3f SphericalDirection(Float sinTheta, Float cosTheta, Float phi) {
		return Vector3f(sinTheta * std::cos(phi), cosTheta,
			sinTheta * std::sin(phi));
	}

	inline Float SphericalTheta(const Vector3f &v) {
		return std::acos(Clamp(v.y, -1, 1));
	}

	inline Float SphericalPhi(const Vector3f &v) {
		Float p = std::atan2(v.z, v.x);
		return (p < 0) ? (p + 2 * Pi) : p;
	}

	
}

