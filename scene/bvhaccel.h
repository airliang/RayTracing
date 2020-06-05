#pragma once
#include "robject.h"

namespace AIR
{
	struct BVHBuildNode;
	struct BVHPrimitiveInfo;
	struct LinearBVHNode;

	class SurfaceInteraction;

	class BVHAccel : public Primitive
	{
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
		BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
			int maxPrimsInNode = 1,
			SplitMethod splitMethod = SplitMethod::SAH);
		~BVHAccel();

		bool Intersect(const Ray& ray, SurfaceInteraction* isect) const;
		bool IntersectP(const Ray& ray) const;

		static std::shared_ptr<Primitive> CreateBVHAccelerator(
			std::vector<std::shared_ptr<Primitive>> prims,
			int maxPrimsInNode = 4,
			const std::string& splitName = "bvh");

	private:
		//�ݹ鹹����
		//arena         �ڴ������
		//primitiveInfo ������primitves��ȡ��Ϣ��ŵ��Ķ���
		//start         �����Ӷ�����primitiveInfo�е���ʼλ��
		//end           �����Ӷ�����primitiveInfo�еĽ���λ��
		//totalNodes    tracks the total number of BVH nodes(BVHBuildNode) that have been created
		//orderedPrims  
		BVHBuildNode* recursiveBuild(
			MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int start, int end, int* totalNodes,
			std::vector<std::shared_ptr<Primitive>>& orderedPrims);

		//�������õ����ױ������ڴ�ṹ��
		//Ҳ��һ���ݹ�ķ���
		int FlattenBVHTree(BVHBuildNode* node, int* offset);

		const int maxPrimsInNode;
		std::vector<std::shared_ptr<Primitive>> primitives;
		const SplitMethod splitMethod;
		LinearBVHNode* linearNodes = nullptr;
	};
}
