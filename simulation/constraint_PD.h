#pragma once
#include "simulation_type.h"
#include <Eigen/Dense>


namespace SimPD {

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
		const Vector_type& pos, Scalar_type sigma_min, Scalar_type sigma_max);

	std::vector<Triplet_type> get_v_k_G_GT(const Vector_type& p) const;

	void get_v_k_G_ST_p(const Vector_type& q, Vector_type& b) const;

};

}
