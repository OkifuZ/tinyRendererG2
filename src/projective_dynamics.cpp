#pragma once

#include "projective_dynamics.h"
#include "UI_layout.h"

#include "json11.hpp"
#include <string>
#include <fstream>


namespace PD_param {

	float k{ 100.0f };
	float sigma_min{ 0.97f }, sigma_max{ 1.03f };
	float dt{ 0.0166f };
	int iter_num = 30;
	float g = 9.8f;
	float m = 0.01f;

	std::string PD_param_to_json_str() {
		json11::Json params = json11::Json::object{
			{"dt", ui_flags.pd_.dt},
			{"g", ui_flags.pd_.g},
			{"m", ui_flags.pd_.m},
			{"k", ui_flags.pd_.k},
			{"sigma_min", ui_flags.pd_.sigmas[0]},
			{"sigma_max", ui_flags.pd_.sigmas[1]},
			{"iter_num", ui_flags.pd_.iter_num},
		};
		std::string params_str = params.dump();
		return params_str;

	}
	bool json_str_to_PD_param(const std::string& content) {
		std::string err;
		json11::Json PD_param_json = json11::Json::parse(content, err, json11::JsonParse::COMMENTS);
		if (err != "") {
			return false;
		}
		try {
			ui_flags.pd_.dt = static_cast<float>(PD_param_json["dt"].number_value());
			ui_flags.pd_.g = static_cast<float>(PD_param_json["g"].number_value());
			ui_flags.pd_.m = static_cast<float>(PD_param_json["m"].number_value());
			ui_flags.pd_.k = static_cast<float>(PD_param_json["k"].number_value());
			ui_flags.pd_.sigmas[0] = static_cast<float>(PD_param_json["sigma_min"].number_value());
			ui_flags.pd_.sigmas[1] = static_cast<float>(PD_param_json["sigma_max"].number_value());
			ui_flags.pd_.iter_num = static_cast<float>(PD_param_json["iter_num"].int_value());
		}
		catch (...) { return false; }
		return true;
	}

	// with prefactor!!!
	void UI_to_params() {
		k = ui_flags.pd_.k;
		g = ui_flags.pd_.g;
		m = ui_flags.pd_.m;
		iter_num = ui_flags.pd_.iter_num;
		dt = ui_flags.pd_.dt;
		sigma_min = ui_flags.pd_.sigmas[0];
		sigma_max = ui_flags.pd_.sigmas[1];

		ui_flags.reset = true;
		ui_flags.pause = true;
	}
}