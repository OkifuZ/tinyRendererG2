#pragma once

#include "camera.h"
#include "entity.h"

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
};

class RayCaster {
public:
	Ray_uptr ray = nullptr;
	void cast_ray_from_camera(Camera_ptr camera, float x, float y) {};
};