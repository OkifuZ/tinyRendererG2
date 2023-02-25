#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "UI_layout.h"
#include "file_system.h"
#include "fps.h"
#include "resource_manager.h"
#include "solver.h"

#include <filesystem>

UI_Flags ui_flags;

void top_header_layout() {
	ImGui::Text(fps_counter_global.fps_str.c_str());
	ImGui::Separator();
	
	const char* sim_items[] = {
		"No Physics",
		"Projective Dynamics",
		"Position Based Dynamics"
	};
	ImGui::Combo("Sim type", &(ui_flags.sim_choice), sim_items, IM_ARRAYSIZE(sim_items));
	ImGui::Separator();

	
}

void play_and_pause_bar() {
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
}

void save_and_load_block() {
	ImGui::Text("File save&load");
	{
		ImGui::InputText("file to save", ui_flags.file_.fname_save, IM_ARRAYSIZE(ui_flags.file_.fname_save));
		if (ImGui::Button("Save")) ui_flags.file_.to_save_params = true;
		ImGui::SameLine();
		ImGui::Text("click to load PBD params");
		ImGui::Text(ui_flags.file_.show_str_save.c_str());
	}

	{
		ImGui::InputText("file to load", ui_flags.file_.fname_load, IM_ARRAYSIZE(ui_flags.file_.fname_load));
		if (ImGui::Button("Load")) ui_flags.file_.to_load_params = true;
		ImGui::SameLine();
		ImGui::Text("click to load PBD params");
		ImGui::Text(ui_flags.file_.show_str_load.c_str());
	}
	ImGui::Separator();
}

void PD_UI_layout() {
	if (ImGui::CollapsingHeader("Projective Dynamics", 0)) {
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

		save_and_load_block();
	}
}


void PBD_UI_layout() {
	if (ImGui::CollapsingHeader("Position Based Dynamics", 0)) {
		if (ImGui::Button(ui_flags.pbd_.need_squeeze ? "unsqueeze" : "squeeze")) ui_flags.pbd_.need_squeeze = !ui_flags.pbd_.need_squeeze;
		std::string squeeze_info = ui_flags.pbd_.need_squeeze ?
			"click [reset] to activate change" : "click to squeeze";
		ImGui::Text(squeeze_info.c_str());

		ImGui::Separator();
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
		const char* constraint_items[] = {
			"Volume + Edge length",
			"Corotated + Edge length"
		};
		ImGui::Combo("constraint type", &(ui_flags.pbd_.constraint_type), constraint_items, IM_ARRAYSIZE(constraint_items));
		ImGui::DragFloat("stiff", &(ui_flags.pbd_.stiff), 0.05f, 0.0f, 10000.0f);

		ImGui::Separator();

		save_and_load_block();
	}
}


void UI_layout_update() {

	//ImGui::ShowDemoWindow();

	ImGui::Begin("Simulation Viewer");
	{

		top_header_layout();
		if (ui_flags.current_choice == static_cast<int>(UI_Flags::SIM_TYPE::None)) {
			// Do nothing
		}
		else if (ui_flags.current_choice == static_cast<int>(UI_Flags::SIM_TYPE::PD)) {
			play_and_pause_bar();
			PD_UI_layout();
		}
		else if (ui_flags.current_choice == static_cast<int>(UI_Flags::SIM_TYPE::PBD)) {
			play_and_pause_bar();
			PBD_UI_layout();
		}
		else {
			// never reached
		}
		
	}

	ImGui::End();
}

std::function<void()> get_UI_layout_update() {
	return UI_layout_update;
}


bool is_current_choice(UI_Flags::SIM_TYPE choice) {
	return (ui_flags.current_choice == static_cast<int>(choice));
}
