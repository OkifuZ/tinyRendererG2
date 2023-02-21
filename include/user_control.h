#pragma once

#include "window_sys.h"

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <map>

class ControllSystem;
typedef ControllSystem* ControllSystem_rptr;
typedef std::unique_ptr<ControllSystem> ControllSystem_uptr;
typedef std::shared_ptr<ControllSystem> ControllSystem_sptr;


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

	static void get_mouse_state(MouseState& mouse_state);
};


enum class KEYBOARD_STAT {
	DOWN,
	PRESS,
	NONE
};

struct KeyboardState {
	KEYBOARD_STAT W = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT A = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT S = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT D = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT SPACE = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT LEFT_CTRL = KEYBOARD_STAT::NONE;
	KEYBOARD_STAT ESC = KEYBOARD_STAT::NONE;
	bool captured = false;

	static void get_keyboard_state(KeyboardState& keyboard_state);
};


class ControllSystem {
	WindowSystem& window_sys = window_global;

public:

	enum class MOUSE_EVENT {
		ON_LEFT_POINTER_PRESS,
		ON_LEFT_POINTER_DOWN,
		ON_LEFT_POINTER_MOVE, // down & move
		ON_LEFT_POINTER_RELEASE,
		INVALID
	};

	ControllSystem() {
		for (int event = static_cast<int>(MOUSE_EVENT::ON_LEFT_POINTER_PRESS);
			event != static_cast<int>(MOUSE_EVENT::INVALID); event++) {
			this->event_functions[static_cast<MOUSE_EVENT>(event)] = {};
		}
	}

	ControllSystem(const ControllSystem&) = delete;
	ControllSystem& operator=(const ControllSystem&) = delete;

	

	void process_input();
	void process_mouse();
	void process_keyboard();

	void register_mouse_event_function(MOUSE_EVENT event, std::function<void(const MouseState&)> foo) {
		event_functions[event].push_back(foo);
	}

	void register_keyboard_function(std::function<void(const KeyboardState&)> foo) {
		keyboard_functions.push_back(foo);
	}

private:
	std::map<MOUSE_EVENT, std::vector<std::function<void(const MouseState&)>>>
		event_functions;
	std::vector<std::function<void(const KeyboardState&)>> keyboard_functions;

};

