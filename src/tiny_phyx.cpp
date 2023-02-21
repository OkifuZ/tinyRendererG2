#include "tiny_phyx.h"

#include <Eigen/Core>
#include <limits>

void TinyPhyxSole::register_entity(Entity_ptr entity) {
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

		/*ui_flags.pause = true;
		ui_flags.reset = false;*/
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



void TinyPhyxSole_PBD::use() {
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
		SimPBD::g, SimPBD::m));

	entity_phymesh->setup_constraint_PBD(SimPBD::stiff);

	// solver
	pbd_solver = std::move(std::make_unique<PBD_solver>(SimPBD::dt, SimPBD::substep_num, entity_phymesh.get()));
	pbd_solver->init();
}

std::function<void()> TinyPhyxSole_PBD::get_reset_foo() {
	return [this]() {
		if (!entity || !entity_phymesh) return;
		/*if (!ui_flags.reset) {
			return;
		}*/

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


		/*ui_flags.pause = true;
		ui_flags.reset = false;*/
	};
}

std::function<void()> TinyPhyxSole_PBD::get_physics_tick_foo() {
	return  [this]() {
		if (paused) return;
		pbd_solver->step();
		update_ent_mesh_vert(entity, entity_phymesh->position.data(), entity_phymesh->position.size());
	};
}


void TinyPhyxSole_PBD::start_grab(const glm::vec3& intersect_pos) {
	Vector_type& verts  = this->entity_phymesh->position;
	Size_type N = this->entity_phymesh->vert_size;

	Scalar_type dis_min = std::numeric_limits<Scalar_type>::max();
	Index_type closest_vert_id = -1;

	const Vec3_type intersect_v{ intersect_pos.x, intersect_pos.y, intersect_pos.z };
	Vec3_type v;
	for (Index_type i = 0; i < N; i++) {
		v = verts.block<3, 1>(i * 3, 0);
		Scalar_type dis = (v - intersect_v).norm();
		if (dis < dis_min) {
			dis_min = dis;
			closest_vert_id = i;
		}
	}

	if (closest_vert_id >= 0) {
		verts.block<3, 1>(closest_vert_id * 3, 0) = intersect_v;

		this->grabbed_id = closest_vert_id;
		this->grabbed_mass = this->entity_phymesh->mass[closest_vert_id];

		this->entity_phymesh->mass[closest_vert_id] = std::numeric_limits<Scalar_type>::max();
		this->entity_phymesh->inv_mass[closest_vert_id] = 0.0f;

		Vec3_type closest_v = verts.block<3, 1>(closest_vert_id * 3, 0);
		printf("closest point: %.3f, %.3f, %.3f; %d\n", closest_v.x(), closest_v.y(), closest_v.z(), closest_vert_id);
	}
}

void TinyPhyxSole_PBD::move_grab(const glm::vec3& pos) {
	if (grabbed_id == -1) return;
	Vector_type& verts = this->entity_phymesh->position;
	verts.block<3, 1>(grabbed_id * 3, 0) = Eigen::Vector3f{ pos.x, pos.y, pos.z };
}

void TinyPhyxSole_PBD::end_grab(const glm::vec3& pos) {
	if (grabbed_id == -1) return;
	this->entity_phymesh->mass[this->grabbed_id] = this->grabbed_mass;
	this->entity_phymesh->inv_mass[this->grabbed_id] = 1.0f / this->grabbed_mass;
	this->grabbed_id = -1;
}