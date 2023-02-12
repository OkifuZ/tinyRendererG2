#pragma once

struct UI_Flags {
	bool pause = true;
	bool reset = false;
};
extern UI_Flags ui_flags;

void UI_layout_update();