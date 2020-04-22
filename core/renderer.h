#pragma once
#include <string>

namespace AIR
{
	class Integrator;
	class Sampler;
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
