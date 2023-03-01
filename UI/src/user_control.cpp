#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "user_control.h"


static MouseState mouse_state{};
static KeyboardState keyboard_state{};

void MouseState::get_mouse_state(MouseState & mouse_state) {
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

void KeyboardState::get_keyboard_state(KeyboardState& keyboard_state) {

	keyboard_state.W = KEYBOARD_STAT::NONE;
	keyboard_state.A= KEYBOARD_STAT::NONE;
	keyboard_state.S = KEYBOARD_STAT::NONE;
	keyboard_state.D = KEYBOARD_STAT::NONE;
	keyboard_state.ESC = KEYBOARD_STAT::NONE;
	keyboard_state.SPACE = KEYBOARD_STAT::NONE;
	keyboard_state.LEFT_CTRL = KEYBOARD_STAT::NONE;

	if      (ImGui::IsKeyDown(ImGuiKey_W)) keyboard_state.W = KEYBOARD_STAT::DOWN;
	if      (ImGui::IsKeyPressed(ImGuiKey_W)) keyboard_state.W = KEYBOARD_STAT::PRESS;

	if (ImGui::IsKeyDown(ImGuiKey_A)) keyboard_state.A = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_A)) keyboard_state.A = KEYBOARD_STAT::PRESS;

	if (ImGui::IsKeyDown(ImGuiKey_S)) keyboard_state.S = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) keyboard_state.S = KEYBOARD_STAT::PRESS;

	if (ImGui::IsKeyDown(ImGuiKey_D)) keyboard_state.D = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_D)) keyboard_state.D = KEYBOARD_STAT::PRESS;

	if (ImGui::IsKeyDown(ImGuiKey_Escape)) keyboard_state.ESC = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_Escape)) keyboard_state.ESC = KEYBOARD_STAT::PRESS;
	
	if (ImGui::IsKeyDown(ImGuiKey_Space)) keyboard_state.SPACE = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_Space)) keyboard_state.SPACE = KEYBOARD_STAT::PRESS;
 
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) keyboard_state.LEFT_CTRL = KEYBOARD_STAT::DOWN;
	if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) keyboard_state.LEFT_CTRL = KEYBOARD_STAT::PRESS;


	auto& io = ImGui::GetIO();
	keyboard_state.captured = io.WantCaptureKeyboard;
}

void ControllSystem::process_mouse() {
	

	MouseState::get_mouse_state(mouse_state);

	// cursor on widget
	if (!mouse_state.captured_by_app) {
		// onLeftPointerRelease
		if (mouse_state.botton_released == MOUSE_BOTTON::LEFT) {
			const auto& foos = this->event_functions[MOUSE_EVENT::ON_LEFT_POINTER_RELEASE];
			for (auto& foo : foos) { foo(mouse_state); }
		}

		return;
	}
	

	// onLeftPointerDown
	if (mouse_state.botton_down == MOUSE_BOTTON::LEFT) {
		const auto& foos = this->event_functions[MOUSE_EVENT::ON_LEFT_POINTER_DOWN];
		for (auto& foo : foos) { foo(mouse_state); }
	}

	// onLeftPointerPressed
	if (mouse_state.botton_pressed == MOUSE_BOTTON::LEFT) {
		const auto& foos = this->event_functions[MOUSE_EVENT::ON_LEFT_POINTER_PRESS];
		for (auto& foo : foos) { foo(mouse_state); }
	}

	// onLEftPointerMove
	if (mouse_state.botton_down == MOUSE_BOTTON::LEFT &&
		mouse_state.botton_pressed != MOUSE_BOTTON::LEFT &&
		(mouse_state.cursor_delta_x != 0 || mouse_state.cursor_delta_y != 0)) {
		const auto& foos = this->event_functions[MOUSE_EVENT::ON_LEFT_POINTER_MOVE];
		for (auto& foo : foos) { foo(mouse_state); }
	}

	// onLeftPointerRelease
	if (mouse_state.botton_released == MOUSE_BOTTON::LEFT) {
		const auto& foos = this->event_functions[MOUSE_EVENT::ON_LEFT_POINTER_RELEASE];
		for (auto& foo : foos) { foo(mouse_state); }
	}

}


void ControllSystem::process_keyboard() {

	KeyboardState::get_keyboard_state(keyboard_state);


	if (keyboard_state.captured) {
		if (keyboard_state.ESC == KEYBOARD_STAT::PRESS)
			glfwSetWindowShouldClose(window_sys.window, true);
		return;
	}

	if (keyboard_state.ESC == KEYBOARD_STAT::PRESS) {
		glfwSetWindowShouldClose(window_sys.window, true);
		return;
	}

	for (auto& foo : keyboard_functions) {
		foo(keyboard_state);
	}

}

void ControllSystem::process_input()
{
	
	// if (!camera) return;

	this->process_mouse();

	this->process_keyboard();
	
}