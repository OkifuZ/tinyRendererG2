#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "UI_layout.h"
#include "fps.h"

UI_Flags ui_flags;

void UI_layout_update() {
	// TODO: 
	// 1. display fps
	// 2. pause
	// 3. reset

	ImGui::Begin("Simulation Viewer");
	{
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

		if (ImGui::CollapsingHeader("Projective Dynamics")) {
			ImGui::Text("common parameters");
			ImGui::DragFloat("k", &(ui_flags.k), 0.05f, 0.0f, 10000.0f);
			ImGui::DragFloat("g", &(ui_flags.g), 0.05f, 0.0f, 100.0f);
			ImGui::DragFloat("m", &(ui_flags.m), 0.005f, 0.0f, 100.0f);
			ImGui::DragFloat("dt", &(ui_flags.dt), 0.001f, 0.0f, 0.2f);
			ImGui::DragInt("iter num", &(ui_flags.iter_num), 1.0f, 5, 200);

			ImGui::Separator();
			ImGui::Text("Constraints");
			ImGui::DragFloat2("sigma", &(ui_flags.sigmas[0]), 0.01f, 0.2f, 5.0f);
			if (ui_flags.sigmas[0] > 1.0f) ui_flags.sigmas[0] = 1.0f;
			if (ui_flags.sigmas[1] < 1.0f) ui_flags.sigmas[1] = 1.0f;
		}
	}

	ImGui::End();
}