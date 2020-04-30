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
		//��primitives�����е�����
		size_t primitiveNumber;
		//��Ӧprimitive��worldbound
		Bounds3f bounds;
		//bound�����ĵ�
		Point3f centroid;
	};

	struct BVHBuildNode 
	{
		// BVHBuildNode Public Methods
		//���ø÷�������BVHBuildNode����һ��Ҷ�ӽڵ�
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
		//��BVHAccel::primivites�е�����
		int firstPrimOffset;
		//leaf�й��˶��ٸ�primitive
		int nPrimitives;
	};

	struct LinearBVHNode {
		Bounds3f bounds;
		union {
			//��primtives�����е�����
			int primitivesOffset;    // leaf

			int secondChildOffset;   // interior
		};
		//�����Ҷ�ӽڵ㣬ӵ�е�primitves������
		//��Ϊ��build tree nodeʱ��primitive�Ѿ����ݷ��õ�node��������
		//����primitve�Ǻ�node���յģ���ͬһ����Ҷ��primitve��˳������������primitives�����µ�
		uint16_t nPrimitives;  // 0 -> interior node
		uint8_t axis;          // interior node: xyz
		uint8_t pad[1];        // ensure 32 byte total size
	};

	struct BucketInfo 
	{
		//ӵ�е�primitive������
		int count = 0;
		//bucket��bounds
		Bounds3f bounds;
	};

	std::shared_ptr<Primitive> BVHAccel::CreateBVHAccelerator(std::vector<std::shared_ptr<Primitive>> prims, int maxPrimsInNode, const std::string& splitName)
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

	BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> p, int maxPrimsInNode,
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
		std::vector<std::shared_ptr<Primitive>> orderedPrims;
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
		std::vector<std::shared_ptr<Primitive>>& orderedPrims)
	{
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		(*totalNodes)++;

		Bounds3f bounds;
		for (int i = start; i < end; ++i)
		{
			bounds = Bounds3f::Union(bounds, primitiveInfo[i].bounds);
		}
		node->bounds = bounds;

		//�ж����鳤��
		int nPrimitives = end - start;
		if (nPrimitives == 1)
		{
			//������1��ʱ���������»��֣�����leaf
			int firstPrimOffset = orderedPrims.size();
			int primIndex = primitiveInfo[start].primitiveNumber;
			orderedPrims.push_back(primitives[primIndex]);
			node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}

		//��ʼ�����ӽڵ�
		//���ȼ����primitive�����ĵ㹹�ɵ�Bounds
		Bounds3f centroidBounds;
		for (int i = start; i < end; ++i)
		{
			centroidBounds = Union(centroidBounds, primitiveInfo[i].centroid);
		}
		int dim = centroidBounds.MaximumExtent();

		//����centroidBounds��һ����
		//�������primitiveInfo�����ĵ���ͬһ��λ��
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
			//���еĻ��־������mid
			//Ȼ����������primitiveInfo
			//����ԭ�����Ż��ֵ��ᣬ< centroid����midǰ�棬������mid����
			switch (splitMethod)
			{
				//��centroidBound�����ĵ㻮��
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
				//��primitiveInfo��������
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
						//���㵱ǰ��Primitive�����ĸ�bucket
						int b = nBuckets *
							centroidBounds.Offset(
								primitiveInfo[i].centroid)[dim];
						if (b == nBuckets) 
							b = nBuckets - 1;
						//CHECK_GE(b, 0);
						//CHECK_LT(b, nBuckets);
						buckets[b].count++;
						//����bucket��bounds
						buckets[b].bounds =
							Union(buckets[b].bounds, primitiveInfo[i].bounds);
					}

					//���飬����ÿ���cost
					//cost(A,B) = t_trav + pA��t_isect(ai) + pB��t_isect(ai)
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

					//����Ҷ�ӽڵ������
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
		int currentNodeIndex = 0; //��ǰ���ڷ��ʵ�node
		//��һ��Ҫ���ʵ�node��nodesToVisit��index
		int toVisitOffset = 0;
		//nodesToVisit��һ��Ҫ���ʵ�node��stack
		int nodesToVisit[64]; 
		while (true)
		{
			const LinearBVHNode* node = &linearNodes[currentNodeIndex];
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				//�����Ҷ�ӽڵ�
				if (node->nPrimitives > 0)
				{
					for (int i = 0; i < node->nPrimitives; i++)
					{
						if (primitives[node->primitivesOffset + i]->Intersect(ray, isect))
						{
							hit = true;
						}
					}
					if (toVisitOffset == 0) 
						break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					//���߷����������ķ�������ǳ���90��
					//�ȷ��ʵڶ����ӽڵ�
					//��һ���ڵ����ǵ�һ���ӽڵ�(currentNodeIndex + 1)
					//���߷����������ķ��� < 90�����෴
					if (dirIsNeg[node->axis]) 
					{
                     	nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                     	currentNodeIndex = node->secondChildOffset;
                  	} 
					else 
					{
                     	nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                     	currentNodeIndex = currentNodeIndex + 1;
                  	}
				}
				
			}
		}
		return hit;
	}

	bool BVHAccel::IntersectP(const Ray& ray) const
	{
		if (linearNodes == nullptr)
		    return false;
		Vector3f invDir(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
		int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
		int currentNodeIndex; //��ǰ���ڷ��ʵ�node
		//��һ��Ҫ���ʵ�node��nodesToVisit��index
		int toVisitOffset = 0;
		//nodesToVisit��һ��Ҫ���ʵ�node��stack
		int nodesToVisit[64]; 
		while (true)
		{
			const LinearBVHNode* node = &linearNodes[currentNodeIndex];
			if (node->bounds.IntersectP(ray, invDir, dirIsNeg))
			{
				//�����Ҷ�ӽڵ�
				if (node->nPrimitives > 0)
				{
					for (int i = 0; i < node->nPrimitives; i++)
					{
						if (primitives[node->primitivesOffset + i]->IntersectP(ray))
						{
							return true;
						}
					}
					if (toVisitOffset == 0) 
						break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else
				{
					//���߷����������ķ�������ǳ���90��
					//�ȷ��ʵڶ����ӽڵ�
					//��һ���ڵ����ǵ�һ���ӽڵ�(currentNodeIndex + 1)
					//���߷����������ķ��� < 90�����෴
					if (dirIsNeg[node->axis]) 
					{
                     	nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                     	currentNodeIndex = node->secondChildOffset;
                  	} 
					else 
					{
                     	nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                     	currentNodeIndex = currentNodeIndex + 1;
                  	}
				}
				
			}
		}
		return false;
	}

	int BVHAccel::FlattenBVHTree(BVHBuildNode* node, int* offset)
	{
		LinearBVHNode* linearNode = &linearNodes[*offset];
		linearNode->bounds = node->bounds;
		int myOffset = (*offset)++;
		if (node->nPrimitives > 0)
		{
			//��һ��Ҷ�ӽڵ�
			linearNode->nPrimitives = node->nPrimitives;
			linearNode->primitivesOffset = node->firstPrimOffset;
		}
		else
		{
			linearNode->axis = node->splitAxis;
			linearNode->nPrimitives = 0;
			//���ﷵ����offset
			FlattenBVHTree(node->children[0], offset);
			linearNode->secondChildOffset = FlattenBVHTree(node->children[1], offset);
		}

		return myOffset;
	}
}