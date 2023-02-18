#include "tiny_renderer.h"
#include "tiny_phyx.h"
#include "UI_layout.h"

#include <filesystem>
#include <random>



int main()
{
	TinyRenderer renderer;
	renderer.init();
	
	// attach physics to entity
	Entity_ptr sphere = TinyRenderer::get_entity("sphere_tet_entity");


	//TinyPhyxSole_uptr tiny_physics = std::make_unique<TinyPhyxSole_PD>();
	TinyPhyxSole_uptr tiny_physics = nullptr;
	//tiny_physics = std::make_unique<TinyPhyxSole_PD>();
	tiny_physics = std::make_unique<TinyPhyxSole_PBD>();

	/*if (ui_flags.current_choice == static_cast<int>(UI_Flags::SIM_TYPE::PD)) 
		tiny_physics = std::make_unique<TinyPhyxSole_PD>();

	if (ui_flags.current_choice == static_cast<int>(UI_Flags::SIM_TYPE::PBD))
		tiny_physics = std::make_unique<TinyPhyxSole_PBD>();*/

	tiny_physics->entity = sphere;

	tiny_physics->use();
	auto reset_foo = tiny_physics->get_reset_foo();
	auto physics_tick_foo = tiny_physics->get_physics_tick_foo();

	renderer.register_ui_layout_update(UI_layout_update);
	renderer.register_reset(reset_foo);
	renderer.register_physics_tick(physics_tick_foo);

	renderer.loop();

	renderer.terminate();

	return 0;
}

