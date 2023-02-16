#include "tiny_renderer.h"
#include "tiny_phyx.h"

#include <filesystem>
#include <random>



int main()
{
	TinyRenderer renderer;
	renderer.init();
	
	
	TinyPhyxSole tiny_physics;
	// attach physics to entity
	Entity_ptr sphere = TinyRenderer::get_entity("sphere_tet_entity");
	tiny_physics.entity = sphere;

	tiny_physics.use_PD();
	auto reset_foo = tiny_physics.get_reset_foo();
	auto physics_tick_foo = tiny_physics.get_physics_tick_foo();

	renderer.register_ui_layout_update(UI_layout_update);
	renderer.register_reset(reset_foo);
	renderer.register_physics_tick(physics_tick_foo);

	renderer.loop();

	renderer.terminate();

	return 0;
}

