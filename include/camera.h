#pragma once

#include "transform.h"
#include "window_sys.h"
#include "user_control.h"

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <functional>

class Camera;
typedef std::shared_ptr<Camera> Camera_ptr;

class Camera {
public:
	
	std::string name = "";
	Transform transform;
	float near = 0.1f;
	float far = 40.0f;
	float fov_degree = 45.0f;
	
	float scr_width = 0;
	float scr_height = 0;

	bool freezed = false;

	WindowSystem_rptr current_window;

	bool cam_look_at(glm::vec3 source, glm::vec3 target, glm::vec3 up) {
		glm::vec3 dir = target - source;
		if (glm::length(dir) < 0.001) return false;
		if (std::abs(glm::dot(glm::normalize(dir), glm::vec3{ 0, 1, 0 })) > 0.99) return false;
		transform.look_at(source, source + dir, up); 
		return true;
	}

	glm::mat4 get_view_mat() {
		return glm::lookAt(
			transform.translate,
			transform.translate + transform.get_front() * 0.5f,
			glm::vec3{0, 1, 0});
	}

	glm::mat4 get_proj_mat() {
		return glm::perspective(glm::radians(fov_degree), (float)current_window->width / (float)current_window->height, near, far);
	}
	
	void update_xy_offset(float x_off, float y_off, float scale) {
		//transform.yaw_pitch_roll(-x_off * 0.005, -y_off * 0.005, 0);
		transform.yaw_add(-x_off * scale);
		transform.pitch_add(-y_off * scale);
	}

	void register_event_to_controller(ControllSystem& ctr_sys) {
		auto on_move_event = [this](const MouseState& mouse_state) {
			if (freezed) return;
			update_xy_offset(mouse_state.cursor_delta_x, mouse_state.cursor_delta_y, 0.1);
		};
		ctr_sys.register_mouse_event_function(ControllSystem::MOUSE_EVENT::ON_LEFT_POINTER_MOVE, on_move_event);
	}

	void register_keyboard_to_controller(ControllSystem& ctr_sys) {
		auto on_key_handler = [this](const KeyboardState& keyboard_state) {
			if (freezed) return;

			if (keyboard_state.W == KEYBOARD_STAT::DOWN) {
				transform.translate += transform.get_front() * 0.1f;
			}
			if (keyboard_state.S == KEYBOARD_STAT::DOWN) {
				transform.translate += -transform.get_front() * 0.1f;
			}
			if (keyboard_state.D == KEYBOARD_STAT::DOWN) {
				transform.translate += transform.get_right() * 0.1f;
			}
			if (keyboard_state.A == KEYBOARD_STAT::DOWN) {
				transform.translate += -transform.get_right() * 0.1f;
			}

			if (keyboard_state.SPACE == KEYBOARD_STAT::DOWN) {
				transform.translate += glm::vec3{ 0,1,0 } *0.1f;
			}
			if (keyboard_state.LEFT_CTRL == KEYBOARD_STAT::DOWN) {
				transform.translate += -glm::vec3{ 0,1,0 } *0.1f;
			}
		};
		ctr_sys.register_keyboard_function(on_key_handler);
	}

};