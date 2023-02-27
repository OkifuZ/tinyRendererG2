#include "UI_event.h"
#include "UI_layout.h"

#include "projective_dynamics.h"
#include "position_based_dynamics.h"
#include "tiny_renderer.h"
#include "tiny_phyx.h"

#include "json11.hpp"
#include <string>
#include <functional>


void save_params();
void load_params();
std::string get_current_choice_params_str();
bool load_current_choice_params(const std::string& content);
void reload_physics_scene(TinyRenderer& renderer, Entity_ptr& sphere, TinyPhyxSole_uptr& tiny_physics);


std::string get_current_choice_params_str() {
	std::string content;
	if (is_current_choice(UI_Flags::SIM_TYPE::PD)) content = SimPD::PD_param_to_json_str();
	else if (is_current_choice(UI_Flags::SIM_TYPE::PBD)) content = SimPBD::PBD_param_to_json_str();
	else if (is_current_choice(UI_Flags::SIM_TYPE::None)) content = "";
	else content = "";
	return content;
}

bool load_current_choice_params(const std::string& content) {
	bool res = false;
	if (is_current_choice(UI_Flags::SIM_TYPE::PD)) res = SimPBD::json_str_to_PBD_param(content);
	else if (is_current_choice(UI_Flags::SIM_TYPE::PBD)) res = SimPBD::json_str_to_PBD_param(content);
	else if (is_current_choice(UI_Flags::SIM_TYPE::None)) res = true;
	else res = false;
	return res;
}

void reload_physics_scene(TinyRenderer& renderer, Entity_ptr& sphere, TinyPhyxSole_uptr& tiny_physics)
{
	// Dangerous: level 0 caution of lifecycle of tiny_physics && sphere!!!

	// reset render scene
	if (tiny_physics) {
		sphere->vdata() = tiny_physics->vdata_ori;
		//sphere->ndata() = ndata_ori;
		auto gizmo = get_bound_gizmo(sphere);
		if (gizmo) gizmo->odata() = tiny_physics->vdata_ori;
	}

	// clear all physics
	if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::None)) {
		tiny_physics = nullptr;
		resource_manager_global.grabber->enabled = false;
		renderer.register_physics_tick([]() {});
	}
	else if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PD)) {
		resource_manager_global.grabber->enabled = false;
		tiny_physics = std::make_unique<TinyPhyxSole_PD>();
	} 
	else if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PBD)) {
		resource_manager_global.grabber->enabled = true;
		tiny_physics = std::make_unique<TinyPhyxSole_PBD>();
	}
		
	if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PD) || 
		ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PBD)) 
	{
		tiny_physics->register_entity(sphere);
		tiny_physics->use();
		renderer.register_physics_tick(tiny_physics->get_physics_tick_foo());
	}
}

void save_params()
{
	std::filesystem::path save_path = resource_manager_global.simulation_path / ui_flags.file_.fname_save;

	if (save_str_to_file(save_path, get_current_choice_params_str())) {
		ui_flags.file_.show_str_save = "saved to " + save_path.lexically_normal().string();
	}
	else {
		ui_flags.file_.show_str_save = "ERROR: invalid path: " + save_path.lexically_normal().string();
	}
	ui_flags.file_.to_save_params = false;
}

void load_params()
{
	std::filesystem::path load_path = resource_manager_global.simulation_path / ui_flags.file_.fname_load;
	std::string param_json_str = read_file_as_str(load_path.string());
	bool stat_load = load_current_choice_params(param_json_str);
	if (stat_load) {
		SimPBD::UI_to_params();
		ui_flags.file_.show_str_load = "loaded " + load_path.lexically_normal().string();
	}
	else {
		ui_flags.file_.show_str_load = "ERROR: loading failed " + load_path.lexically_normal().string();
	}
	ui_flags.file_.to_load_params = false;
	ui_flags.reset = true;
}

std::function<void()> get_UI_event_handler(TinyRenderer& renderer, Entity_ptr& sphere, TinyPhyxSole_uptr& tiny_physics) {
	return [&renderer, &sphere, &tiny_physics]() {
		// need reset
		if (ui_flags.reset) {
			if (tiny_physics) tiny_physics->get_reset_foo()();

			ui_flags.reset = false;
			ui_flags.pause = true;
		}

		// reload physical scene
		if (ui_flags.current_choice != ui_flags.sim_choice) {
			reload_physics_scene(renderer, sphere, tiny_physics);
			ui_flags.current_choice = ui_flags.sim_choice;
			ui_flags.pause = true;
		}

		// save params
		if (ui_flags.file_.to_save_params)
			save_params();

		// load params
		if (ui_flags.file_.to_load_params)
			load_params();

		// pause
		if (ui_flags.pause) {
			if (tiny_physics) tiny_physics->paused = true;
		}
		else {
			if (tiny_physics) tiny_physics->paused = false;
		}
	};
}


