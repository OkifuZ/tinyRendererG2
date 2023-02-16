#pragma once
#include "UI_layout.h"
#include "phymesh.h"
#include "solver.h"
#include "projective_dynamics.h"


class TinyPhyxSole {

public:

	PhyMesh_uptr entity_phymesh = nullptr;
	Entity_ptr entity = nullptr;
	std::vector<float> vdata_ori;

	// PD
	PD_solver_uptr pd_solver;

	void use_PD() {
		entity->centerlize_vert(); // deliminate transform
		vdata_ori = entity->vdata_c();
		//std::vector<float> ndata_ori = sphere->ndata_c();

		// Physics
		// model->phymesh
		entity_phymesh = std::move(std::make_unique<PhyMesh>(
			entity->vdata_c(), entity->vdata_c().size() / 3,
			PD_param::g, PD_param::m,
			entity->tetdata_c(), entity->tetdata_c().size() / 4));

		entity_phymesh->setup_constraint(PD_param::sigma_min, PD_param::sigma_max, PD_param::k);

		// solver
		pd_solver = std::move(std::make_unique<PD_solver>(PD_param::dt, entity_phymesh.get()));
		pd_solver->prefactor();
	}

	std::function<void()> get_reset_foo() {
		return [this]() {
			if (!ui_flags.reset) {
				return;
			}
			PD_param::UI_to_params();
			// reset entity
			entity->vdata() = vdata_ori;
			//sphere->ndata() = ndata_ori;
			auto gizmo = get_bound_gizmo(entity);
			if (gizmo) gizmo->odata() = vdata_ori;

			// renew phymesh of entity
			entity_phymesh = std::make_unique<PhyMesh>(
				entity->vdata_c(), entity->vdata_c().size() / 3,
				PD_param::g, PD_param::m,
				entity->tetdata_c(), entity->tetdata_c().size() / 4);
			entity_phymesh->setup_constraint(PD_param::sigma_min, PD_param::sigma_max, PD_param::k);
			// reset solver
			pd_solver->reset(PD_param::dt, entity_phymesh.get());

			ui_flags.pause = true;
			ui_flags.reset = false;
		};
	}

	std::function<void()> get_physics_tick_foo() {
		return  [this]() {
			if (ui_flags.pause) return;
			pd_solver->step(PD_param::iter_num);
			update_ent_mesh_vert(entity, entity_phymesh->position.data(), entity_phymesh->position.size());
		};
	}

};
