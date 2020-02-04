#pragma once
#include "mathdef.h"

namespace AIR
{
	class GeometryParam
	{
	public:
		GeometryParam()
		{
			memset(&param, 0, sizeof(param));
		}
		~GeometryParam();

		union
		{
			struct
			{
				Float radius;
				Float phiMax;
				Float thetaMin;
				Float thetaMax;
			} sphere;
			//struct
			//{
			//	Float radius;
			//	Float yMin;
			//	Float yMax;
			//	Float phiMax;
			//} cylinder;
		}param;
	private:

	};

	
}