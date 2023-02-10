
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "rhi.h"
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "entity.h"
#include "material.h"
#include "light.h"
#include "window_sys.h"
#include <glm/glm.hpp>
#include "rhi_buffer.h"
#include <limits>
#include <set>


bool rhi_init(WindowSystem& win_sys, const RHI_InitConfig& cfg) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, cfg.hide_window ? GLFW_FALSE : GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(cfg.window_width, cfg.window_height, 
		cfg.title.c_str(), NULL, NULL);
	if (window == NULL)
	{
		printf("[render_init()] ERROR: Failed to create GLFW window\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("[render_init()] ERROR: Failed to initialize GLAD\n");
		return false;
	}
	win_sys.window = window;


	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	if (cfg.cull_face_enable) glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	if (cfg.polygon_linemode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glClearColor(cfg.background_color.x, cfg.background_color.y, cfg.background_color.z, 1.0f);
	glClearDepth(cfg.background_depth);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

void render_terminate(WindowSystem& win_sys) {
	if (win_sys.window)	glfwDestroyWindow(win_sys.window);
	glfwTerminate();
}

bool render_should_quit(WindowSystem& win_sys) {
	if (win_sys.window)	return glfwWindowShouldClose(win_sys.window);
	else return true;
}

template<typename ... Args>
std::string _rhi_h_string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

VAO_ptr mesh_to_VAO(MeshDataContainer_const_ptr mesh) {
	// prequest: mesh must be rearranged (which is done inside parse_obj_file())

	bool has_vertex = true;
	bool has_normal = (mesh->norms.size() != 0);
	bool has_texture = (mesh->texts.size() != 0);

	std::vector<VAO_Attr> attributes;
	if (has_vertex) {
		BufferObject verts_vbo(mesh->verts.data(), mesh->verts_num * 3 * sizeof(float), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		//attributes.emplace_back("vert", verts_vbo, 0, 3, GL_FLOAT, 0, false); // error, why???
		attributes.push_back(VAO_Attr("vert", verts_vbo, 0, 3, GL_FLOAT, 0, false));
	}
	if (has_normal) {
		BufferObject norms_vbo(mesh->norms.data(), mesh->verts_num * 3 * sizeof(float), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		attributes.push_back(VAO_Attr("norm", norms_vbo, 1, 3, GL_FLOAT, 0, false));
	}
	if (has_texture) {
		BufferObject texts_vbo(mesh->texts.data(), mesh->verts_num * 2 * sizeof(float), GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		attributes.push_back(VAO_Attr("text", texts_vbo, 2, 2, GL_FLOAT, 0, false));
	}

	BufferObject ebo(mesh->faces_vertID.data(), mesh->face_num * 3 * sizeof(uint32_t), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);

	VAO_ptr vao_ptr = std::make_shared<VAO>();
	vao_ptr->add_ebo(ebo);
	vao_ptr->add_attributes(attributes);

	return vao_ptr;
}

bool bound_instance_to_VAO(VAO_ptr vao, Entity_ptr entity) {
	if (!vao) return false;
	if (!entity) return false;

	std::vector<VAO_Attr> attributes;
	
	if (entity->instance_data.offset.size() != 0) {
		BufferObject instance_offset_vbo(
			entity->instance_data.offset.data(), 
			entity->instance_data.instance_num * 3 * sizeof(float), 
			GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		attributes.push_back(VAO_Attr("offset", instance_offset_vbo, 3, 3, GL_FLOAT, 0, true));
	}
	
	if (entity->instance_data.color.size() != 0) {
		BufferObject instance_color_vbo(
			entity->instance_data.color.data(),
			entity->instance_data.instance_num * 3 * sizeof(float),
			GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		attributes.push_back(VAO_Attr("color", instance_color_vbo, 4, 3, GL_FLOAT, 0, true));
	}

	if (entity->instance_data.scale.size() != 0) {
		BufferObject instance_scale_vbo(
			entity->instance_data.scale.data(),
			entity->instance_data.instance_num * sizeof(float),
			GL_STATIC_DRAW, GL_ARRAY_BUFFER);
		attributes.push_back(VAO_Attr("scale", instance_scale_vbo, 5, 1, GL_FLOAT, 0, true));
	}

	vao->add_attributes(attributes);
	return true;
}


void clear_render_target(glm::vec4 clear_color) {
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void bound_material_to_shader(Shader_ptr shader, RenderMaterial_ptr material) {
	shader->use_shader();
	auto& int_data = material->material_config.int_data;
	auto& float_data = material->material_config.float_data;
	auto& bool_data = material->material_config.bool_data;
	auto& vec2_data = material->material_config.vec2_data;
	auto& vec3_data = material->material_config.vec3_data;
	auto& vec4_data = material->material_config.vec4_data;
	auto& other_data = material->material_config.other_data;

	for (auto it = int_data.begin(); it != int_data.end(); it++)
		shader->setInt(it->first, it->second);
	for (auto it = bool_data.begin(); it != bool_data.end(); it++)
		shader->setBool(it->first, it->second);
	for (auto it = float_data.begin(); it != float_data.end(); it++)
		shader->setFloat(it->first, it->second);
	for (auto it = vec2_data.begin(); it != vec2_data.end(); it++)
		shader->setVec2(it->first, it->second);
	for (auto it = vec3_data.begin(); it != vec3_data.end(); it++)
		shader->setVec3(it->first, it->second);
	for (auto it = vec4_data.begin(); it != vec4_data.end(); it++)
		shader->setVec4(it->first, it->second);
}

void bound_vao_to_shader(Shader_ptr shader, VAO_ptr vao) {
	shader->use_shader();
	vao->use();
}

void _bound_camera_to_shader(Shader_ptr current_shader, Camera_ptr camera) {
	glm::mat4 projection = camera->get_proj_mat((float)window_global.width, (float)window_global.height);
	glm::mat4 view = camera->get_view_mat();
	current_shader->setMat4("projection", projection);
	current_shader->setMat4("view", view);
	current_shader->setVec3("viewPos", camera->transform.translate);
}

void _bound_transform_to_shader(Shader_ptr current_shader, Entity_ptr entity) {
	glm::mat4 model;
	model = entity->transform.get_model_mat();
	current_shader->setMat4("model", model);
}

void bound_mvp_to_shader(Shader_ptr shader, Camera_ptr camera, Entity_ptr entity) {
	shader->use_shader();
	_bound_camera_to_shader(shader, camera);
	_bound_transform_to_shader(shader, entity);
}

void bound_lights_to_shader(Shader_ptr shader, std::vector<PointLight_ptr>& point_lights,
	std::vector<DirectLight_ptr>& direct_lights) {
	shader->use_shader();
	shader->setInt("num_pointLights", point_lights.size());
	shader->setInt("num_directLights", direct_lights.size());

	int pt_idx = 0;
	for (auto& pt_light : point_lights) {
		shader->setVec3(_rhi_h_string_format<int>("pointLights[%d].position", pt_idx), pt_light->transform.translate);
		shader->setVec3(_rhi_h_string_format<int>("pointLights[%d].ambient", pt_idx), pt_light->light_common_attr.ambient);
		shader->setVec3(_rhi_h_string_format<int>("pointLights[%d].diffuse", pt_idx), pt_light->light_common_attr.diffuse);
		shader->setVec3(_rhi_h_string_format<int>("pointLights[%d].specular", pt_idx), pt_light->light_common_attr.specular);
		shader->setFloat(_rhi_h_string_format<int>("pointLights[%d].constant", pt_idx), pt_light->constant);
		shader->setFloat(_rhi_h_string_format<int>("pointLights[%d].linear", pt_idx), pt_light->linear);
		shader->setFloat(_rhi_h_string_format<int>("pointLights[%d].quadratic", pt_idx), pt_light->quadratic);
		pt_idx++;
	}
	int d_idx = 0;
	for (auto& dir_light : direct_lights) {
		shader->setVec3(_rhi_h_string_format<int>("directLights[%d].direction", d_idx), dir_light->get_direction());
		shader->setVec3(_rhi_h_string_format<int>("directLights[%d].ambient", d_idx), dir_light->light_common_attr.ambient);
		shader->setVec3(_rhi_h_string_format<int>("directLights[%d].diffuse", d_idx), dir_light->light_common_attr.diffuse);
		shader->setVec3(_rhi_h_string_format<int>("directLights[%d].specular", d_idx), dir_light->light_common_attr.specular);
		d_idx++;
	}
}


void drawcall_mesh(Shader_ptr shader, MeshDataContainer_ptr mesh, Entity_ptr entity) {
	shader->use_shader();
	
	if (entity->wireframe) {
		glLineWidth(entity->linewidth);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	{
		if (entity->instance_data.used()) {
			glDrawElementsInstanced(GL_TRIANGLES, mesh->face_num * 3, GL_UNSIGNED_INT, 0, entity->instance_data.instance_num);
		}
		else {
			glDrawElements(GL_TRIANGLES, mesh->face_num * 3, GL_UNSIGNED_INT, 0);
		}
	}
	if (entity->wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

	glBindVertexArray(0);
}


void update_entity_if_dirty(Entity_ptr entity, MeshDataContainer_ptr mesh, VAO_ptr vao) {
	if (entity->instance_data.used()) {
		if (entity->instance_data.offset_dirty)
			vao->update_buffer("offset", entity->instance_data.offset.data());
		if (entity->instance_data.color_dirty)
			vao->update_buffer("color", entity->instance_data.color.data());
		if (entity->instance_data.scale_dirty)
			vao->update_buffer("scale", entity->instance_data.color.data());
	}
	if (mesh->verts_dirty) 
		vao->update_buffer("vert", mesh->verts.data());
	if (mesh->norms_dirty)
		vao->update_buffer("norm", mesh->norms.data());
	if (mesh->texts_dirty)
		vao->update_buffer("text", mesh->texts.data());
	if (mesh->face_dirty)
		vao->update_buffer("vert", mesh->verts.data());

	if (entity->instance_data.used()) {
		entity->instance_data.offset_dirty = false;
		entity->instance_data.color_dirty= false;
		entity->instance_data.scale_dirty = false;
	}
	mesh->verts_dirty = mesh->norms_dirty = mesh->texts_dirty = mesh->face_dirty = false;
}

