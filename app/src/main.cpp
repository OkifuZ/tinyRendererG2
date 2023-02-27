#include "tiny_renderer.h"
#include "tiny_phyx.h"
#include "UI_layout.h"
#include "UI_event.h"

#include <filesystem>
#include <random>



int main()
{
	TinyRenderer renderer;
	renderer.init();
	
	// attach physics to entity
	Entity_ptr sphere = nullptr;
	TinyPhyxSole_uptr tiny_physics = nullptr;

	sphere = TinyRenderer::get_entity("cloth");
	
	renderer.register_ui_layout_update(get_UI_layout_update());
	renderer.register_ui_event_handler(get_UI_event_handler(renderer, sphere, tiny_physics));
	
	renderer.loop();

	renderer.terminate();

	return 0;
}

