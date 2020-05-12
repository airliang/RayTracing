#include "shape.h"
#include <vector>

namespace AIR
{
    struct TriangleMesh
    {
        TriangleMesh(int nTriangles, const int* vertexIndices,
            int nVertices, const Point3f* P, const Vector3f* S, const Vector3f* N,
            const Point2f* uv);

        const int nTriangles; //三角面数量
        const int nVertices;  //顶点数量
        std::vector<int> vertexIndices;  //三角形对应的顶点位置，长度是nTriangles * 3
        std::unique_ptr<Point3f[]> p;   //位置
        std::unique_ptr<Vector3f[]> n;   //normals
        std::unique_ptr<Vector3f[]> s;   //tangents x方向
        std::unique_ptr<Point2f[]> uv;    //texture coordinates
    };

    //注意：这个Triangle是指一个三角形，并不是TriangleMesh的集合
	class Triangle : public Shape
	{
	public:
        //Transform 变换信息
        //mesh Triangle属于哪个mesh
        //triNumber 该三角形在mesh里的三角形序号
		Triangle(Transform* pTransform, const std::shared_ptr<TriangleMesh>& mesh, int triNumber);

        Bounds3f ObjectBound() const;
        Bounds3f WorldBound() const;
        bool Intersect(const Ray& ray, Float* tHit, Interaction* isect) const;
        bool IntersectP(const Ray& ray) const;

        //返回三角形的面积
        //世界坐标系下的
        Float Area() const;

        using Shape::Sample;  // Bring in the other Sample() overload.
        Interaction Sample(const Point2f& u, Float* pdf) const;

        // Returns the solid angle subtended by the triangle w.r.t. the given
        // reference point p.
        Float SolidAngle(const Point3f& p, int nSamples = 0) const;

    private:
        bool IntersectMoller(const Ray& ray, Float* tHit, Interaction* isect) const;
        // Triangle Private Methods
        void GetUVs(Point2f uv[3]) const 
        {
            if (mesh->uv) 
            {
                uv[0] = mesh->uv[vIndices[0]];
                uv[1] = mesh->uv[vIndices[1]];
                uv[2] = mesh->uv[vIndices[2]];
            }
            else {
                uv[0] = Point2f(0, 0);
                uv[1] = Point2f(1, 0);
                uv[2] = Point2f(1, 1);
            }
        }

        std::shared_ptr<TriangleMesh> mesh;
        const int* vIndices;
	};
}
