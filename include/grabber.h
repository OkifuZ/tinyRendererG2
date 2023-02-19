#pragma once

#include "ray_caster.h"

#include <memory>

class Grabber;
using Grabber_rptr = Grabber*;


class Grabber {
public:
	RayCaster_sptr ray_caster = nullptr;


};

extern Grabber_rptr grabber_global;