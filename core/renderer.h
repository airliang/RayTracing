#pragma once


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
	protected:
	private:
		Renderer();

		Integrator* CreateIntegrator();
		Sampler* CreateSampler();
	};
}
