#pragma once
#include <GLFW/glfw3.h>

#include <string>
#include <memory>
class WindowSystem;
using WindowSystem_rptr = WindowSystem*;
using WindowSystem_uptr = std::unique_ptr<WindowSystem>;
using WindowSystem_sptr = std::shared_ptr<WindowSystem>;

class WindowSystem {
public:
	unsigned int width = 1200;
	unsigned int height = 900;
	GLFWwindow* window = nullptr;
	std::string title = "";

	bool hide = false;
};



extern WindowSystem window_global;
