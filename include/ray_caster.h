#pragma once

#include "entity.h"
#include "window_sys.h"
#include "camera.h"
#include "resource_manager.h"

#include <glm/glm.hpp>

#include <memory>

class RayCaster;
struct Ray;
struct Intersect;
struct Triangle_raycaster;

using RayCaster_sptr = std::shared_ptr<RayCaster>;
using RayCaster_uptr = std::unique_ptr<RayCaster>;
using RayCaster_rptr = RayCaster*;

using Ray_uptr = std::unique_ptr<Ray>;
using Ray_rptr = Ray*;


bool is_ray_intersect_with_aabb(const Entity_const_ptr& entity, const Ray_rptr ray);
bool is_ray_intersect_with_mesh(const Entity_const_ptr& entity, const Ray_rptr ray, Triangle_raycaster&);

struct Triangle_raycaster
{
	glm::vec3 va{};
	glm::vec3 vb{};
	glm::vec3 vc{};
	size_t ia{}, ib{}, ic{};
	float dis = 0;
	Triangle_raycaster() = default;
	Triangle_raycaster(glm::vec3& va, glm::vec3& vb, glm::vec3& vc, size_t ia, size_t ib, size_t ic)
		: va(va), vb(vb), vc(vc), ia(ia), ib(ib), ic(ic) {}
};

struct Intersect {
	Entity_ptr entity = nullptr;
	Triangle_raycaster triangle{};
};


struct Ray {
	glm::vec3 ori{};
	glm::vec3 direction{};
	Intersect intersects;

	Ray() = default;
	Ray(const glm::vec3& ori, const glm::vec3& direction) : ori(ori), direction(direction) {}
};


class RayCaster {

public:
	Ray_uptr ray = nullptr;

	void cast_ray_from_camera(Camera_ptr camera, float x, float y);

private:

	


};