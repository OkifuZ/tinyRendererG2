#include "ray_caster.h"
#include "resource_manager.h"

void RayCaster::cast_ray_from_camera(Camera_ptr camera, float x, float y) {

	// screen space to world space
	glm::vec4 v{
		2.0f * x / (float)camera->current_window->width - 1.0f,
		1.0 - 2.0f * y / (float)camera->current_window->height,
		1.0f,
		1.0f
	};

	glm::mat4 v_inv_p_inv = glm::inverse(camera->get_proj_mat() * camera->get_view_mat());
	v = v_inv_p_inv * v;
	v /= v.w; // v now stands for world coordinates

	// cast ray
	glm::vec3 origin = camera->transform.translate;
	glm::vec3 direction = glm::normalize(glm::vec3(v) - origin);

	this->ray = std::move(std::make_unique<Ray>(origin, direction));

}

void RayCaster::intersect_ray_with_entities() {

	// intersect
	Triangle_raycaster tri_closest{};
	tri_closest.dis = std::numeric_limits<float>::max();
	Entity_ptr ent_closest = nullptr;

	auto& entities = resource_manager_global.get_all_entities();
	for (auto& entity : entities) {
		if (entity->instance_data.used()) continue; // not supported yet

		AABB_data aabb = entity->get_AABB_no_transform();
		if (!is_ray_intersect_with_aabb(aabb, this->ray.get())) continue;

		Triangle_raycaster tri_intersect;
		if (!is_ray_intersect_with_mesh(entity, this->ray.get(), tri_intersect)) continue;
		if (tri_closest.dis > tri_intersect.dis) {
			tri_closest = tri_intersect;
			ent_closest = entity;
		}
	}

	if (!ent_closest) return;

	this->ray->intersects.entity = ent_closest;
	this->ray->intersects.triangle = tri_closest;
}

bool is_ray_intersect_with_aabb(const AABB_data& aabb, const Ray_rptr ray) {
	if (ray->direction == glm::zero<glm::vec3>()) return false;

	static auto check_x = [](const AABB_data& aabb, const Ray_rptr ray) {
		float p_x_min = aabb.min_pos.x;
		float p_x_max = aabb.max_pos.x;
		float o_x = ray->ori.x;
		float d_x = ray->direction.x;
		float t_x_1 = (p_x_min - o_x) / d_x;
		float t_x_2 = (p_x_max - o_x) / d_x;
		return std::tuple{ std::min(t_x_1, t_x_2), std::max(t_x_1, t_x_2) };
	};

	static auto check_y = [](const AABB_data& aabb, const Ray_rptr ray) {
		float p_y_min = aabb.min_pos.y;
		float p_y_max = aabb.max_pos.y;
		float o_y = ray->ori.y;
		float d_y = ray->direction.y;
		float t_y_1 = (p_y_min - o_y) / d_y;
		float t_y_2 = (p_y_max - o_y) / d_y;
		return std::tuple{ std::min(t_y_1, t_y_2), std::max(t_y_1, t_y_2) };
	};

	static auto check_z = [](const AABB_data& aabb, const Ray_rptr ray) {
		float p_z_min = aabb.min_pos.z;
		float p_z_max = aabb.max_pos.z;
		float o_z = ray->ori.z;
		float d_z = ray->direction.z;
		float t_z_1 = (p_z_min - o_z) / d_z;
		float t_z_2 = (p_z_max - o_z) / d_z;
		return std::tuple{ std::min(t_z_1, t_z_2), std::max(t_z_1, t_z_2) };
	};

	if (ray->direction.x == 0 && ray->direction.y == 0) {
		if (ray->ori.x < aabb.max_pos.x && ray->ori.x > aabb.min_pos.x &&
			ray->ori.y < aabb.max_pos.y && ray->ori.y > aabb.min_pos.y) {
			auto [t_z_min, t_z_max] = check_z(aabb, ray);
			return t_z_min < t_z_max&& t_z_max > 0;
		}
		else return false;
	}
	else if (ray->direction.y == 0 && ray->direction.z == 0) {
		if (ray->ori.y < aabb.max_pos.y && ray->ori.y > aabb.min_pos.y &&
			ray->ori.z < aabb.max_pos.z && ray->ori.z > aabb.min_pos.z) {
			auto [t_x_min, t_x_max] = check_x(aabb, ray);
			return t_x_min < t_x_max&& t_x_max > 0;
		}
		else return false;
	}
	else if (ray->direction.x == 0 && ray->direction.z == 0) {
		if (ray->ori.x < aabb.max_pos.x && ray->ori.x > aabb.min_pos.x &&
			ray->ori.z < aabb.max_pos.z && ray->ori.z > aabb.min_pos.z) {
			auto [t_y_min, t_y_max] = check_y(aabb, ray);
			return t_y_min < t_y_max&& t_y_max > 0;
		}
		else return false;
	}
	else if (ray->direction.x == 0) {
		if (ray->ori.x < aabb.max_pos.x && ray->ori.x > aabb.min_pos.x) {
			auto [t_y_min, t_y_max] = check_y(aabb, ray);
			auto [t_z_min, t_z_max] = check_z(aabb, ray);
			float t_min = std::max(t_y_min, t_z_min);
			float t_max = std::min(t_y_max, t_z_max);
			return t_min < t_max&& t_max > 0;
		}
		else return false;
	}
	else if (ray->direction.y == 0) {
		if (ray->ori.y < aabb.max_pos.y && ray->ori.y > aabb.min_pos.y) {
			auto [t_x_min, t_x_max] = check_x(aabb, ray);
			auto [t_z_min, t_z_max] = check_z(aabb, ray);
			float t_min = std::max(t_x_min, t_z_min);
			float t_max = std::min(t_x_max, t_z_max);
			return t_min < t_max&& t_max > 0;
		}
		else return false;
	}
	else if (ray->direction.z == 0) {
		if (ray->ori.z < aabb.max_pos.z && ray->ori.z > aabb.min_pos.z) {
			auto [t_y_min, t_y_max] = check_y(aabb, ray);
			auto [t_x_min, t_x_max] = check_x(aabb, ray);
			float t_min = std::max(t_y_min, t_x_min);
			float t_max = std::min(t_y_max, t_x_max);
			return t_min < t_max&& t_max > 0;
		}
		else return false;
	}
	else {
		auto [t_x_min, t_x_max] = check_x(aabb, ray);
		auto [t_y_min, t_y_max] = check_y(aabb, ray);
		auto [t_z_min, t_z_max] = check_z(aabb, ray);
		float t_min = std::max({ t_x_min, t_y_min, t_z_min });
		float t_max = std::min({ t_x_max, t_y_max, t_z_max });
		return t_min < t_max&& t_max > 0;
	}
}



bool _is_ray_intersect_with_tri(Triangle_raycaster& tri, const Ray_rptr ray) {
	glm::vec3 n{};
	glm::vec3 q{};

	n = glm::normalize(glm::cross((tri.vb - tri.va), (tri.vc - tri.va)));
	if (glm::dot(n, ray->direction) == 0) return false;

	float d = glm::dot(n, tri.va);
	float t = (d - glm::dot(n, ray->ori)) / glm::dot(n, ray->direction);
	if (t < 0) return false;
	q = ray->ori + t * ray->direction;

	float check_1 = glm::dot(glm::cross(tri.vb - tri.va, q - tri.va), n);
	float check_2 = glm::dot(glm::cross(tri.vc - tri.vb, q - tri.vb), n);
	float check_3 = glm::dot(glm::cross(tri.va - tri.vc, q - tri.vc), n);

	if (check_1 >= 0 && check_2 >= 0 && check_3 >= 0) {
		tri.dis = t;
		return true;
	}
	return false;
}

bool is_ray_intersect_with_mesh(const Entity_const_ptr& entity, const Ray_rptr ray, Triangle_raycaster& tri_intersect) {

	auto& triangles = entity->facedata_c();
	auto& verts = entity->vdata_c();
	size_t N_tri = triangles.size() / 3;

	std::vector<Triangle_raycaster> intersected_tris{};

	glm::vec3 va{}, vb{}, vc{};
	size_t ia{}, ib{}, ic{};

	for (int i = 0; i < N_tri; i++) {
		ia = triangles[i * 3 + 0];
		ib = triangles[i * 3 + 1];
		ic = triangles[i * 3 + 2];
		va = { verts[ia * 3 + 0], verts[ia * 3 + 1], verts[ia * 3 + 2] };
		vb = { verts[ib * 3 + 0], verts[ib * 3 + 1], verts[ib * 3 + 2] };
		vc = { verts[ic * 3 + 0], verts[ic * 3 + 1], verts[ic * 3 + 2] };
		Triangle_raycaster tri( va, vb, vc, ia, ib, ic );
		
		if (_is_ray_intersect_with_tri(tri, ray)) intersected_tris.push_back(tri);
	}
	
	if (intersected_tris.empty()) {
		return false;
	}
	std::sort(intersected_tris.begin(), intersected_tris.end(),
		[](const Triangle_raycaster& tri_a, const Triangle_raycaster& tri_b) { return tri_a.dis < tri_b.dis; });

	tri_intersect = intersected_tris[0];
	return true;
}