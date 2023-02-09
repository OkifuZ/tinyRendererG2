#include "resource_manager.h"

ResourceManager resource_manager_global;

MeshDataContainer_ptr ResourceManager::get_mesh_by_uuid(Snowflake_type uuid) {
	auto it = mesh_elements.find(uuid);
	if (it != mesh_elements.end())
	{
		return it->second;
	}
	return MeshDataContainer_ptr();
}

Shader_ptr ResourceManager::get_shader_by_uuid(Snowflake_type uuid) {
	auto it = shader_elements.find(uuid);
	if (it != shader_elements.end()) {
		return it->second;
	}
	return Shader_ptr();
}

Snowflake_type ResourceManager::get_shader_uuid_by_name(const std::string& shader_name) {
	auto it = shader_name_uuid_map.find(shader_name);
	if (it != shader_name_uuid_map.end()) {
		return it->second;
	}
	return SNOWFLAKE_INVALID;
}

Shader_ptr ResourceManager::get_shader_by_name(const std::string& shader_name) {
	return resource_manager_global.get_shader_by_uuid(
		resource_manager_global.get_shader_uuid_by_name(shader_name));
}

RenderMaterial_ptr ResourceManager::get_material_by_uuid(Snowflake_type uuid) {
	auto it = material_elements.find(uuid);
	if (it != material_elements.end()) {
		return it->second;
	}
	return RenderMaterial_ptr();
}

Entity_ptr ResourceManager::get_entity_by_uuid(Snowflake_type uuid) {
	auto it = entity_elements.find(uuid);
	if (it != entity_elements.end()) {
		return it->second;
	}
	return Entity_ptr();
}

SceneData_ptr ResourceManager::get_scene_by_uuid(Snowflake_type uuid) {
	auto it = scene_elements.find(uuid);
	if (it != scene_elements.end()) {
		return it->second;
	}
	return SceneData_ptr();
}

PointLight_ptr ResourceManager::get_pointLight_by_uuid(Snowflake_type uuid) {
	auto it = pointLight_elements.find(uuid);
	if (it != pointLight_elements.end()) {
		return it->second;
	}
	return PointLight_ptr();
}

DirectLight_ptr ResourceManager::get_directLight_by_uuid(Snowflake_type uuid) {
	auto it = directLight_elements.find(uuid);
	if (it != directLight_elements.end()) {
		return it->second;
	}
	return DirectLight_ptr();
}

Camera_ptr ResourceManager::get_camera_by_uuid(Snowflake_type uuid) {
	auto it = camera_elements.find(uuid);
	if (it != camera_elements.end()) {
		return it->second;
	}
	return Camera_ptr();
}

Snowflake_type ResourceManager::add_mesh(MeshDataContainer_ptr mesh) {
	if (!mesh) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	mesh_elements.insert(mesh_map_type::value_type(uuid, mesh));
	return uuid;
}

Snowflake_type ResourceManager::add_shader(Shader_ptr shader, const std::string& name) {
	if (!shader) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	shader_elements.insert(shader_map_type::value_type(uuid, shader));
	shader_name_uuid_map.insert(shader_name_uuid_map_type::value_type(name, uuid));
	return uuid;
}

Snowflake_type ResourceManager::add_material(RenderMaterial_ptr mesh) {
	if (!mesh) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	material_elements.insert(material_map_type::value_type(uuid, mesh));
	return uuid;
}

Snowflake_type ResourceManager::add_entity(Entity_ptr entity) {
	if (!entity) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	entity_elements.insert(entity_map_type::value_type(uuid, entity));
	return uuid;
}

Snowflake_type ResourceManager::add_scene(SceneData_ptr scene) {
	if (!scene) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	scene_elements.insert(scene_map_type::value_type(uuid, scene));
	return uuid;
}

Snowflake_type ResourceManager::add_pointLight(PointLight_ptr ptl) {
	if (!ptl) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	pointLight_elements.insert(pointLight_map_type::value_type(uuid, ptl));
	return uuid;
}

Snowflake_type ResourceManager::add_directLight(DirectLight_ptr dirl) {
	if (!dirl) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	directLight_elements.insert(directLight_map_type::value_type(uuid, dirl));
	return uuid;
}

Snowflake_type ResourceManager::add_camera(Camera_ptr cam) {
	if (!cam) return SNOWFLAKE_INVALID;
	Snowflake_type uuid = snow_generator.nextid();
	camera_elements.insert(camera_map_type::value_type(uuid, cam));
	return uuid;
}

// TODO! finish me
#include "json11.hpp"

typedef json11::Json::object map_type;
typedef json11::Json::array  arr_type;
typedef json11::Json         json_type;

bool _load_entity(Entity_ptr entity, const json_type& scene_json) {
	if (scene_json != json_type::OBJECT) return false;
	return false;
}

bool _load_point_light(PointLight_ptr point_light, const json_type& ptl_json) {
	return false;

}

bool _load_direct_light (DirectLight_ptr direct_light, const json_type& dl_json) {
	return false;

}

bool _load_camera(Camera_ptr camera, const json_type& cam_json) {
	return false;

}

bool _load_scene(SceneData_ptr scene, const json_type& scene_json) {

	map_type scene_map = scene_json.object_items();
	if (scene_map.empty()) return false;

	for (map_type::iterator it = scene_map.begin(); it != scene_map.end(); it++) {
		auto& descriptor = it->first;
		auto& value = it->second;
		json_type::Type val_type = value.type();

		if (descriptor == "name") {
			if (val_type != json_type::STRING) return false;
			scene->name = value.string_value();
		}
		else if (descriptor == "entities") {
			if (val_type != json_type::ARRAY) return false;

			auto& entities_arr = value.array_items();
			for (auto& entity_json : entities_arr) {
				if (!entity_json.type() == json_type::OBJECT) continue;
				Entity_ptr entity = std::make_shared<Entity>();
				if (!_load_entity(entity, entity_json)) continue;
				
				Snowflake_type uuid = resource_manager_global.add_entity(entity);
				scene->entities.push_back(uuid);
			}
		}
		else if (descriptor == "point_lights") {
			if (val_type != json_type::ARRAY) return false;

			auto& ptl_arr = value.array_items();
			for (auto& ptl_json : ptl_arr) {
				if (ptl_json.type() != json_type::OBJECT) continue;
				PointLight_ptr ptl = std::make_shared<PointLight>();
				if (!_load_point_light(ptl, ptl_json)) continue;

				Snowflake_type uuid = resource_manager_global.add_pointLight(ptl);
				scene->point_lights.push_back(uuid);
			}
		}
		else if (descriptor == "direct_lights") {
			if (!val_type == json_type::ARRAY) return false;

			auto& dir_arr = value.array_items();
			for (auto& dir_json : dir_arr) {
				if (dir_json.type() != json_type::OBJECT) continue;
				DirectLight_ptr dl = std::make_shared<DirectLight>();
				if (!_load_direct_light(dl, dir_json)) continue;

				Snowflake_type uuid = resource_manager_global.add_directLight(dl);
				scene->direct_lights.push_back(uuid);
			}
		}
		else if (descriptor == "camera") {

			Camera_ptr camera = std::make_shared<Camera>();
			if (!_load_camera(camera, value)) return false;
			
			Snowflake_type uuid = resource_manager_global.add_camera(camera);
			scene->camera = uuid;
		}
		else {
			// just ignore it
		}
	}

	return true;
}

bool load_scene(SceneData_ptr scene, const std::string& fname) {
	if (!scene) return false;

	std::string scene_json_str = read_file_as_str(fname);
	std::string err;
	json11::Json scene_json = json11::Json::parse(scene_json_str, err, json11::JsonParse::COMMENTS);
	if (err != "") {
		printf("[load_scene()] error occurred!\n");
		exit(-1);
	}

	return _load_scene(scene, scene_json);
}

bool write_scene(SceneData_ptr scene, const std::string& fname) {
	return false;
}