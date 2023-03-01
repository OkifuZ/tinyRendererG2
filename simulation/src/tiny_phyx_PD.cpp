#include "tiny_phyx.h"
#include "spatial_hash.h"

#include <Eigen/Core>
#include <limits>
#include <random>

void TinyPhyxSole::bind_entity(Entity_ptr entity) {
	this->entity = entity;
	entity->phy_object = this;
}

void TinyPhyxSole_PD::use() {
	if (!entity) return;
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

std::function<void()> TinyPhyxSole_PD::get_reset_foo() {
	// if we need polymorphic, we need std::bind instead of lambda capture
	return [this]() {
		if (!entity || !entity_phymesh) return;
		/*if (!ui_flags.reset) {
			return;
		}*/
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

	};
}

std::function<void()> TinyPhyxSole_PD::get_physics_tick_foo() {
	return  [this]() {
		if (!entity || !entity_phymesh) return;
		if (paused) return;
		pd_solver->step(SimPD::iter_num);
		update_ent_mesh_vert(entity, entity_phymesh->position.data(), entity_phymesh->position.size());
	};
}

void TinyPhyxSole_PD::start_grab(const glm::vec3& intersect_pos) {}

void TinyPhyxSole_PD::move_grab(const glm::vec3& pos) {}

void TinyPhyxSole_PD::end_grab(const glm::vec3& pos) {}

void TinyPhyxSole_PD::choose_point(const glm::vec3& intersect_pos) {}
