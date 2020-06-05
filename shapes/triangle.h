#include "shape.h"
#include <vector>

namespace AIR
{
    struct TriangleMesh
    {
        TriangleMesh(int nTriangles, const int* vertexIndices,
            int nVertices, const Point3f* P, const Vector3f* S, const Vector3f* N,
            const Point2f* uv);

        const int nTriangles; //����������
        const int nVertices;  //��������
        std::vector<int> vertexIndices;  //�����ζ�Ӧ�Ķ���λ�ã�������nTriangles * 3
        std::unique_ptr<Point3f[]> p;   //λ��
        std::unique_ptr<Vector3f[]> n;   //normals
        std::unique_ptr<Vector3f[]> s;   //tangents x����
        std::unique_ptr<Point2f[]> uv;    //texture coordinates
    };

    //ע�⣺���Triangle��ָһ�������Σ�������TriangleMesh�ļ���
	class Triangle : public Shape
	{
	public:
        //Transform �任��Ϣ
        //mesh Triangle�����ĸ�mesh
        //triNumber ����������mesh������������
		Triangle(Transform* pTransform, const std::shared_ptr<TriangleMesh>& mesh, int triNumber);

        Bounds3f ObjectBound() const;
        Bounds3f WorldBound() const;
        bool Intersect(const Ray& ray, Float* tHit, SurfaceInteraction* isect) const;
        bool IntersectP(const Ray& ray) const;

        //���������ε����
        //��������ϵ�µ�
        Float Area() const;

        using Shape::Sample;  // Bring in the other Sample() overload.
        Interaction Sample(const Point2f& u, Float* pdf) const;

        // Returns the solid angle subtended by the triangle w.r.t. the given
        // reference point p.
        Float SolidAngle(const Point3f& p, int nSamples = 0) const;

    private:
        bool IntersectMoller(const Ray& ray, Float* tHit, SurfaceInteraction* isect) const;
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
