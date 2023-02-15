#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "user_control.h"


void ControllSystem::process_input()
{
	if (!camera) return;

	GLFWwindow* window = window_sys.window;

	static double last_X = 0, last_Y = 0;
	static bool firstMouse_left = true;

	// if mouse on imgui windows, don't process this event; so as keyboard
	auto& io = ImGui::GetIO();
	if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		return;
	}

	int state_left = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT));
	double current_X = 0, current_Y = 0;
	double x_off = 0, y_off = 0;
	glfwGetCursorPos(window, &current_X, &current_Y);
	if (state_left == GLFW_PRESS) {
		if (!firstMouse_left) {
			x_off = current_X - last_X;
			y_off = current_Y - last_Y;
		}
		firstMouse_left = false;
		last_X = current_X;
		last_Y = current_Y;
		camera->update_xy_offset(x_off, y_off, 0.1);
	}
	else if (state_left == GLFW_RELEASE) {
		firstMouse_left = true;
		last_X = 0;
		last_Y = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		if (camera) camera->transform.translate += camera->transform.get_front() * 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		if (camera) camera->transform.translate += -camera->transform.get_front() * 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (camera) camera->transform.translate += camera->transform.get_right() * 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		if (camera) camera->transform.translate += -camera->transform.get_right() * 0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (camera) camera->transform.translate += glm::vec3{ 0,1,0 } *0.1f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		if (camera) camera->transform.translate += -glm::vec3{ 0,1,0 } *0.1f;
	}
}