#pragma once

#include "transform.h"

#include <glm/glm.hpp>
#include <memory>


class Camera;
typedef std::shared_ptr<Camera> Camera_ptr;

class Camera {
public:
	std::string name = "";
	Transform transform;
	float near = 0.1f;
	float far = 40.0f;
	float fov_degree = 45.0f;

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

	glm::mat4 get_proj_mat(float scr_w, float scr_h) {
		return glm::perspective(glm::radians(fov_degree), scr_w / scr_h, near, far);
	}
	
	void update_xy_offset(float x_off, float y_off, float scale) {
		//transform.yaw_pitch_roll(-x_off * 0.005, -y_off * 0.005, 0);
		transform.yaw_add(-x_off * scale);
		transform.pitch_add(-y_off * scale);
		glm::vec3 eu = transform.get_pyr();
		glm::quat q = transform.rotationQ;
	}

};