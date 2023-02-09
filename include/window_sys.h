#pragma once
#include <GLFW/glfw3.h>

#include <string>

class WindowSystem {
public:
	unsigned int width = 1200;
	unsigned int height = 900;
	GLFWwindow* window = nullptr;
	std::string title = "";

	bool hide = false;
};



extern WindowSystem window_global;
