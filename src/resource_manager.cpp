#include "resource_manager.h"
#include <map>

ResourceManager resource_manager_global;

const std::filesystem::path ResourceManager::material_path = "../resource/material";
const std::filesystem::path ResourceManager::shader_path = "../resource/shader";
const std::filesystem::path ResourceManager::mesh_path = "../resource/mesh";
const std::filesystem::path ResourceManager::scene_path = "../resource/scene";
const std::filesystem::path ResourceManager::simulation_path = "../resource/simulation";



void init_resource_manager() {
	// load all shaders
	struct ShaderChoice {
		bool vert = false;
		bool frag = false;
	};
	std::map<std::string, ShaderChoice> shader_name_map;
	for (const auto& entry : std::filesystem::directory_iterator(ResourceManager::shader_path)) {
		auto& stem = entry.path().stem();
		auto& extension = entry.path().extension();
		if (shader_name_map.find(stem.string()) != shader_name_map.end()) {
			shader_name_map[stem.string()];
		}
		if (extension.string() == ".vert")
			shader_name_map[stem.string()].vert = true;
		else if (extension.string() == ".frag")
			shader_name_map[stem.string()].frag = true;
	}

	for (const auto& it: shader_name_map) {
		auto& choice = it.second;
		if (choice.frag && choice.vert) {
			auto& name = it.first;
			Shader_ptr shader = std::make_shared<Shader>();
			load_shader(shader,
				(ResourceManager::shader_path / (name + ".vert")).string(),
				(ResourceManager::shader_path / (name + ".frag")).string());
			Snowflake_type shader_uuid = resource_manager_global.add_shader(shader, name);
		}
	}
}

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

Entity_ptr ResourceManager::get_entity_by_name(const std::string& name) {
	for (const auto& it : entity_elements) {
		if (it.second->name == name) {
			return it.second;
		}
	}
	return nullptr;
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

Camera_ptr ResourceManager::get_camera_by_name(const std::string& name) {
	for (const auto& it : camera_elements) {
		if (it.second->name == name) {
			return it.second;
		}
	}
	return nullptr;
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

std::vector<Entity_ptr> ResourceManager::filter_entities(std::function<bool(Entity_const_ptr)> filter_foo) {
	std::vector<Entity_ptr> res;
	for (auto& it : entity_elements) {
		if (filter_foo(it.second)) res.push_back(it.second);
	}
	return res;
}



#include "json11.hpp"

typedef json11::Json::object map_type;
typedef json11::Json::array  arr_type;
typedef json11::Json         json_type;

template<class T>
std::vector<T> _get_vector(const json_type& arr_json, int desire_length) {
	if (arr_json.type() != json_type::ARRAY) return std::vector<T>(desire_length, 0);
	auto& arr = arr_json.array_items();
	if (arr.size() != desire_length) return std::vector<T>(desire_length, 0);
	else {
		std::vector<T> res;
		for (auto& val : arr) {
			res.push_back(static_cast<T>(val.number_value()));
		}
		return res;
	}
}

bool _load_transform(Transform& t, const json_type& transform_json) {
	if (transform_json.type() != json_type::OBJECT) return false;
	map_type transform_map = transform_json.object_items();
	for (map_type::iterator it = transform_map.begin(); it != transform_map.end(); it++) {
		auto& descriptor = it->first;
		auto& value = it->second;
		if (descriptor == "quat") {
			auto& vec4 = _get_vector<float>(value, 4);
			t.rotationQ.w = vec4[0];
			t.rotationQ.x = vec4[1];
			t.rotationQ.y = vec4[2];
			t.rotationQ.z = vec4[3];
		}
		else if (descriptor == "scale") {
			auto& vec3 = _get_vector<float>(value, 3);
			t.scale.x = vec3[0];
			t.scale.y = vec3[1];
			t.scale.z = vec3[2];
		}
		else if (descriptor == "translate") {
			auto& vec3 = _get_vector<float>(value, 3);
			t.translate.x = vec3[0];
			t.translate.y = vec3[1];
			t.translate.z = vec3[2];
		}
	}
	return true;
}

std::string _rm_split_string(std::string& str, const std::string& delimiter) {
	if (str.find(delimiter) == std::string::npos) {
		std::string res = str;
		str = "";
		return res;
	}
	auto& res = str.substr(0, str.find(delimiter));
	str = str.substr(str.find(delimiter)+2, str.size());
	return res;
}

bool _load_instance_data(Entity_ptr entity, const json_type& ins_json) {
	if (!entity) return false;
	if (ins_json.type() != json_type::OBJECT) return false;
	map_type ins_map = ins_json.object_items();
	float scale = 1.0f;
	std::vector<float> color{};
	std::vector<float> offset_data;
	for (map_type::iterator it = ins_map.begin(); it != ins_map.end(); it++) {
		auto& desc = it->first;
		auto& val = it->second;
		if (desc == "offset") {
			std::string ins_data_str = val.string_value();
			auto ent_name = _rm_split_string(ins_data_str, "->");
			if (ins_data_str == "vert") {
				Entity_ptr target_ent = resource_manager_global.get_entity_by_name(ent_name);
				if (!target_ent) return false;
				MeshDataContainer_ptr target_mesh = resource_manager_global.get_mesh_by_uuid(target_ent->mesh_uuid);
				if (!target_mesh) return false;
				offset_data = target_mesh->verts;
				entity->instance_data.bound_entity = ent_name;
			}
			else return false;
		}
		else if (desc == "scale") {
			scale = static_cast<float>(val.number_value());
		}
		else if (desc == "color") {
			color = _get_vector<float>(val, 3);
		}
	}
	entity->instance_data.offset = std::move(offset_data);
	entity->instance_data.instance_num = entity->instance_data.offset.size() / 3;
	entity->instance_data.scale = std::vector<float>(entity->instance_data.instance_num, scale);
	for (size_t i = 0; i < entity->instance_data.instance_num; i++) {
		entity->instance_data.color.insert(entity->instance_data.color.end(), color.begin(), color.end());
	}
	return true;
}

bool _load_entity(SceneData_ptr scene, const json_type& entities_json) {
	if (entities_json.type() != json_type::ARRAY) return false;
	auto& entities_arr = entities_json.array_items();
	
	struct EntityItem {
		bool name = false;
		bool instance_data = false;
		bool mesh = false;
		bool material = false;
		bool transform = false;
		bool wireframe = false;
	} items;

	for (auto& entity_json : entities_arr) {
		if (!entity_json.type() == json_type::OBJECT) continue;
		Entity_ptr entity = std::make_shared<Entity>();

		map_type entity_map = entity_json.object_items();
		for (map_type::iterator it = entity_map.begin(); it != entity_map.end(); it++) {
			auto& descriptor = it->first;
			auto& value = it->second;
			if (descriptor == "name") {
				entity->name = value.string_value();
				items.name = true;
			}
			else if (descriptor == "instance_data") {
				if (value.type() == json_type::NUL) continue;
				if (!_load_instance_data(entity, value)) return false;
				items.instance_data = true;
			}
			else if (descriptor == "material") {
				RenderMaterial_ptr mat = std::make_shared<RenderMaterial>();
				load_material(*mat, (ResourceManager::material_path / value.string_value()).string());
				Snowflake_type mat_uuid = resource_manager_global.add_material(mat);
				entity->material_uuid = mat_uuid;
				items.material = true;
			}
			else if (descriptor == "mesh") {
				MeshDataContainer_ptr mesh = std::make_shared<MeshDataContainer>();
				load_mesh(mesh, (ResourceManager::mesh_path / value.string_value()).string());
				Snowflake_type mesh_uuid = resource_manager_global.add_mesh(mesh);
				entity->mesh_uuid = mesh_uuid;
				items.mesh = true;
			}
			else if (descriptor == "transform") {
				if (value.type() == json_type::NUL) continue;
				Transform t;
				if (!_load_transform(t, value)) return false;
				entity->transform = t;
				items.transform = true;
			}
			else if (descriptor == "wireframe") {
				if (value.type() == json_type::NUL) continue;
				entity->linewidth = static_cast<float>(value.number_value());
				entity->wireframe = true;
			}
			else if (descriptor == "cullface") {
				if (value.type() == json_type::NUL) continue;
				entity->cullface = value.bool_value();
			}
		}
		if (!items.instance_data && !items.transform) return false;
		if (!items.material || !items.mesh || !items.name) return false;

		Snowflake_type uuid = resource_manager_global.add_entity(entity);
		scene->entities.push_back(uuid);
	}

	return true;
}


bool _load_point_light(SceneData_ptr scene, const json_type& ptl_json) {
	if (!ptl_json.type() == json_type::ARRAY) return false;

	auto& ptl_arr = ptl_json.array_items();
	for (auto& ptl_json : ptl_arr) {
		if (ptl_json.type() != json_type::OBJECT) continue;
		PointLight_ptr ptl = std::make_shared<PointLight>();
		
		map_type ptl_map = ptl_json.object_items();
		for (map_type::iterator it = ptl_map.begin(); it != ptl_map.end(); it++) {
			auto& descriptor = it->first;
			auto& value = it->second;
			if (descriptor == "name") {
				ptl->name = value.string_value();
			}
			else if (descriptor == "light_common_attr") {
				if (value.type() != json_type::OBJECT) continue;
				map_type lca_map = value.object_items();
				for (map_type::iterator it = lca_map.begin(); it != lca_map.end(); it++) {
					auto& desc = it->first;
					auto& val = it->second;
					if (desc == "ambient") {
						auto& vec3 = _get_vector<float>(val, 3);
						ptl->light_common_attr.ambient.x = vec3[0];
						ptl->light_common_attr.ambient.y = vec3[1];
						ptl->light_common_attr.ambient.z = vec3[2];
					}
					else if (desc == "diffuse") {
						auto& vec3 = _get_vector<float>(val, 3);
						ptl->light_common_attr.diffuse.x = vec3[0];
						ptl->light_common_attr.diffuse.y = vec3[1];
						ptl->light_common_attr.diffuse.z = vec3[2];
					}
					else if (desc == "specular") {
						auto& vec3 = _get_vector<float>(val, 3);
						ptl->light_common_attr.specular.x = vec3[0];
						ptl->light_common_attr.specular.y = vec3[1];
						ptl->light_common_attr.specular.z = vec3[2];
					}
				}
			}
			else if (descriptor == "transform") {
				Transform t;
				if (!_load_transform(t, value)) return false;
				ptl->transform = t;
			}
			else if (descriptor == "constant") {
				ptl->constant = static_cast<float>(value.number_value());
			}
			else if (descriptor == "linear") {
				ptl->linear = static_cast<float>(value.number_value());
			}
			else if (descriptor == "quadratic") {
				ptl->quadratic = static_cast<float>(value.number_value());
			}
		}

		Snowflake_type uuid = resource_manager_global.add_pointLight(ptl);
		scene->point_lights.push_back(uuid);
	}
	return true;

}

bool _load_direct_light (SceneData_ptr scene, const json_type& dl_json) {
	if (!dl_json.type() == json_type::ARRAY) return false;

	auto& dl_arr = dl_json.array_items();
	for (auto& dl_json : dl_arr) {
		if (dl_json.type() != json_type::OBJECT) continue;
		DirectLight_ptr dl = std::make_shared<DirectLight>();

		map_type dl_map = dl_json.object_items();
		for (map_type::iterator it = dl_map.begin(); it != dl_map.end(); it++) {
			auto& descriptor = it->first;
			auto& value = it->second;
			if (descriptor == "name") {
				dl->name = value.string_value();
			}
			else if (descriptor == "light_common_attr") {
				if (value.type() != json_type::OBJECT) continue;
				map_type lca_map = value.object_items();
				for (map_type::iterator it = lca_map.begin(); it != lca_map.end(); it++) {
					auto& desc = it->first;
					auto& val = it->second;
					if (desc == "ambient") {
						auto& vec3 = _get_vector<float>(val, 3);
						dl->light_common_attr.ambient.x = vec3[0];
						dl->light_common_attr.ambient.y = vec3[1];
						dl->light_common_attr.ambient.z = vec3[2];
					}
					else if (desc == "diffuse") {
						auto& vec3 = _get_vector<float>(val, 3);
						dl->light_common_attr.diffuse.x = vec3[0];
						dl->light_common_attr.diffuse.y = vec3[1];
						dl->light_common_attr.diffuse.z = vec3[2];
					}
					else if (desc == "specular") {
						auto& vec3 = _get_vector<float>(val, 3);
						dl->light_common_attr.specular.x = vec3[0];
						dl->light_common_attr.specular.y = vec3[1];
						dl->light_common_attr.specular.z = vec3[2];
					}
				}
			}
			else if (descriptor == "transform") {
				if (value.type() != json_type::OBJECT) return false;
				map_type transform_map = value.object_items();
				glm::vec3 eye{}, target{}, up{};
				for (map_type::iterator it = transform_map.begin(); it != transform_map.end(); it++) {
					auto& des = it->first;
					auto& val = it->second;
					if (des == "eye") {
						auto& vec3 = _get_vector<float>(val, 3);
						eye.x = vec3[0];
						eye.y = vec3[1];
						eye.z = vec3[2];
					}
					else if (des == "target") {
						auto& vec3 = _get_vector<float>(val, 3);
						target.x = vec3[0];
						target.y = vec3[1];
						target.z = vec3[2];
					}
					else if (des == "up") {
						auto& vec3 = _get_vector<float>(val, 3);
						up.x = vec3[0];
						up.y = vec3[1];
						up.z = vec3[2];
					}
				}
				Transform t;
				t.look_at(eye, target, up);
				dl->transform = t;
			}
		}

		Snowflake_type uuid = resource_manager_global.add_directLight(dl);
		scene->direct_lights.push_back(uuid);
	}
	return true;

}

bool _load_camera(SceneData_ptr scene, const json_type& cam_json) {
	if (cam_json.type() != json_type::OBJECT) return false;

	Camera_ptr camera = std::make_shared<Camera>();

	map_type cam_map = cam_json.object_items();
	if (cam_map.empty()) return false;

	for (map_type::iterator it = cam_map.begin(); it != cam_map.end(); it++) {
		auto& descriptor = it->first;
		auto& value = it->second;
		if (descriptor == "name") {
			camera->name = value.string_value();
		}
		// else if (descriptor == "transform") {
		// 	Transform t;
		// 	if (!_load_transform(t, value)) return false;
		// 	camera->transform = t;
		// }
		else if (descriptor == "lookat") {
			if (value.type() != json_type::OBJECT) return false;
			map_type transform_map = value.object_items();
			glm::vec3 eye{}, target{}, up{};
			for (map_type::iterator it = transform_map.begin(); it != transform_map.end(); it++) {
				auto& des = it->first;
				auto& val = it->second;
				if (des == "eye") {
					auto& vec3 = _get_vector<float>(val, 3);
					eye.x = vec3[0];
					eye.y = vec3[1];
					eye.z = vec3[2];
				}
				else if (des == "target") {
					auto& vec3 = _get_vector<float>(val, 3);
					target.x = vec3[0];
					target.y = vec3[1];
					target.z = vec3[2];
				}
				else if (des == "up") {
					auto& vec3 = _get_vector<float>(val, 3);
					up.x = vec3[0];
					up.y = vec3[1];
					up.z = vec3[2];
				}
			}
			camera->cam_look_at(eye, target, up);
		}
		else if (descriptor == "near") {
			camera->near = static_cast<float>(value.number_value());
		}
		else if (descriptor == "far") {
			camera->far = static_cast<float>(value.number_value());
		}
		else if (descriptor == "fov_degree") {
			camera->fov_degree = static_cast<float>(value.number_value());
		}
	}

	Snowflake_type uuid = resource_manager_global.add_camera(camera);
	scene->camera = uuid;

	return true;
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
			if (!_load_entity(scene, value)) return false;
		}
		else if (descriptor == "point_lights") {
			if (!_load_point_light(scene, value)) return false;
		}
		else if (descriptor == "direct_lights") {
			if (!_load_direct_light(scene, value)) return false;
		}
		else if (descriptor == "camera") {
			if (!_load_camera(scene, value)) return false;
		}
		else if (descriptor == "background") {
			auto& vec3 = _get_vector<float>(value, 3);
			scene->background_color.x = vec3[0];
			scene->background_color.y = vec3[1];
			scene->background_color.z = vec3[2];
		}
		else {
			// just ignore it
		}
	}

	return true;
}

bool _load_scene(SceneData_ptr scene, const std::string& fname) {
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


Snowflake_type load_scene(const std::filesystem::path& fpath) {
	SceneData_ptr scene = std::make_shared<SceneData>();
	bool stat = _load_scene(scene, fpath.string());
	if (!stat) {
		printf("[load_scene()] error occured\n");
		exit(-1);
	}
	return resource_manager_global.add_scene(scene);
}

bool write_scene(SceneData_ptr scene, const std::string& fname) {
	return false;
}