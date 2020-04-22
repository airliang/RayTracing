#pragma once
#include <string>
#include <vector>
#include <memory>

namespace AIR
{
	class Light;
	class RObject;

	class SceneParser
	{
	public:
		SceneParser(){}

		bool Load(const std::string& file, std::vector<std::shared_ptr<Light>>& lights,
			std::vector<std::shared_ptr<RObject>>& primitives);
	};
}
