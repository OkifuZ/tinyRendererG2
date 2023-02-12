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

		if (ImGui::CollapsingHeader("Configuration")) {
			ImGui::Text("some configuration");
		}
	}

	ImGui::End();
}