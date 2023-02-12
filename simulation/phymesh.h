#pragma once

#include "simulation_type.h"
#include "constraint.h"
#include <vector>
#include <memory>

class PhyMesh {
	Size_type vert_size;
	Size_type constrain_size;

	Vector_type position{};
	Elements_type elements{};
	Vector_type f_ext{};
	Vector_type velocity{};
	Vector_type masses{};
	Scalar_type dt{}, dt2{}, inv_dt{}, inv_dt2{};

	std::vector<std::unique_ptr<StrainConstraint>> constraints;


	void setup_constraint(Scalar_type sigma_min, Scalar_type sigma_max, Scalar_type k) {
		size_t v1{}, v2{}, v3{}, v4{};
		for (size_t i = 0; i < vert_size; i++) {
			v1 = elements[i * 4 + 0];
			v2 = elements[i * 4 + 1];
			v3 = elements[i * 4 + 2];
			v4 = elements[i * 4 + 3];
			auto constraint = std::make_unique<StrainConstraint>(
				std::initializer_list<Index_type>{v1, v2, v3, v4}, 
				k, position, sigma_min, sigma_max);
			this->constraints.push_back(std::move(constraint));
		}
	}
};