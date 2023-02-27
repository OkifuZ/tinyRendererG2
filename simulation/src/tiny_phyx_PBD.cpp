#include "tiny_phyx.h"

#include <Eigen/Core>
#include <limits>
#include <random>


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

	if (ui_flags.pbd_.constraint_type == static_cast<int>(UI_Flags::PBD_CONSTRAINT_TYPE::Edge_Volume)) {
		entity_phymesh->setup_constraint_Edge_PBD(SimPBD::stiff);
		entity_phymesh->setup_constraint_Volume_PBD(SimPBD::stiff);
	}
	else if (ui_flags.pbd_.constraint_type == static_cast<int>(UI_Flags::PBD_CONSTRAINT_TYPE::Edge_Corotated)) {
		entity_phymesh->setup_constraint_Edge_PBD(SimPBD::stiff);
		entity_phymesh->setup_constraint_Corotated_PBD(SimPBD::stiff);
	}
	else if (ui_flags.pbd_.constraint_type == static_cast<int>(UI_Flags::PBD_CONSTRAINT_TYPE::ClothEdge)) {
		entity_phymesh->setup_structured_cloth_edges(ui_flags.pbd_.strcture_edge_width, ui_flags.pbd_.strcture_edge_height);
		entity_phymesh->setup_constraint_Structured_Cloth_Edge_PBD(SimPBD::stiff);
	}
	// solver
	pbd_solver = std::move(std::make_unique<PBD_solver>(SimPBD::dt, SimPBD::substep_num, entity_phymesh.get()));
	pbd_solver->init();
}

std::function<void()> TinyPhyxSole_PBD::get_reset_foo() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> dis(0, 1);//uniform distribution between 0 and 1

	return [this]() {
		if (!entity || !entity_phymesh) return;
		/*if (!ui_flags.reset) {
			return;
		}*/

		SimPBD::UI_to_params();

		// reset entity
		auto& verts = entity->vdata();
		verts = vdata_ori;
		auto gizmo = get_bound_gizmo(entity);
		if (gizmo) gizmo->odata() = verts;

		// renew phymesh of entity
		entity_phymesh = std::make_unique<PhyMesh>(
			verts, verts.size() / 3,
			entity->edgedata_c(), entity->edgedata_c().size() / 2,
			entity->tetdata_c(), entity->tetdata_c().size() / 4,
			SimPBD::g, SimPBD::m);

		if (ui_flags.pbd_.constraint_type == static_cast<int>(UI_Flags::PBD_CONSTRAINT_TYPE::Edge_Volume)) {
			entity_phymesh->setup_constraint_Edge_PBD(SimPBD::stiff);
			entity_phymesh->setup_constraint_Volume_PBD(SimPBD::stiff);
		}
		else if (ui_flags.pbd_.constraint_type == static_cast<int>(UI_Flags::PBD_CONSTRAINT_TYPE::Edge_Corotated)) {
			entity_phymesh->setup_constraint_Edge_PBD(SimPBD::stiff);
			entity_phymesh->setup_constraint_Corotated_PBD(SimPBD::stiff);
		}
		else {
			entity_phymesh->setup_constraint_Edge_PBD(SimPBD::stiff);
		}

		if (ui_flags.pbd_.need_squeeze) {
			for (int i = 0; i < verts.size(); i++) {
				verts[i] = dis(gen) * 1.0f;
			}
			entity_phymesh->setup_position(verts, verts.size() / 3);
			if (gizmo) gizmo->odata() = verts;
			ui_flags.pbd_.need_squeeze = false;
		}


		// reset solver
		pbd_solver->reset(SimPBD::dt, entity_phymesh.get());

		// choose point
		choosed_verts.clear();

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
	if (ui_flags.pause) return;

	VectorX_type& verts = this->entity_phymesh->position;
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
	if (ui_flags.pause) return;

	if (grabbed_id == -1) return;
	VectorX_type& verts = this->entity_phymesh->position;
	verts.block<3, 1>(grabbed_id * 3, 0) = Eigen::Vector3f{ pos.x, pos.y, pos.z };
}

void TinyPhyxSole_PBD::end_grab(const glm::vec3& pos) {
	if (ui_flags.pause) return;

	if (grabbed_id == -1) return;
	this->entity_phymesh->mass[this->grabbed_id] = this->grabbed_mass;
	this->entity_phymesh->inv_mass[this->grabbed_id] = 1.0f / this->grabbed_mass;
	this->grabbed_id = -1;
}

void TinyPhyxSole_PBD::choose_point(const glm::vec3& intersect_pos) {
	const float small_value = 1e-6f;
	if (!ui_flags.pause) return;

	VectorX_type& verts = this->entity_phymesh->position;
	Size_type N = this->entity_phymesh->vert_size;

	const Scalar_type choose_critera = 4e-2f;
	Index_type closest_vert_id = -1;

	const Vec3_type intersect_v{ intersect_pos.x, intersect_pos.y, intersect_pos.z };
	Vec3_type v;
	for (Index_type i = 0; i < N; i++) {
		v = verts.block<3, 1>(i * 3, 0);
		Scalar_type dis = (v - intersect_v).norm();
		if (dis < choose_critera) {
			closest_vert_id = i;
		}
	}

	if (closest_vert_id >= 0) {
		if (this->choosed_verts.count(closest_vert_id) <= 0) {
			this->choosed_verts[closest_vert_id] = this->entity_phymesh->mass[closest_vert_id];
			this->entity_phymesh->mass[closest_vert_id] = std::numeric_limits<Scalar_type>::max();
			this->entity_phymesh->inv_mass[closest_vert_id] = 0.0f;
		}
		else {
			Scalar_type old_mass = this->choosed_verts.at(closest_vert_id);
			this->entity_phymesh->mass[closest_vert_id] = old_mass;
			this->entity_phymesh->inv_mass[closest_vert_id] = 1.0f / old_mass;
			this->choosed_verts.erase(closest_vert_id);
		}

		Vec3_type closest_v = verts.block<3, 1>(closest_vert_id * 3, 0);
		printf("choose point: %.3f, %.3f, %.3f; %d\n", closest_v.x(), closest_v.y(), closest_v.z(), closest_vert_id);
	}
	// todo: reset this

}
