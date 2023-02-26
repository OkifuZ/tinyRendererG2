#pragma once

#include <string>

namespace SimPBD {

	extern float dampen;
	extern float ground_height;
	extern float mu_t;
	extern float mu_n;
	extern float stiff;
	extern int substep_num;
	extern float dt;
	extern float m;
	extern float g;

	std::string PBD_param_to_json_str();
	bool json_str_to_PBD_param(const std::string& content);
	void UI_to_params();

}