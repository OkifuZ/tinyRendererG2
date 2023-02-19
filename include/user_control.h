#pragma once
#include "window_sys.h"
#include "camera.h"
#include "grabber.h"
#include <memory>

class ControllSystem;
typedef ControllSystem* ControllSystem_rptr;
typedef std::unique_ptr<ControllSystem> ControllSystem_uptr;
typedef std::shared_ptr<ControllSystem> ControllSystem_sptr;


class ControllSystem {
	WindowSystem& window_sys = window_global;
	Camera_ptr camera = nullptr;
	Grabber_rptr grabber = nullptr;

public:
	ControllSystem() {}
	ControllSystem(const ControllSystem&) = delete;
	ControllSystem& operator=(const ControllSystem&) = delete;

	void register_camera(Camera_ptr camera) {
		this->camera = camera;
	}

	void process_input();
	void process_mouse();
	void process_keyboard();
};