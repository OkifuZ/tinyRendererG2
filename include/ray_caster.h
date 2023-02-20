#pragma once

#include "entity.h"
#include "window_sys.h"
#include "camera.h"
#include "resource_manager.h"

#include <glm/glm.hpp>

#include <memory>

class RayCaster;
struct Ray;

using RayCaster_sptr = std::shared_ptr<RayCaster>;
using RayCaster_uptr = std::unique_ptr<RayCaster>;
using RayCaster_rptr = RayCaster*;

using Ray_uptr = std::unique_ptr<Ray>;
using Ray_rptr = Ray*;


struct Intersect {
	Entity_ptr entity = nullptr;
	float distance = 0;
};

struct Ray {
	glm::vec3 ori{};
	glm::vec3 direction{};
	Intersect intersects;

	Ray() = default;
	Ray(const glm::vec3& ori, const glm::vec3& direction) : ori(ori), direction(direction) {}
};

static bool is_intersect(const Entity_const_ptr& entity, const Ray_rptr ray) {
	if (!entity) return false;
	if (ray->direction == glm::zero<glm::vec3>()) return false;

	static auto check

	AABB_data aabb = entity->get_AABB_no_transform();
	float p_x_min = aabb.min_pos.x;
	float p_y_min = aabb.min_pos.y;
	float p_z_min = aabb.min_pos.z;
	float p_x_max = aabb.max_pos.x;
	float p_y_max = aabb.max_pos.y;
	float p_z_max = aabb.max_pos.z;
	float o_x = ray->ori.x;
	float o_y = ray->ori.y;
	float o_z = ray->ori.z;
	float d_x = ray->direction.x;
	float d_y = ray->direction.y;
	float d_z = ray->direction.z;

	if ()

	float t_x_min = (p_x_min - o_x) / d_x;
	float t_y_min = (p_y_min - o_y) / d_y;
	float t_z_min = (p_z_min - o_z) / d_z;
	float t_x_max = (p_x_max - o_x) / d_x;
	float t_y_max = (p_y_max - o_y) / d_y;
	float t_z_max = (p_z_max - o_z) / d_z;
	
	float t_min = std::max(t_x_min, t_y_min, t_z_min);
	float t_max = std::min(t_x_max, t_y_max, t_z_max);

	if (t_min < t_max && t_max > 0) return true;
}

class RayCaster {



public:
	Ray_uptr ray = nullptr;

	void cast_ray_from_camera(Camera_ptr camera, float x, float y) {

		glm::vec4 v{
			2.0f *x / (float)camera->current_window->width - 1.0f,
			1.0f - 2.0f * y / (float)camera->current_window->height,
			1.0f,
			0.05f
		};

		glm::mat4 v_inv_p_inv = glm::inverse(camera->get_proj_mat() * camera->get_view_mat());
		v = v_inv_p_inv * v;
		v /= v.w; // v now stands for world coordinates

		glm::vec3 origin = camera->transform.translate;
		glm::vec3 direction = glm::normalize(glm::vec3(v) - origin);

		this->ray = std::move(std::make_unique<Ray>(origin, direction));

		// intersect
		auto& entities = resource_manager_global.get_all_entities();
		for (auto& entity : entities) {
			AABB_data aabb = entity->get_AABB_no_transform();

		}
	};

private:

	


};