#pragma once
#include "entity.h"
#include "UI_layout.h"
#include "phymesh.h"
#include "solver.h"
#include "projective_dynamics.h"

#include <memory>


class TinyPhyxSole;
using TinyPhyxSole_uptr = std::unique_ptr<TinyPhyxSole>;


class TinyPhyxSole {
public:

	PhyMesh_uptr entity_phymesh = nullptr;
	Entity_ptr entity = nullptr;
	std::vector<float> vdata_ori; // initial vertice data

	TinyPhyxSole() = default;
	TinyPhyxSole(const TinyPhyxSole&) = delete;
	TinyPhyxSole& operator=(const TinyPhyxSole&) = delete;

	virtual void use() = 0;
	virtual std::function<void()> get_reset_foo() = 0;
	virtual std::function<void()> get_physics_tick_foo() = 0;
};


class TinyPhyxSole_PD : public TinyPhyxSole {

public:

	// PD
	PD_solver_uptr pd_solver;

	void use() override {
		entity->centerlize_vert(); // deliminate transform
		vdata_ori = entity->vdata_c();
		//std::vector<float> ndata_ori = sphere->ndata_c();

		// Physics
		// model->phymesh
		entity_phymesh = std::move(std::make_unique<PhyMesh>(
			entity->vdata_c(), entity->vdata_c().size() / 3,
			entity->edgedata_c(), entity->edgedata_c().size() / 2,
			entity->tetdata_c(), entity->tetdata_c().size() / 4,
			SimPD::g, SimPD::m));

		entity_phymesh->setup_constraint_PD(SimPD::sigma_min, SimPD::sigma_max, SimPD::k);

		// solver
		pd_solver = std::move(std::make_unique<PD_solver>(SimPD::dt, entity_phymesh.get()));
		pd_solver->init();
	}

	std::function<void()> get_reset_foo() override {
		return [this]() {
			if (!ui_flags.reset) {
				return;
			}
			SimPD::UI_to_params();
			// reset entity
			entity->vdata() = vdata_ori;
			//sphere->ndata() = ndata_ori;
			auto gizmo = get_bound_gizmo(entity);
			if (gizmo) gizmo->odata() = vdata_ori;

			// renew phymesh of entity
			entity_phymesh = std::make_unique<PhyMesh>(
				entity->vdata_c(), entity->vdata_c().size() / 3,
				entity->edgedata_c(), entity->edgedata_c().size() / 2,
				entity->tetdata_c(), entity->tetdata_c().size() / 4,
				SimPD::g, SimPD::m);
			entity_phymesh->setup_constraint_PD(SimPD::sigma_min, SimPD::sigma_max, SimPD::k);
			// reset solver
			pd_solver->reset(SimPD::dt, entity_phymesh.get());

			ui_flags.pause = true;
			ui_flags.reset = false;
		};
	}

	std::function<void()> get_physics_tick_foo() override {
		return  [this]() {
			if (ui_flags.pause) return;
			pd_solver->step(SimPD::iter_num);
			update_ent_mesh_vert(entity, entity_phymesh->position.data(), entity_phymesh->position.size());
		};
	}

};


class TinyPhyxSole_PBD : public TinyPhyxSole {

public:

	// PD
	PBD_solver_uptr pbd_solver;

	void use() override {
		entity->centerlize_vert(); // deliminate transform
		vdata_ori = entity->vdata_c();
		//std::vector<float> ndata_ori = sphere->ndata_c();

		// Physics
		// model->phymesh
		entity_phymesh = std::move(std::make_unique<PhyMesh>(
			entity->vdata_c(), entity->vdata_c().size() / 3,
			entity->edgedata_c(), entity->edgedata_c().size() / 2,
			entity->tetdata_c(), entity->tetdata_c().size() / 4,
			SimPBD::g, SimPBD::m));

		entity_phymesh->setup_constraint_PBD(SimPBD::stiff);

		// solver
		pbd_solver = std::move(std::make_unique<PBD_solver>(SimPBD::dt, SimPBD::substep_num, entity_phymesh.get()));
		pbd_solver->init();
	}

	std::function<void()> get_reset_foo() override {
		return [this]() {
			if (!ui_flags.reset) {
				return;
			}

			SimPBD::UI_to_params();
			// reset entity
			entity->vdata() = vdata_ori;
			//sphere->ndata() = ndata_ori;
			auto gizmo = get_bound_gizmo(entity);
			if (gizmo) gizmo->odata() = vdata_ori;

			// renew phymesh of entity
			entity_phymesh = std::make_unique<PhyMesh>(
				entity->vdata_c(), entity->vdata_c().size() / 3,
				entity->edgedata_c(), entity->edgedata_c().size() / 2,
				entity->tetdata_c(), entity->tetdata_c().size() / 4,
				SimPBD::g, SimPBD::m);
			entity_phymesh->setup_constraint_PBD(SimPBD::stiff);
			// reset solver
			pbd_solver->reset(SimPBD::dt, entity_phymesh.get());


			ui_flags.pause = true;
			ui_flags.reset = false;
		};
	}

	std::function<void()> get_physics_tick_foo() override {
		return  [this]() {
			if (ui_flags.pause) return;
			pbd_solver->step();
			update_ent_mesh_vert(entity, entity_phymesh->position.data(), entity_phymesh->position.size());
		};
	}

};
