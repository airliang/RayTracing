#pragma once
#include "integrator.h"

namespace AIR
{
	// LightStrategy Declarations
	enum class LightStrategy 
	{ 
		//����ȫ����Դ�������ݹ�Դ��nSamples������������
		UniformSampleAll, 
		//ֻ�����ȡһ����Դһ��sample
		UniformSampleOne 
	};

	class DirectLightingIntegrator : public SamplerIntegrator
	{
	public:

	};
}
