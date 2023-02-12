#pragma once

#include <chrono>
class FPS_Counter {

	static float get_FPS() {
		static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
		auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);
		oldTime = std::chrono::high_resolution_clock::now();
		float fps = 1e6f / interval.count();
		return fps;
	}

public:
	double update_minimum_duration = 0.5 * 1e6;
	std::string fps_str = "";

	void update_fps() {
		static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
		auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);

		float fps = get_FPS();
		if (interval.count() > update_minimum_duration) { // display fps every second
			char title[256];
			title[255] = '\0';
			snprintf(title, 255, "[FPS: %3.2f]", fps);
			// TODO: UPDTAE STRING
			fps_str = std::string(title);
			oldTime = std::chrono::high_resolution_clock::now();
		}
	}

};

extern FPS_Counter fps_counter_global;