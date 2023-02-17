#include "tiny_renderer.h"

#include "UI_layout.h"
#include "phymesh.h"
#include "solver.h"
#include "projective_dynamics.h"

#include <filesystem>
#include <random>



int main()
{
	TinyRenderer renderer;
	renderer.init();

	// some data
	Entity_ptr sphere = TinyRenderer::get_entity("sphere_tet_entity");
	sphere->centerlize_vert(); // deliminate transform
	std::vector<float> vdata_ori = sphere->vdata_c();
	//std::vector<float> ndata_ori = sphere->ndata_c();

	// Physics
	// model->phymesh
	PhyMesh_uptr sphere_phymesh = std::make_unique<PhyMesh>(
		sphere->vdata_c(), sphere->vdata_c().size() / 3,
		SimPD::g, SimPD::m,
		sphere->tetdata_c(), sphere->tetdata_c().size() / 4);
	sphere_phymesh->setup_constraint_PD(SimPD::sigma_min, SimPD::sigma_max, SimPD::k);
	// solver
	PD_solver solver(SimPD::dt, sphere_phymesh.get());
	solver.init();

	std::function<void()> reset = [&sphere, &vdata_ori, &sphere_phymesh, &solver]() {
		if (!ui_flags.reset) {
			return;
		}
		SimPD::UI_to_params();
		// reset entity
		sphere->vdata() = vdata_ori;
		//sphere->ndata() = ndata_ori;
		auto gizmo = get_bound_gizmo(sphere);
		if (gizmo) gizmo->odata() = vdata_ori;

		// renew phymesh of entity
		sphere_phymesh = std::make_unique<PhyMesh>(
			sphere->vdata_c(), sphere->vdata_c().size() / 3,
			SimPD::g, SimPD::m,
			sphere->tetdata_c(), sphere->tetdata_c().size() / 4);
		sphere_phymesh->setup_constraint_PD(SimPD::sigma_min, SimPD::sigma_max, SimPD::k);
		// reset solver
		solver.reset(SimPD::dt, sphere_phymesh.get());

		ui_flags.pause = true;
		ui_flags.reset = false;
	};

	std::function<void()> physics_tick = [&solver, &sphere, &sphere_phymesh]() {
		if (ui_flags.pause) return;
		solver.step(SimPD::iter_num);
		update_ent_mesh_vert(sphere, sphere_phymesh->position.data(), sphere_phymesh->position.size());
	};


	renderer.register_ui_layout_update(UI_layout_update);
	renderer.register_reset(reset);
	renderer.register_physics_tick(physics_tick);

	renderer.loop();

	renderer.terminate();

	//while (render_next(window_global)) {
	//	// app input process
	//	controller.process_input();

	//	// prerender UI
	//	UI_layout_update();

	//	// physics tick
	//	if (!ui_flags.pause) {
	//		solver.step(SimPD::iter_num);
	//		update_ent_mesh_vert(sphere, sphere_phymesh->position.data(), sphere_phymesh->position.size());
	//	}

	//	// render tick
	//	render_pipeline->render(scene_uuid);

	//	// render UI
	//	render_ui();

	//	// if reset
	//	if (ui_flags.reset) {
	//		SimPD::UI_to_params();

	//		// reset entity
	//		sphere->vdata() = vdata_ori;
	//		//sphere->ndata() = ndata_ori;
	//		auto gizmo = get_bound_gizmo(sphere);
	//		if (gizmo) gizmo->odata() = vdata_ori;

	//		// renew phymesh of entity
	//		sphere_phymesh = std::make_unique<PhyMesh>(
	//			sphere->vdata_c(), sphere->vdata_c().size() / 3,
	//			SimPD::g, SimPD::m,
	//			sphere->tetdata_c(), sphere->tetdata_c().size() / 4);
	//		sphere_phymesh->setup_constraint_PD(SimPD::sigma_min, SimPD::sigma_max, SimPD::k);
	//		// reset solver
	//		solver.reset(SimPD::dt, sphere_phymesh.get());

	//		ui_flags.pause = true;
	//		ui_flags.reset = false;
	//	}
	//}

	//render_terminate(window_global);

	return 0;
}
