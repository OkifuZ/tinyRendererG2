#include "collision_pbd.h"
#include "simulation_type.h"
#include "spatial_hash.h"
#include "phymesh.h"

void Collision_PBD::init(PhyMesh_rptr phymesh, Scalar_type friction, Scalar_type radius,
	Scalar_type grid_size, Scalar_type search_radius, Scalar_type hash_scale) {
	this->phymesh = phymesh;
	
	this->friction = friction;
	this->radius = radius;

	this->grid_size = grid_size;
	this->search_radius = search_radius;
	this->hash_scale = hash_scale;
	this->grid_hash = std::move(std::make_unique<GridHash>(this->grid_size, phymesh->vert_size, this->hash_scale));
	
	this->rest_pos = this->phymesh->position;
}


void Collision_PBD::prepare(bool use_hash) {
	if (!phymesh || !grid_hash) return;
	if (use_hash) this->grid_hash->construct(phymesh->position.data(), phymesh->vert_size);
	collision_ids.clear();
}

void Collision_PBD::handle_with_gridHash() {
	if (!phymesh || !grid_hash) return;

	auto& verts_pos = phymesh->position;
	bool has_coll = false;
	for (size_t i = 0; i < phymesh->vert_size; i++) {
		/*printf("%.4f, %.4f, %.4f\n", verts_pos[i * 3 + 0], verts_pos[i * 3 + 1], verts_pos[i * 3 + 2]);
		continue;*/

		if (phymesh->inv_mass(i) == 0.0f) continue;

		auto& pos_vi = verts_pos.block<3, 1>(i * 3, 0);
		auto near_ids = this->grid_hash->find_range_vertIDs(pos_vi(0), pos_vi(1), pos_vi(2), this->search_radius);
		
		for (auto& j : near_ids) {
			if (j >= i) continue;
			if (phymesh->inv_mass(j) == 0.0f) continue;

			auto& pos_vj = verts_pos.block<3, 1>(j * 3, 0);
			Scalar_type dist_ij = (pos_vi - pos_vj).norm();
			if (dist_ij > this->radius * 2 || dist_ij == 0.0f) continue;

			Scalar_type rest_length = (this->rest_pos.block<3, 1>(3 * i, 0) - this->rest_pos.block<3, 1>(3 * j, 0)).norm();

			if (dist_ij > rest_length) continue;

			const Scalar_type min_thickness = std::min(rest_length, this->radius * 2);

			Scalar_type scale_ij = (min_thickness - dist_ij) / dist_ij; // positive
			Vec3_type pos_corr_ij = (pos_vj - pos_vi) * scale_ij; // from i to j
			pos_vi += -0.5f * pos_corr_ij;
			pos_vj += 0.5f * pos_corr_ij;
			Vec3_type velo_i_x_dt = pos_vi - phymesh->position_prev.block<3, 1>(3 * i, 0);
			Vec3_type velo_j_x_dt = pos_vj - phymesh->position_prev.block<3, 1>(3 * j, 0);
			Vec3_type velo_x_dt_avg = 0.5f * (velo_i_x_dt + velo_j_x_dt);
			pos_vi += this->friction * (velo_x_dt_avg - velo_i_x_dt);
			pos_vj += this->friction * (velo_x_dt_avg - velo_j_x_dt);

			collision_ids.insert(i);
			collision_ids.insert(j);
			has_coll = true;
		}
	}
	// if (has_coll) printf("has collision\n");
}

void Collision_PBD::handle_with_bruteforce() {
	if (!phymesh) return;

	auto& verts_pos = phymesh->position;
	bool has_coll = false;
	for (size_t i = 0; i < phymesh->vert_size; i++) {
		if (phymesh->inv_mass(i) == 0.0f) continue;
		for (size_t j = i; j < phymesh->vert_size; j++) {
			if (phymesh->inv_mass(j) == 0.0f) continue;

			auto& pos_vi = verts_pos.block<3, 1>(i * 3, 0);
			auto& pos_vj = verts_pos.block<3, 1>(j * 3, 0);
			Scalar_type dist_ij = (pos_vi - pos_vj).norm();
			if (dist_ij > this->radius * 2 || dist_ij == 0.0f) continue;

			Scalar_type rest_length = (this->rest_pos.block<3, 1>(3 * i, 0) - this->rest_pos.block<3, 1>(3 * j, 0)).norm();

			if (dist_ij > rest_length) continue;

			const Scalar_type min_thickness = std::min(rest_length, this->radius * 2);
			//printf("(%d, %d) %.5f %.5f\n", i, j, rest_length, dist_ij);

			Scalar_type scale_ij = (min_thickness - dist_ij) / dist_ij; // positive
			Vec3_type pos_corr_ij = (pos_vj - pos_vi) * scale_ij; // from i to j
			pos_vi += -0.5f * pos_corr_ij;
			pos_vj += 0.5f * pos_corr_ij;
			Vec3_type velo_i_x_dt = pos_vi - phymesh->position_prev.block<3, 1>(3 * i, 0);
			Vec3_type velo_j_x_dt = pos_vj - phymesh->position_prev.block<3, 1>(3 * j, 0);
			Vec3_type velo_x_dt_avg = 0.5f * (velo_i_x_dt + velo_j_x_dt);
			pos_vi += this->friction * (velo_x_dt_avg - velo_i_x_dt);
			pos_vj += this->friction * (velo_x_dt_avg - velo_j_x_dt);
			collision_ids.insert(i);
			collision_ids.insert(j);
			has_coll = true;
		}
	}
}
