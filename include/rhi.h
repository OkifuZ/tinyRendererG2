#pragma once

#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "entity.h"
#include "material.h"
#include "light.h"
#include "window_sys.h"
#include <glm/glm.hpp>

struct RHI_InitConfig {
	bool hide_window = false;
	int window_width = 1200;
	int window_height = 900;
	std::string title = "";
	bool cull_face_enable = true;
	bool polygon_linemode = false;
	glm::vec3 background_color{ 0.6f, 0.5f, 0.6f };
	float background_depth = 1.0f;

};

bool rhi_init(WindowSystem& win_sys, const RHI_InitConfig&);

void render_terminate(WindowSystem& win_sys);

bool render_should_quit(WindowSystem& win_sys);

VAO_ptr mesh_to_VAO(MeshDataContainer_const_ptr mesh);

bool bound_instance_to_VAO(VAO_ptr vao, Entity_ptr entity);

void clear_render_target(glm::vec4 clear_color);

void bound_material_to_shader(Shader_ptr shader, RenderMaterial_ptr material);

void bound_vao_to_shader(Shader_ptr shader, VAO_ptr vao);

void _bound_camera_to_shader(Shader_ptr current_shader, Camera_ptr camera);

void _bound_transform_to_shader(Shader_ptr current_shader, Entity_ptr entity);

void bound_mvp_to_shader(Shader_ptr shader, Camera_ptr camera, Entity_ptr entity);

void bound_lights_to_shader(Shader_ptr shader, std::vector<PointLight_ptr>& point_lights,
	std::vector<DirectLight_ptr>& direct_lights);

void drawcall_mesh(Shader_ptr shader, MeshDataContainer_ptr mesh, Entity_ptr entity);

void update_entity_if_dirty(Entity_ptr entity, MeshDataContainer_ptr mesh, VAO_ptr vao); // only instance for now