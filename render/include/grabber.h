#pragma once

#include "ray_caster.h"
#include "user_control.h"
#include "camera.h"
#include <memory>

class Grabber;
using Grabber_rptr = Grabber*;
using Grabber_sptr = std::shared_ptr<Grabber>;


class Grabber {
public:
	RayCaster_uptr ray_caster = nullptr;
	Camera_ptr camera = nullptr;

	Grabber(Camera_ptr cam): ray_caster(std::make_unique<RayCaster>()), camera(cam) {}
	
	Entity_ptr grabbed_entity = nullptr;
	float distance = 0.0f;

	void register_grabber_to_controller(ControllSystem& ctr_sys);


};

extern Grabber_rptr grabber_global;