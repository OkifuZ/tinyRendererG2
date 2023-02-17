#pragma once

#include <string>


namespace SimPD {

	extern float k;
	extern float sigma_min, sigma_max;
	extern float dt;
	extern int iter_num;
	extern float g;
	extern float m;
	extern float dampen;
	extern float mu_t;
	extern float mu_n;
	extern float ground_height;


	std::string PD_param_to_json_str();
	bool json_str_to_PD_param(const std::string& content);
	void UI_to_params();

}