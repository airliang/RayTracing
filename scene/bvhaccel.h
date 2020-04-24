#pragma once
#include "robject.h"

namespace AIR
{
	struct BVHBuildNode;
	struct BVHPrimitiveInfo;
	struct LinearBVHNode;

	class Interaction;

	class BVHAccel : public Primitive
	{
	public:
		enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };
		BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
			int maxPrimsInNode = 1,
			SplitMethod splitMethod = SplitMethod::SAH);
		~BVHAccel();

		bool Intersect(const Ray& ray, Interaction* isect) const;
		bool IntersectP(const Ray& ray) const;

		static std::shared_ptr<Primitive> CreateBVHAccelerator(
			std::vector<std::shared_ptr<Primitive>> prims,
			int maxPrimsInNode = 4,
			const std::string& splitName = "bvh");

	private:
		//递归构建树
		//arena         内存分配器
		//primitiveInfo 场景的primitves抽取信息后放到的队列
		//start         构建子队列在primitiveInfo中的起始位置
		//end           构建子队列在primitiveInfo中的结束位置
		//totalNodes    tracks the total number of BVH nodes(BVHBuildNode) that have been created
		//orderedPrims  
		BVHBuildNode* recursiveBuild(
			MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo,
			int start, int end, int* totalNodes,
			std::vector<std::shared_ptr<Primitive>>& orderedPrims);

		//把树放置到容易遍历的内存结构上
		//也是一个递归的方法
		int FlattenBVHTree(BVHBuildNode* node, int* offset);

		const int maxPrimsInNode;
		std::vector<std::shared_ptr<Primitive>> primitives;
		const SplitMethod splitMethod;
		LinearBVHNode* linearNodes = nullptr;
	};
}
