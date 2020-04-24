#pragma once
#include <string>
#include <vector>
#include "memory.h"

namespace AIR
{
	class Integrator;
	class Sampler;
	class Transform;

	//这里，我把Transform当成是Unity的GameObject
	//每个Primitive有一个Transform的指针
	//可以理解成一个Transform有一堆Primitive的集合。
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
