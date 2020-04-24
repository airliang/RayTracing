#pragma once
#include <string>
#include <vector>
#include "memory.h"

namespace AIR
{
	class Integrator;
	class Sampler;
	class Transform;

	//����Ұ�Transform������Unity��GameObject
	//ÿ��Primitive��һ��Transform��ָ��
	//��������һ��Transform��һ��Primitive�ļ��ϡ�
	class TransformCache {
	public:
		static TransformCache& GetInstance()
		{
			static TransformCache instance;
			return instance;
		}
		

		// TransformCache Public Methods
		Transform* Lookup(const Transform& t);

		void Clear();

	private:
		TransformCache()
			: hashTable(512), hashTableOccupancy(0) {}
		void Insert(Transform* tNew);
		void Grow();

		static uint64_t Hash(const Transform& t);

		// TransformCache Private Data
		std::vector<Transform*> hashTable;
		int hashTableOccupancy;
		MemoryArena arena;
	};

	class Renderer
	{
	public:
		static Renderer& GetInstance()
		{
			static Renderer instance;
			return instance;
		}

		void Init();

		void Run();

		void Cleanup();

		void ParseScene(const std::string& filename);
	protected:
	private:
		Renderer();
	};
}
