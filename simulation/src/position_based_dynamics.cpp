#include "position_based_dynamics.h"
#include "UI_layout.h"

#include "json11.hpp"

namespace SimPBD {

	float dampen{0.995};
	float ground_height{ 0.0f };
	float mu_t{ 0.7f };
	float mu_n{ 0.9f };
	float stiff{ 10.0f };
	int substep_num{ 20 };
	float dt{ 0.02f };
	float m{ 0.01f };
	float g{ 9.8f };

	std::string PBD_param_to_json_str() {
		json11::Json params = json11::Json::object{
			{"dt", ui_flags.pbd_.dt},
			{"g", ui_flags.pbd_.g},
			{"m", ui_flags.pbd_.m},
			{"stiff", ui_flags.pbd_.stiff},
			{"substep_num", ui_flags.pbd_.substep_num},
			{"dampen", ui_flags.pbd_.dampen},
			{"mu_t", ui_flags.pbd_.mu_t},
			{"mu_n", ui_flags.pbd_.mu_n},
			{"ground_height", ui_flags.pbd_.ground_height},
		};
		std::string params_str = params.dump();
		return params_str;
	}

	bool json_str_to_PBD_param(const std::string& content) {
		std::string err;
		json11::Json PD_param_json = json11::Json::parse(content, err, json11::JsonParse::COMMENTS);
		if (err != "") {
			return false;
		}
		try {
			ui_flags.pbd_.dt = static_cast<float>(PD_param_json["dt"].number_value());
			ui_flags.pbd_.g = static_cast<float>(PD_param_json["g"].number_value());
			ui_flags.pbd_.m = static_cast<float>(PD_param_json["m"].number_value());
			ui_flags.pbd_.stiff = static_cast<float>(PD_param_json["stiff"].number_value());
			ui_flags.pbd_.mu_t = static_cast<float>(PD_param_json["mu_t"].number_value());
			ui_flags.pbd_.mu_n = static_cast<float>(PD_param_json["mu_t_n"].number_value());
			ui_flags.pbd_.substep_num = PD_param_json["substep_num"].int_value();
			ui_flags.pbd_.dampen = static_cast<float>(PD_param_json["dampen"].number_value());
			ui_flags.pbd_.ground_height = static_cast<float>(PD_param_json["ground_height"].number_value());

		}
		catch (...) { return false; }
		return true;
	}

	void UI_to_params() {
		dampen = ui_flags.pbd_.dampen;
		ground_height = ui_flags.pbd_.ground_height;
		mu_t = ui_flags.pbd_.mu_t;
		mu_n = ui_flags.pbd_.mu_n;
		stiff = ui_flags.pbd_.stiff;
		substep_num = ui_flags.pbd_.substep_num;
		dt = ui_flags.pbd_.dt;
		m = ui_flags.pbd_.m;
		g = ui_flags.pbd_.g;
	}


}