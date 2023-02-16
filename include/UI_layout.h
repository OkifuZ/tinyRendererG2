#pragma once
#include <string>

struct UI_Flags {
	enum class SIM_TYPE {
		PD,
		PBD
	};
	int sim_choice = 0;

	bool pause = true;
	bool reset = false;

	struct PD_UI_Flags {
		float k{ 100.0f };
		float sigmas[2]{ 0.97f, 1.03f };
		float dt{ 0.0166f };
		int iter_num = 30;
		float g = 9.8f;
		float m = 0.01f;
	} pd_;
	

};
extern UI_Flags ui_flags;

void UI_layout_update();
