#pragma once

#include "tiny_renderer.h"
#include "tiny_phyx.h"

#include <string>
#include <functional>



std::function<void()> get_UI_event_handler(TinyRenderer& renderer, Entity_ptr& sphere, TinyPhyxSole_uptr& tiny_physics);

