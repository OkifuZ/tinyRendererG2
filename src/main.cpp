#include "scene3d.h"
#include "mpm.h"
#include "ticktock.h"
#include "material.h"
#include "renderer.h"
#include "general_phong_rp.h"
#include "entity.h"
#include "resource_manager.h"
#include "rhi.h"
#include "fps.h"
#include "UI_layout.h"
#include "user_control.h"
#include "phymesh.h"
#include "solver.h"


#include <filesystem>
#include <random>

namespace PD_param {

float k{ 10.0f };
float sigma_min{ 0.95f }, sigma_max{ 1.05f };
float dt{ 0.02f };
int iter_num = 10;
float g = 0.5f;
float m = 1.0f;

}


int main()
{
	// runtime environment
	RHI_InitConfig rhi_cfg;
	rhi_init(window_global, rhi_cfg);
	init_resource_manager();
	auto scene_uuid = load_scene(ResourceManager::scene_path / "scene.json");
	ControllSystem controller;
	controller.register_camera(resource_manager_global.get_camera_by_name("camera"));
	// render
	RenderPipeline_ptr render_pipeline = std::make_shared<GeneralPhongRenderPipeline>();
	bool render_prepared = render_pipeline->prepare(scene_uuid);
	if (!render_prepared) {
		printf("Render pipeline preparation failed\n");
		exit(-1);
	}

	// some data
	Entity_ptr sphere = resource_manager_global.get_entity_by_name("sphere_tet_entity");
	sphere->centerlize_vert(); // deliminate transform
	std::vector<float> vdata_ori = sphere->vdata_c();
	//std::vector<float> ndata_ori = sphere->ndata_c();
	
	// Physics
	// model->phymesh
	PhyMesh_uptr sphere_phymesh = std::make_unique<PhyMesh>(
		sphere->vdata_c(), sphere->vdata_c().size() / 3,
		PD_param::g, PD_param::m,
		sphere->tetdata_c(), sphere->tetdata_c().size() / 4);
	sphere_phymesh->setup_constraint(PD_param::sigma_min, PD_param::sigma_max, PD_param::k);
	// solver
	PD_solver solver(PD_param::dt, sphere_phymesh.get());
	solver.prefactor();
	
	while (render_next(window_global)) {
		// app input process
		controller.process_input();

		// prerender UI
		UI_layout_update();

		// physics tick
		if (!ui_flags.pause) {
			//solver.mock_step();
			// TODO: ERROR-> Eigen assertion failed
			solver.step(PD_param::iter_num);
			update_ent_mesh_vert(sphere, sphere_phymesh->position.data(), sphere_phymesh->position.size());
			glm::vec3 center = sphere->get_center();
			printf("%.3f, %.3f, %.3f\n", center.x, center.y, center.z);
		}

		// render tick
		render_pipeline->render(scene_uuid);

		// render UI
		render_ui();

		// if reset
		if (ui_flags.reset) {
			PD_param::k		    = ui_flags.k;
			PD_param::g		    = ui_flags.g;
			PD_param::m		    = ui_flags.m;
			PD_param::iter_num  = ui_flags.iter_num;
			PD_param::dt	    = ui_flags.dt;
			PD_param::sigma_min = ui_flags.sigmas[0];
			PD_param::sigma_max = ui_flags.sigmas[1];

			// reset entity
			sphere->vdata() = vdata_ori;
			//sphere->ndata() = ndata_ori;
			auto gizmo = get_bound_gizmo(sphere);
			if (gizmo) gizmo->odata() = vdata_ori;

			// renew phymesh of entity
			sphere_phymesh = std::make_unique<PhyMesh>(
				sphere->vdata_c(), sphere->vdata_c().size() / 3,
				PD_param::g, PD_param::m,
				sphere->tetdata_c(), sphere->tetdata_c().size() / 4);
			sphere_phymesh->setup_constraint(PD_param::sigma_min, PD_param::sigma_max, PD_param::k);
			// reset solver
			solver.reset(PD_param::dt, sphere_phymesh.get());

			ui_flags.pause = true;
			ui_flags.reset = false;
		}
	}

	render_terminate(window_global);

	return 0;
}

