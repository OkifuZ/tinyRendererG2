#include "material.h"
#include "resource_manager.h"

// TODO: material disk->cpu(just read, json->cfg class), cpu->gpu(set shader)

void load_material(RenderMaterial& material, const std::string& material_path) {

	std::string matcfg_string = read_file_as_str(material_path);
	std::string err;
	material.material_config_json = json11::Json::parse(matcfg_string, err, json11::JsonParse::COMMENTS);
	if (err != "") {
		printf("[load_material()] error occurred!\n");
		exit(-1);
	}

	typedef RenderMaterial::MaterialConfig_json_type::object cfg_map_type;
	typedef RenderMaterial::MaterialConfig_json_type::array  cfg_arr_type;
	typedef RenderMaterial::MaterialConfig_json_type         cfg_json_type;

	cfg_map_type cfg_map = material.material_config_json.object_items();
	std::string prefix = "";
	for (cfg_map_type::iterator it = cfg_map.begin(); it != cfg_map.end(); it++) {
		auto& descriptor = it->first;
		auto& value = it->second;		
		cfg_json_type::Type val_type = value.type();

		if (descriptor == "shader") {
			material.shader_name = value.string_value();
			continue;
		}
		else if (descriptor == "prefix") {
			prefix = value.string_value() + ".";
		}
	}
	for (cfg_map_type::iterator it = cfg_map.begin(); it != cfg_map.end(); it++) {
		auto descriptor = it->first;
		descriptor = prefix + descriptor;
		auto& value = it->second;

		if (descriptor == "shader" || descriptor == "prefix") {
			continue;
		}

		cfg_json_type::Type val_type = value.type();
		if (val_type != cfg_json_type::OBJECT) {
			printf("[load_material()] error occurred!\n");
			exit(-1);
		}
		else {
			cfg_map_type data_map = value.object_items();
			std::string type_str = ""; 
			if (data_map.find("type") != data_map.end())
				type_str = data_map["type"].string_value();
			else continue;
			if (type_str == "bool") {
				bool val_data = false;
				if (data_map.find("data") != data_map.end())
					val_data = data_map["data"].bool_value();
				else continue;
				material.material_config.bool_data.insert(
					std::map<std::string, bool>::value_type(descriptor, val_data)
				);
			} 
			else if (type_str == "float") {
				float val_data = 0;
				if (data_map.find("data") != data_map.end())
					val_data = static_cast<float>(data_map["data"].number_value());
				else continue;
				material.material_config.float_data.insert(
					std::map<std::string, float>::value_type(descriptor, val_data)
				);
			}
			else if (type_str == "int") {
				int val_data = 0;
				if (data_map.find("data") != data_map.end())
					val_data = static_cast<int>(data_map["data"].number_value());
				else continue;
				material.material_config.int_data.insert(
					std::map<std::string, int>::value_type(descriptor, val_data)
				);
			}
			else if (type_str == "vec2") {
				std::vector<cfg_json_type> arr;
				if (data_map.find("data") != data_map.end())
					arr = data_map["data"].array_items();
				else continue;
				glm::vec2 val_vec2;
				val_vec2.x = static_cast<float>(arr[0].number_value());
				val_vec2.y = static_cast<float>(arr[1].number_value());
				material.material_config.vec2_data.insert(
					std::map<std::string, glm::vec2>::value_type(descriptor, val_vec2)
				);
			}
			else if (type_str == "vec3") {
				std::vector<cfg_json_type> arr;
				if (data_map.find("data") != data_map.end())
					arr = data_map["data"].array_items();
				else continue;
				glm::vec3 val_vec3;
				val_vec3.x = static_cast<float>(arr[0].number_value());
				val_vec3.y = static_cast<float>(arr[1].number_value());
				val_vec3.z = static_cast<float>(arr[2].number_value());
				material.material_config.vec3_data.insert(
					std::map<std::string, glm::vec3>::value_type(descriptor, val_vec3)
				);
			}
			else if (type_str == "vec4") {
				std::vector<cfg_json_type> arr;
				if (data_map.find("data") != data_map.end())
					arr = data_map["data"].array_items();
				else continue;
				glm::vec4 val_vec4;
				val_vec4.x = static_cast<float>(arr[0].number_value());
				val_vec4.y = static_cast<float>(arr[1].number_value());
				val_vec4.z = static_cast<float>(arr[2].number_value());
				val_vec4.w = static_cast<float>(arr[3].number_value());
				material.material_config.vec4_data.insert(
					std::map<std::string, glm::vec4>::value_type(descriptor, val_vec4)
				);
			}
			else if (type_str == "texture") {
				// TODO
			}
			else {
				printf("[load_material()] error occured\n");
				exit(-1);
			}
		}
	}
	material.loaded = true;
	printf("load_material() %s completed\n", material_path.c_str());
}
