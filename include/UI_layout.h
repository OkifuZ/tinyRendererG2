#pragma once
#include <string>

struct UI_Flags {
	bool need_break_loop = false;

	enum class SIM_TYPE {
		PD,
		PBD
	};
	int sim_choice = 0;
	int current_choice = 0;

	bool pause = true;
	bool reset = false;

	struct PD_UI_Flags {
		float k{ 100.0f };
		float sigmas[2]{ 0.97f, 1.03f };
		float dt{ 0.0166f };
		int iter_num = 30;
		float g = 9.8f;
		float m = 0.01f;
		float dampen = 0.995f;
		float mu_t = 0.7f;
		float mu_n = 0.9f;
		float ground_height = 0.0f;
	} pd_;

	struct PBD_UI_Flags {
		float dampen{ 0.995 };
		float ground_height{ 0.0f };
		float mu_t{ 0.7f };
		float mu_n{ 0.9f };
		float stiff{ 10.0f };
		int substep_num{ 20 };
		float dt{ 0.02f };
		float m{ 0.01f };
		float g{ 9.8f };
	} pbd_;
	
	

};
extern UI_Flags ui_flags;

void UI_layout_update();
