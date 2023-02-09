#pragma once

#include "shader.h"
#include "snowflake.h"
#include "json11.hpp"
#include "file_system.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <map>

class RenderMaterial;
typedef std::shared_ptr<RenderMaterial> RenderMaterial_ptr;
void load_material(RenderMaterial& material, const std::string& material_path);


class MaterialConfig {
public:
	std::map<std::string, bool>        bool_data;  // bool
	std::map<std::string, int>         int_data;   // int, texture2D
	std::map<std::string, float>	   float_data; // float
	std::map<std::string, glm::vec2>   vec2_data;
	std::map<std::string, glm::vec3>   vec3_data;
	std::map<std::string, glm::vec4>   vec4_data;
	std::map<std::string, std::string> other_data;
};

class RenderMaterial {
public:
	std::string shader_name;

	bool loaded = false;
	typedef json11::Json MaterialConfig_json_type;
	MaterialConfig_json_type material_config_json;
	MaterialConfig material_config;
};



