#pragma once

#include "simulation_type.h"
#include "constraint.h"
#include <vector>
#include <memory>

class PhyMesh;
typedef PhyMesh*                 PhyMesh_rptr;
typedef std::unique_ptr<PhyMesh> PhyMesh_uptr;
typedef std::shared_ptr<PhyMesh> PhyMesh_sptr;

class PhyMesh {
public:
	Size_type vert_size;
	Size_type ele_size;
	Size_type constraint_size;

	Vector_type position{}; // 3*N
	Elements4_type elements{}; // 3*M
	Vector_type f_ext{}; // 3*N
	Vector_type velocity{}; // 3*N
	Vector_type mass{}; // N

	std::vector<std::unique_ptr<StrainConstraint>> constraints;

	PhyMesh(const std::vector<float>& position, Size_type N, Scalar_type g, Scalar_type m, const std::vector<uint32_t>& elements, Size_type E) {
		this->mass.resize(N); this->mass.setZero();
		this->f_ext.resize(N * 3); this->f_ext.setZero();
		for (Size_type i = 0; i < N; i++) {
			this->f_ext.block<3, 1>(i * 3, 0) = Vec3_type{ 0, -g * m, 0 };
			this->mass[i] = m;
		}
		this->velocity.resize(N * 3); this->velocity.setZero();
		setup_position(position, N);
		setup_elements(elements, E);
	}

	void setup_position(const std::vector<float>& position, Size_type N) {
		this->position.resize(N * 3);
		Vec3_type pos;
		for (Size_type i = 0; i < N; i++) {
			pos = { position[i * 3 + 0],position[i * 3 + 1] , position[i * 3 + 2] };
			this->position.block<3, 1>(i * 3, 0) = pos; // can we make this one line?
		}
		this->vert_size = N;
	}

	void setup_elements(const std::vector<uint32_t>& elements, Size_type E) {
		for (Size_type i = 0; i < E; i++) {
			this->elements.push_back({
				elements[i * 4 + 0], elements[i * 4 + 1],
				elements[i * 4 + 2], elements[i * 4 + 3]});
		}
		this->ele_size = E;
	}

	void setup_constraint(Scalar_type sigma_min, Scalar_type sigma_max, Scalar_type k) {
		size_t v1{}, v2{}, v3{}, v4{};
		for (size_t i = 0; i < ele_size; i++) {
			auto [v1, v2, v3, v4] = this->elements[i];
			auto constraint = std::make_unique<StrainConstraint>(
				std::initializer_list<Index_type>{v1, v2, v3, v4}, 
				k, position, sigma_min, sigma_max);
			this->constraints.push_back(std::move(constraint));
			this->constraint_size++;
		}
	}

};