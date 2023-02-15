#pragma once

struct UI_Flags {
	bool pause = true;
	bool reset = false;

	float k{ 10.0f };
	float sigmas[2]{0.95f, 1.05f};
	float dt{ 0.02f };
	int iter_num = 10;
	float g = 0.5f;
	float m = 1.0f;

};
extern UI_Flags ui_flags;

void UI_layout_update();