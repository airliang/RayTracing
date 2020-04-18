#include "bvhaccel.h"
#include "memory.h"

namespace AIR
{
	struct BVHPrimitiveInfo 
	{
		BVHPrimitiveInfo() {}
		BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f& bounds)
			: primitiveNumber(primitiveNumber),
			bounds(bounds),
			centroid(.5f * bounds.pMin + .5f * bounds.pMax) {}
		//在primitives数组中的索引
		size_t primitiveNumber;
		//对应primitive的worldbound
		Bounds3f bounds;
		//bound的中心点
		Point3f centroid;
	};

	struct BVHBuildNode 
	{
		// BVHBuildNode Public Methods
		//调用该方法表面BVHBuildNode就是一个叶子节点
		void InitLeaf(int first, int n, const Bounds3f& b) 
		{
			firstPrimOffset = first;
			nPrimitives = n;
			bounds = b;
			children[0] = children[1] = nullptr;
			//++leafNodes;
			//++totalLeafNodes;
			//totalPrimitives += n;
		}
		void InitInterior(int axis, BVHBuildNode* c0, BVHBuildNode* c1) 
		{
			children[0] = c0;
			children[1] = c1;
			bounds = Union(c0->bounds, c1->bounds);
			splitAxis = axis;
			nPrimitives = 0;
			//++interiorNodes;
		}
		Bounds3f bounds;
		BVHBuildNode* children[2];
		int splitAxis;
		//在BVHAccel::primivites中的索引
		int firstPrimOffset;
		//leaf中挂了多少个primitive
		int nPrimitives;
	};

	struct LinearBVHNode {
		Bounds3f bounds;
		union {
			//在primtives数组中的索引
			int primitivesOffset;    // leaf

			int secondChildOffset;   // interior
		};
		//如果是叶子节点，拥有的primitves的数量
		//因为在build tree node时，primitive已经根据放置的node做了排序
		//所以primitve是和node紧凑的，在同一个树叶下primitve的顺序是连续放在primitives数组下的
		uint16_t nPrimitives;  // 0 -> interior node
		uint8_t axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	struct BucketInfo 
	{
		//拥有的primitive的数量
		int count = 0;
		//bucket的bounds
		Bounds3f bounds;
	};

	std::shared_ptr<RObject> BVHAccel::CreateBVHAccelerator(std::vector<std::shared_ptr<RObject>> prims, int maxPrimsInNode, const std::string& splitName)
	{
		SplitMethod method = SplitMethod::SAH;
		if (splitName == "middle")
		{
			method = SplitMethod::Middle;
		}
		else if (splitName == "equalcounts")
		{
			method = SplitMethod::EqualCounts;
		}

		return std::make_shared<BVHAccel>(std::move(prims), maxPrimsInNode, method);
	}

	BVHAccel::BVHAccel(std::vector<std::shared_ptr<RObject>> p, int maxPrimsInNode,
		SplitMethod splitMethod)
		: maxPrimsInNode(std::min(255, maxPrimsInNode)),
		primitives(std::move(p)),
		splitMethod(splitMethod)
	{
		std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
		for (size_t i = 0; i < primitives.size(); ++i)
			primitiveInfo[i] = { i, primitives[i]->WorldBound() };

		MemoryArena arena(1024 * 1024);
		int totalNodes = 0;
		std::vector<std::shared_ptr<RObject>> orderedPrims;
		BVHBuildNode* root;

		root = recursiveBuild(arena, primitiveInfo, 0, primitives.size(),
				&totalNodes, orderedPrims);

		//primitives is replaced with the orderedPrims
		primitives.swap(orderedPrims);
		primitiveInfo.resize(0);

		linearNodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		FlattenBVHTree(root, &offset);
	}

	BVHAccel::~BVHAccel()
	{
		FreeAligned(linearNodes);
	}

	BVHBuildNode* BVHAccel::recursiveBuild(
		MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfo,
		int start, int end, int* totalNodes,
		std::vector<std::shared_ptr<RObject>>& orderedPrims)
	{
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		(*totalNodes)++;

		Bounds3f bounds;
		for (int i = start; i < end; ++i)
		{
			bounds = Bounds3f::Union(bounds, primitiveInfo[i].bounds);
		}
		node->bounds = bounds;

		//判断数组长度
		int nPrimitives = end - start;
		if (nPrimitives == 1)
		{
			//数组是1的时候不能再往下划分，创建leaf
			int firstPrimOffset = orderedPrims.size();
			int primIndex = primitiveInfo[start].primitiveNumber;
			orderedPrims.push_back(primitives[primIndex]);
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}

		//开始划分子节点
		//首先计算出primitive的中心点构成的Bounds
		Bounds3f centroidBounds;
		for (int i = start; i < end; ++i)
		{
			centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
		}
		int dim = centroidBounds.MaximumExtent();

		//假如centroidBounds是一个点
		//即上面的primitiveInfo的中心点在同一个位置
		int mid = (start + end) / 2;
		if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim])
		{
			//build the leaf BVHBuildNode
			int firstPrimOffset = orderedPrims.size();
			for (int i = start; i < end; ++i) 
			{
				int primNum = primitiveInfo[i].primitiveNumber;
				orderedPrims.push_back(primitives[primNum]);
			}
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else
		{
			//所有的划分就是求出mid
			//然后重新排序primitiveInfo
			//排序原则：沿着划分的轴，< centroid的在mid前面，否则在mid后面
			switch (splitMethod)
			{
				//按centroidBound的中心点划分
			case SplitMethod::Middle:
			{
				Float pmid = (centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
				BVHPrimitiveInfo* midPtr =
					std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[dim, pmid](const BVHPrimitiveInfo& pi) {
					return pi.centroid[dim] < pmid;
				});
				mid = midPtr - &primitiveInfo[0];
				if (mid != start && mid != end)
					break;
				break;
			}
				//按primitiveInfo等量划分
			case SplitMethod::EqualCounts:
				mid = (start + end) / 2;
				std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
					&primitiveInfo[end - 1] + 1,
					[dim](const BVHPrimitiveInfo& a, const BVHPrimitiveInfo& b) {
					return a.centroid[dim] < b.centroid[dim];
				});
				break;
			case SplitMethod::SAH:
			default:
			{
				if (nPrimitives <= 2) {
					// Partition primitives into equally-sized subsets
					mid = (start + end) / 2;
					std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
						&primitiveInfo[end - 1] + 1,
						[dim](const BVHPrimitiveInfo& a,
							const BVHPrimitiveInfo& b) {
						return a.centroid[dim] <
							b.centroid[dim];
					});
				}
				else
				{
					constexpr int nBuckets = 12;
					BucketInfo buckets[nBuckets];

					// Initialize _BucketInfo_ for SAH partition buckets
					for (int i = start; i < end; ++i) 
					{
						//计算当前的RObject属于哪个bucket
						int b = nBuckets *
							centroidBounds.Offset(
								primitiveInfo[i].centroid)[dim];
						if (b == nBuckets) 
							b = nBuckets - 1;
						//CHECK_GE(b, 0);
						//CHECK_LT(b, nBuckets);
						buckets[b].count++;
						//计算bucket的bounds
						buckets[b].bounds =
							Union(buckets[b].bounds, primitiveInfo[i].bounds);
					}

					//分组，计算每组的cost
					//cost(A,B) = t_trav + pA∑t_isect(ai) + pB∑t_isect(ai)
					//t_trav = 0.125; t_isect = 1
					Float cost[nBuckets - 1];
					for (int i = 0; i < nBuckets - 1; ++i) 
					{
						Bounds3f bA, bB;
						int count0 = 0, count1 = 0;
						for (int j = 0; j <= i; ++j) 
						{
							bA = Union(bA, buckets[j].bounds);
							count0 += buckets[j].count; 
						}
						for (int j = i + 1; j < nBuckets; ++j) 
						{
							bB = Union(bB, buckets[j].bounds);
							count1 += buckets[j].count;
						}
						//t_trav = 0.125f
						cost[i] = 0.125f +
							(count0 * bA.SurfaceArea() +
								count1 * bB.SurfaceArea()) /
							bounds.SurfaceArea();
					}

					// Find bucket to split at that minimizes SAH metric
					Float minCost = cost[0];
					int minCostSplitBucket = 0;
					for (int i = 1; i < nBuckets - 1; ++i) 
					{
						if (cost[i] < minCost) 
						{
							minCost = cost[i];
							minCostSplitBucket = i;
						}
					}

					//生成叶子节点或子树
					Float leafCost = nPrimitives;
					if (nPrimitives > maxPrimsInNode || minCost < leafCost)
					{
						BVHPrimitiveInfo* pmid = std::partition(&primitiveInfo[start],
							&primitiveInfo[end - 1] + 1,
							[=](const BVHPrimitiveInfo& pi) {
							int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
							if (b == nBuckets) b = nBuckets - 1;
							return b <= minCostSplitBucket;
						});
						mid = pmid - &primitiveInfo[0];
					}
					else
					{
						int firstPrimOffset = orderedPrims.size();
						for (int i = start; i < end; ++i) {
							int primNum = primitiveInfo[i].primitiveNumber;
							orderedPrims.push_back(primitives[primNum]);
						}
						node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
						return node;
					}
				}
			}
				break;
			}

			node->InitInterior(dim, recursiveBuild(arena, primitiveInfo, start, mid, totalNodes, orderedPrims),
				recursiveBuild(arena, primitiveInfo, mid, end, totalNodes, orderedPrims));
		}

		

		return node;
	}

	bool BVHAccel::Intersect(const Ray& ray, Interaction* isect) const
	{
		bool hit = false;
		Vector3f invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
		int currentNodeIndex; //当前正在访问的node
		int nodesToVisit[64]; 
		while (true)
		{
			const LinearBVHNode* node = &linearNodes[currentNodeIndex];
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
			}
		}
		return hit;
	}

	int BVHAccel::FlattenBVHTree(BVHBuildNode* node, int* offset)
	{
		LinearBVHNode* linearNode = &linearNodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if (node->nPrimitives > 0)
		{
			//是一个叶子节点
			linearNode->nPrimitives = node->nPrimitives;
			linearNode->primitivesOffset = node->firstPrimOffset;
		}
		else
		{
			linearNode->axis = node->splitAxis;
			linearNode->nPrimitives = 0;
			//这里返回了offset
			FlattenBVHTree(node->children[0], offset);
			linearNode->secondChildOffset = FlattenBVHTree(node->children[1], offset);
		}

		return myOffset;
	}
}