#pragma once
#include "simulation_type.h"



class StrainConstraint {
public:
	std::vector<Index_type> indices{}; // tet ijkl
	Scalar_type k{}; // weight
	Scalar_type V_0{}; // volume of tet in initial configuration
	Mat3_type Dm_inv{}; // 3x3 Mat of tet in initial configuration, F = D_t * Dm_inv; 
	Scalar_type sigma_min{}; // strain_limiting min
	Scalar_type sigma_max{}; // strain_limiting max

	StrainConstraint(
		std::initializer_list<Index_type> indices, Scalar_type k, 
		const Vector_type& pos,
		Scalar_type sigma_min, Scalar_type sigma_max
	) : indices(indices), k(k), sigma_min(sigma_min), sigma_max(sigma_max)
	{
		if (indices.size() != 4) {
			printf("[StarinConstraint] indice size error\n");
			exit(-1);
		}
		
		auto v1 = this->indices.at(0);
		auto v2 = this->indices.at(1);
		auto v3 = this->indices.at(2);
		auto v4 = this->indices.at(3);

		auto p1 = pos.block(v1 * 3, 0, 3, 1);
		auto p2 = pos.block(v2 * 3, 0, 3, 1);
		auto p3 = pos.block(v3 * 3, 0, 3, 1);
		auto p4 = pos.block(v4 * 3, 0, 3, 1);

		Mat3_type Dm;
		Dm.col(0) = p1 - p4;
		Dm.col(1) = p2 - p4;
		Dm.col(2) = p3 - p4;

		this->V_0 = (1.0f / 6.0f) * Dm.determinant();
		this->Dm_inv = Dm.inverse();
	}

	std::vector<Triplet_type> get_v_k_G_GT(const Vector_type& p) {}

	void get_v_k_G_ST_p(const Vector_type& q, Vector_type& b) {}

};