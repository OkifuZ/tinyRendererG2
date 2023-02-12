#pragma once

#include "mesh.h"
#include "material.h"
#include "shader.h"
#include "entity.h"
#include "scene_ele.h"
#include "light.h"
#include "camera.h"
#include "snowflake.h"
#include "window_sys.h"
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <functional>


class ResourceManager {
	Snowflake<snowflake_nonlock> snow_generator;

	std::map<Snowflake_type, MeshDataContainer_ptr> mesh_elements;
	std::map<Snowflake_type, RenderMaterial_ptr>	material_elements;
	std::map<Snowflake_type, Shader_ptr>			shader_elements;
	std::map<Snowflake_type, Entity_ptr>			entity_elements;
	std::map<Snowflake_type, SceneData_ptr>			scene_elements;
	std::map<Snowflake_type, PointLight_ptr>		pointLight_elements;
	std::map<Snowflake_type, DirectLight_ptr>		directLight_elements;
	std::map<Snowflake_type, Camera_ptr>		    camera_elements;
	typedef std::map<Snowflake_type, MeshDataContainer_ptr> mesh_map_type;
	typedef std::map<Snowflake_type, RenderMaterial_ptr>    material_map_type;
	typedef std::map<Snowflake_type, Shader_ptr>            shader_map_type;
	typedef std::map<Snowflake_type, Entity_ptr>			entity_map_type;
	typedef std::map<Snowflake_type, SceneData_ptr>			scene_map_type;
	typedef std::map<Snowflake_type, PointLight_ptr>		pointLight_map_type;
	typedef std::map<Snowflake_type, DirectLight_ptr>		directLight_map_type;
	typedef std::map<Snowflake_type, Camera_ptr>			camera_map_type;

	std::map<std::string, Snowflake_type> shader_name_uuid_map;
	typedef std::map<std::string, Snowflake_type> shader_name_uuid_map_type;

public:

	static const std::filesystem::path material_path;
	static const std::filesystem::path shader_path;
	static const std::filesystem::path mesh_path;
	static const std::filesystem::path scene_path;

	MeshDataContainer_ptr get_mesh_by_uuid(Snowflake_type uuid);

	Shader_ptr get_shader_by_uuid(Snowflake_type uuid);

	RenderMaterial_ptr get_material_by_uuid(Snowflake_type uuid);

	Entity_ptr get_entity_by_uuid(Snowflake_type uuid);

	Entity_ptr get_entity_by_name(const std::string& shader_name);

	SceneData_ptr get_scene_by_uuid(Snowflake_type uuid);
	
	PointLight_ptr get_pointLight_by_uuid(Snowflake_type uuid);

	DirectLight_ptr get_directLight_by_uuid(Snowflake_type uuid);

	Camera_ptr get_camera_by_uuid(Snowflake_type uuid);

	Camera_ptr get_camera_by_name(const std::string& name);

	Snowflake_type add_mesh(MeshDataContainer_ptr mesh);

	Snowflake_type add_shader(Shader_ptr shader, const std::string& name);

	Snowflake_type add_material(RenderMaterial_ptr mesh);

	Snowflake_type add_entity(Entity_ptr entity);

	Snowflake_type add_scene(SceneData_ptr scene);

	Snowflake_type add_pointLight(PointLight_ptr ptl);

	Snowflake_type add_directLight(DirectLight_ptr dirl);

	Snowflake_type add_camera(Camera_ptr camera);

	Snowflake_type get_shader_uuid_by_name(const std::string& shader_name);

	Shader_ptr get_shader_by_name(const std::string& shader_name);

	std::vector<Entity_ptr> filter_entities(std::function<bool(Entity_const_ptr)> filter_foo);


};

extern ResourceManager resource_manager_global;

void init_resource_manager();

Snowflake_type load_scene(const std::filesystem::path& fpath);

bool write_scene(SceneData_ptr scene, const std::string& fname);

