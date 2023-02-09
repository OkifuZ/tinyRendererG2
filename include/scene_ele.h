#pragma once
#include "snowflake.h"

#include <memory>
#include <vector>
#include <string>

class SceneData;
typedef std::shared_ptr<SceneData> SceneData_ptr;

class SceneData {
public:
	std::string name = "";
	std::vector<Snowflake_type> entities;
	std::vector<Snowflake_type> point_lights;
	std::vector<Snowflake_type> direct_lights;
	Snowflake_type camera;
};

bool load_scene(const std::string& fname);