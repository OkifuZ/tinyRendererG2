#pragma once
#include "window_sys.h"
#include "camera.h"


class ControllSystem {
	WindowSystem& window_sys = window_global;
	Camera_ptr camera = nullptr;
public:
	ControllSystem() {}
	ControllSystem(const ControllSystem&) = delete;
	ControllSystem& operator=(const ControllSystem&) = delete;

	void register_camera(Camera_ptr camera) {
		this->camera = camera;
	}

	void process_input();
};