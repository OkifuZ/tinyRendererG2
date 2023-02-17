#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "UI_layout.h"
#include "file_system.h"
#include "fps.h"
#include "resource_manager.h"
#include "projective_dynamics.h"
#include "solver.h"

#include <filesystem>

UI_Flags ui_flags;

void top_header_layout() {
	ImGui::Text(fps_counter_global.fps_str.c_str());
	ImGui::Separator();
	if (ImGui::BeginTable("table1", 2))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Checkbox("pause", &ui_flags.pause);
		ImGui::TableSetColumnIndex(1);
		if (ImGui::Button("reset")) ui_flags.reset = true;
		ImGui::EndTable();
	}
	ImGui::Separator();

	const char* sim_items[] = {
		"Projective Dynamics",
		"Position Based Dynamics"
	};
	ImGui::Combo("Sim type", &(ui_flags.sim_choice), sim_items, IM_ARRAYSIZE(sim_items));
}

void PD_UI_layout() {
	if (ImGui::CollapsingHeader("Projective Dynamics", 1)) {
		ImGui::Text("common parameters");
		ImGui::DragFloat("g", &(ui_flags.pd_.g), 0.05f, 0.0f, 100.0f, "%.5f");
		ImGui::DragFloat("m", &(ui_flags.pd_.m), 0.005f, 0.0f, 100.0f, "%.5f");
		ImGui::DragFloat("dt", &(ui_flags.pd_.dt), 0.001f, 0.0f, 0.2f, "%.5f");
		ImGui::DragFloat("dampen", &(ui_flags.pd_.dampen), 0.0005f, 0.8f, 1.0f, "%.5f");
		ImGui::DragFloat("mu_t", &(ui_flags.pd_.mu_t), 0.001f, 0.0f, 1.0f, "%.5f");
		ImGui::DragFloat("mu_n", &(ui_flags.pd_.mu_n), 0.001f, 0.0f, 1.0f, "%.5f");
		ImGui::DragInt("iter num", &(ui_flags.pd_.iter_num), 1.0f, 5, 200);

		ImGui::Separator();
		ImGui::DragFloat("ground height", &(ui_flags.pd_.ground_height), 0.05f, -10, 10);

		ImGui::Separator();
		ImGui::Text("Constraints");
		ImGui::DragFloat("k", &(ui_flags.pd_.k), 0.05f, 0.0f, 10000.0f);
		ImGui::DragFloat2("sigma", &(ui_flags.pd_.sigmas[0]), 0.01f, 0.2f, 5.0f);
		if (ui_flags.pd_.sigmas[0] > 1.0f) ui_flags.pd_.sigmas[0] = 1.0f;
		if (ui_flags.pd_.sigmas[1] < 1.0f) ui_flags.pd_.sigmas[1] = 1.0f;

		ImGui::Separator();
		{
			static bool to_save_params = false;
			static char save_fname[256] = "PD_params";
			static std::string show_str_save = "";
			ImGui::InputText("file to save", save_fname, IM_ARRAYSIZE(save_fname));
			if (ImGui::Button("Save")) to_save_params = true;
			if (to_save_params)
			{
				std::filesystem::path save_path = resource_manager_global.simulation_path / save_fname;
				if (save_str_to_file(save_path, SimPD::PD_param_to_json_str())) {
					show_str_save = "saved to " + save_path.lexically_normal().string();
				}
				else {
					show_str_save = "ERROR: invalid path: " + save_path.lexically_normal().string();
				}
				to_save_params = false;
			}
			ImGui::SameLine();
			ImGui::Text("click to load PD params");
			ImGui::Text(show_str_save.c_str());
		}

		ImGui::Separator();
		{
			static bool to_load_params = false;
			static char load_fname[256] = "PD_params";
			static std::string show_str_load = "";
			ImGui::InputText("file to load", load_fname, IM_ARRAYSIZE(load_fname));
			if (ImGui::Button("Load")) to_load_params = true;
			if (to_load_params)
			{
				std::filesystem::path load_path = resource_manager_global.simulation_path / load_fname;
				std::string PD_param_json_str = read_file_as_str(load_path.string());
				bool stat_load = SimPD::json_str_to_PD_param(PD_param_json_str);
				if (stat_load) {
					SimPD::UI_to_params();
					show_str_load = "loaded " + load_path.lexically_normal().string();
				}
				else {
					show_str_load = "ERROR: loading failed " + load_path.lexically_normal().string();
				}
				to_load_params = false;
			}
			ImGui::SameLine();
			ImGui::Text("click to load PD params");
			ImGui::Text(show_str_load.c_str());
		}
		ImGui::Separator();
	}
}


void PBD_UI_layout() {
	if (ImGui::CollapsingHeader("Position Based Dynamics", 1)) {
		ImGui::Text("common parameters");
		ImGui::DragFloat("g", &(ui_flags.pbd_.g), 0.05f, 0.0f, 100.0f, "%.5f");
		ImGui::DragFloat("m", &(ui_flags.pbd_.m), 0.005f, 0.0f, 100.0f, "%.5f");
		ImGui::DragFloat("dt", &(ui_flags.pbd_.dt), 0.001f, 0.0f, 0.2f, "%.5f");
		ImGui::DragFloat("dampen", &(ui_flags.pbd_.dampen), 0.0005f, 0.8f, 1.0f, "%.5f");
		ImGui::DragFloat("mu_t", &(ui_flags.pbd_.mu_t), 0.001f, 0.0f, 1.0f, "%.5f");
		ImGui::DragFloat("mu_n", &(ui_flags.pbd_.mu_n), 0.001f, 0.0f, 1.0f, "%.5f");
		ImGui::DragInt("substep_num", &(ui_flags.pbd_.substep_num), 1.0f, 5, 200);

		ImGui::Separator();
		ImGui::DragFloat("ground height", &(ui_flags.pbd_.ground_height), 0.05f, -10, 10);

		ImGui::Separator();
		ImGui::Text("Constraints");
		ImGui::DragFloat("stiff", &(ui_flags.pbd_.stiff), 0.05f, 0.0f, 10000.0f);

		ImGui::Separator();
		{
			static bool to_save_params = false;
			static char save_fname[256] = "PBD_params";
			static std::string show_str_save = "";
			ImGui::InputText("file to save", save_fname, IM_ARRAYSIZE(save_fname));
			if (ImGui::Button("Save")) to_save_params = true;
			if (to_save_params)
			{
				std::filesystem::path save_path = resource_manager_global.simulation_path / save_fname;
				if (save_str_to_file(save_path, SimPBD::PBD_param_to_json_str())) {
					show_str_save = "saved to " + save_path.lexically_normal().string();
				}
				else {
					show_str_save = "ERROR: invalid path: " + save_path.lexically_normal().string();
				}
				to_save_params = false;
			}
			ImGui::SameLine();
			ImGui::Text("click to load PBD params");
			ImGui::Text(show_str_save.c_str());
		}

		ImGui::Separator();
		{
			static bool to_load_params = false;
			static char load_fname[256] = "PBD_params";
			static std::string show_str_load = "";
			ImGui::InputText("file to load", load_fname, IM_ARRAYSIZE(load_fname));
			if (ImGui::Button("Load")) to_load_params = true;
			if (to_load_params)
			{
				std::filesystem::path load_path = resource_manager_global.simulation_path / load_fname;
				std::string PD_param_json_str = read_file_as_str(load_path.string());
				bool stat_load = SimPBD::json_str_to_PBD_param(PD_param_json_str);
				if (stat_load) {
					SimPBD::UI_to_params();
					show_str_load = "loaded " + load_path.lexically_normal().string();
				}
				else {
					show_str_load = "ERROR: loading failed " + load_path.lexically_normal().string();
				}
				to_load_params = false;
			}
			ImGui::SameLine();
			ImGui::Text("click to load PBD params");
			ImGui::Text(show_str_load.c_str());
		}
		ImGui::Separator();
	}
}


void UI_layout_update() {

	ImGui::Begin("Simulation Viewer");
	{

		top_header_layout();

		if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PD)) {
			if (ui_flags.current_choice != ui_flags.sim_choice) {
				ui_flags.current_choice = ui_flags.sim_choice;
				ui_flags.need_break_loop = true;
			}
			PD_UI_layout();
		}
		
		if (ui_flags.sim_choice == static_cast<int>(UI_Flags::SIM_TYPE::PBD)) {
			if (ui_flags.current_choice != ui_flags.sim_choice) {
				ui_flags.current_choice = ui_flags.sim_choice;
				ui_flags.need_break_loop = true;
			}
			PBD_UI_layout();
		}
		
	}

	ImGui::End();
}