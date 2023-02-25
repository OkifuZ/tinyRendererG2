#pragma once
#include <string>
#include <functional>

struct UI_Flags {
	bool need_break_loop = false;

	enum class SIM_TYPE {
		None,
		PD,
		PBD
	};
	int sim_choice = 0;
	int current_choice = 0;

	bool pause = true;
	bool reset = false;

	struct File_UI_Flags
	{
		bool to_save_params = false;
		bool to_load_params = false;
		char fname_save[256] = "sim_params";
		char fname_load[256] = "sim_params";
		std::string show_str_save = "";
		std::string show_str_load = "";
	} file_;

	enum class PBD_CONSTRAINT_TYPE {
		Edge_Volume, 
		Edge_Corotated
	};

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
		bool need_squeeze = false;

		int constraint_type = static_cast<int>(PBD_CONSTRAINT_TYPE::Edge_Volume);
	} pbd_;
	
	

};
extern UI_Flags ui_flags;

std::function<void()> get_UI_layout_update();

bool is_current_choice(UI_Flags::SIM_TYPE choice);

