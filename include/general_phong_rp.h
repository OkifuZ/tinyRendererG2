#pragma once
#include "renderer.h"
#include "resource_manager.h"
#include "snowflake.h"
#include "scene.h"
#include "rhi.h"

#include <vector>

class GeneralPhongRenderPipeline : public RenderPipeline {

	typedef std::tuple<Entity_ptr, MeshDataContainer_ptr, VAO_ptr> entity_tup_type;

	Camera_ptr camera = nullptr;
	std::vector<entity_tup_type> entity_tups;
	//std::vector<Entity_ptr> entities;
	std::vector<PointLight_ptr> point_lights;
	std::vector<DirectLight_ptr> direct_lights;

	bool prepare(Snowflake_type scene_uuid) override {
		SceneData_ptr scene = resource_manager_global.get_scene_by_uuid(scene_uuid);
		if (!scene) return false;
		auto& entities_uuid = scene->entities;
		for (Snowflake_type entity_uuid : entities_uuid) {
			Entity_ptr entity = resource_manager_global.get_entity_by_uuid(entity_uuid);
			if (!entity) continue;

			Snowflake_type mesh_uuid = entity->mesh_uuid;
			MeshDataContainer_ptr mesh = resource_manager_global.get_mesh_by_uuid(mesh_uuid);
			if (!mesh || !mesh->loaded()) continue;
			if (mesh->norms.size() != mesh->verts_num * 3) continue;
			
			VAO_ptr vao = mesh_to_VAO(mesh);
			if (!bound_instance_to_VAO(vao, entity)) continue;

			Snowflake_type material_uuid = entity->material_uuid;
			RenderMaterial_ptr material = resource_manager_global.get_material_by_uuid(material_uuid);
			if (!material || !material->loaded) continue;

			Shader_ptr shader = resource_manager_global.get_shader_by_name(material->shader_name);
			if (!shader) continue;
			compile_shader(shader);

			entity_tups.push_back({ entity, mesh, vao });
		}
		if (entity_tups.empty()) return false;

		auto& ptls_uuid = scene->point_lights;
		for (Snowflake_type pt_uuid : ptls_uuid) {
			auto& ptl = resource_manager_global.get_pointLight_by_uuid(pt_uuid);
			if (!ptl) continue;
			point_lights.push_back(ptl);
		}
		auto& dls_uuid = scene->direct_lights;
		for (Snowflake_type d_uuid : dls_uuid) {
			auto& dl = resource_manager_global.get_directLight_by_uuid(d_uuid);
			if (!dl) continue;
			direct_lights.push_back(dl);
		}

		camera = resource_manager_global.get_camera_by_uuid(scene->camera);

		return true;
	}

	void render(Snowflake_type scene_uuid) override {

		// check if any buffer needs to be updated(dirty)
		for (auto& entity_tup : entity_tups) {
			auto& [entity, mesh, vao] = entity_tup;
			update_entity_if_dirty(entity, mesh, vao);
		}

		clear_render_target({0.2f, 0.2f, 0.2f, 1.0f});

		for (auto& entity_tup : entity_tups) {
			auto& [entity, mesh, vao] = entity_tup;

			RenderMaterial_ptr material = resource_manager_global.get_material_by_uuid(entity->material_uuid);
			if (!material) continue;
			Shader_ptr shader = resource_manager_global.get_shader_by_name(material->shader_name);
			if (!shader) continue;

			bound_lights_to_shader(shader, point_lights, direct_lights);
			bound_vao_to_shader(shader, vao);
			bound_material_to_shader(shader, material);
			bound_mvp_to_shader(shader, camera, entity);

			if (entity->wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			drawcall_mesh(shader, mesh, entity);
		}
		
	}

};