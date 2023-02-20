#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "user_control.h"


enum class MOUSE_BOTTON {
	LEFT,
	RIGHT,
	MIDDLE,
	NONE
};

struct MouseState {
	MOUSE_BOTTON botton_down_lasttime = MOUSE_BOTTON::NONE;
	MOUSE_BOTTON botton_down = MOUSE_BOTTON::NONE;
	MOUSE_BOTTON botton_pressed = MOUSE_BOTTON::NONE;
	MOUSE_BOTTON botton_released = MOUSE_BOTTON::NONE;
	float cursor_x = 0;
	float cursor_y = 0;
	float cursor_delta_x = 0;
	float cursor_delta_y = 0;
	bool captured_by_app = false;
	bool pos_valid = false;

	static void get_mouse_state(MouseState& mouse_state) {
		mouse_state.pos_valid = ImGui::IsMousePosValid();
		if (ImGui::IsMouseDown(0)) mouse_state.botton_down = MOUSE_BOTTON::LEFT;
		else if (ImGui::IsMouseDown(1)) mouse_state.botton_down = MOUSE_BOTTON::RIGHT;
		else if (ImGui::IsMouseDown(2)) mouse_state.botton_down = MOUSE_BOTTON::MIDDLE;
		else                            mouse_state.botton_down = MOUSE_BOTTON::NONE;
		if (mouse_state.botton_down != mouse_state.botton_down_lasttime) {
			mouse_state.botton_pressed = mouse_state.botton_down;
			mouse_state.botton_released = mouse_state.botton_down_lasttime;
		}
		else {
			mouse_state.botton_pressed = MOUSE_BOTTON::NONE;
			mouse_state.botton_released = MOUSE_BOTTON::NONE;
		}
		mouse_state.botton_down_lasttime = mouse_state.botton_down;

		auto& io = ImGui::GetIO();
		mouse_state.cursor_x = io.MousePos.x;
		mouse_state.cursor_y = io.MousePos.y;
		mouse_state.cursor_delta_x = io.MouseDelta.x;
		mouse_state.cursor_delta_y = io.MouseDelta.y;

		mouse_state.captured_by_app = !io.WantCaptureMouse;
	}
};



static MouseState mouse_state{};


void ControllSystem::process_mouse() {
	

	MouseState::get_mouse_state(mouse_state);

	// cursor on widget
	if (!mouse_state.captured_by_app) return;
	
	// TODO: we can make this callback style: onPointerLeftDown...
	
	// cursor not on widget
	// camera movement
	if (camera) {
		if (mouse_state.botton_down == MOUSE_BOTTON::LEFT) {
			camera->update_xy_offset(mouse_state.cursor_delta_x, mouse_state.cursor_delta_y, 0.1);
		}
	}

	// grabber dragging
	if (grabber) {
		// on mouse pressed
		if (mouse_state.botton_pressed == MOUSE_BOTTON::LEFT) {
		}

		// on mouse grab&move
		if (mouse_state.botton_down == MOUSE_BOTTON::LEFT &&
			mouse_state.botton_pressed != MOUSE_BOTTON::LEFT &&
			(mouse_state.cursor_delta_x != 0 || mouse_state.cursor_delta_y != 0)) {
		}

		// on mouse leave
		if (mouse_state.botton_released == MOUSE_BOTTON::LEFT) {
		}
	}

	if (mouse_state.botton_pressed == MOUSE_BOTTON::LEFT) {
		generate_lines_entity(std::vector{ std::tuple{glm::vec3{0,0,0}, glm::vec3{0,3,0}} }, "line");
	}
	// test 
	/*if (mouse_state.botton_pressed == MOUSE_BOTTON::LEFT) {
		glm::vec4 v{};
		
		v.x = 2.0f * mouse_state.cursor_x / window_sys.width - 1.0f;
		v.y = 1.0f - 2.0f * (mouse_state.cursor_y) / window_sys.height;
		v.z = 1.0f;
		v.w = 0.05f;
		glm::mat4 mat_view = camera->get_view_mat();
		glm::mat4 mat_proj = camera->get_proj_mat(window_sys.width, window_sys.height);
		glm::mat4 v_inv_p_inv = glm::inverse(mat_proj * mat_view);

		v = v_inv_p_inv * v;
		v /= v.w;
		printf("pos: %.4f, %.4f, %.4f (%.4f)\n", v.x, v.y, v.z, v.w);
	}*/
}


void ControllSystem::process_keyboard() {
	// TODO
	auto& io = ImGui::GetIO();

	if (io.WantCaptureKeyboard) {
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			glfwSetWindowShouldClose(window_sys.window, true);
		return;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		glfwSetWindowShouldClose(window_sys.window, true);
		return;
	}
	if (ImGui::IsKeyDown(ImGuiKey_W)) {
		if (camera) camera->transform.translate += camera->transform.get_front() * 0.1f;
	}
	if (ImGui::IsKeyDown(ImGuiKey_S)) {
		if (camera) camera->transform.translate += -camera->transform.get_front() * 0.1f;
	}
	if (ImGui::IsKeyDown(ImGuiKey_D)) {
		if (camera) camera->transform.translate += camera->transform.get_right() * 0.1f;
	}
	if (ImGui::IsKeyDown(ImGuiKey_A)) {
		if (camera) camera->transform.translate += -camera->transform.get_right() * 0.1f;
	}
	if (ImGui::IsKeyDown(ImGuiKey_R)) {}
	if (ImGui::IsKeyDown(ImGuiKey_P)) {}
	if (ImGui::IsKeyDown(ImGuiKey_Space)) {
		if (camera) camera->transform.translate += glm::vec3{ 0,1,0 } *0.1f;
	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		if (camera) camera->transform.translate += -glm::vec3{ 0,1,0 } *0.1f;
	}
}

void ControllSystem::process_input()
{
	
	if (!camera) return;

	this->process_mouse();

	this->process_keyboard();
	
}