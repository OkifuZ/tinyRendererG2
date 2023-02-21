#pragma once

#include "simulation_type.h"
#include "Eigen/Dense"

namespace SimPBD {

class Constraint {
public:
	std::vector<Index_type> indices{};
	Scalar_type stiffness{};
	Scalar_type alpha{};

	const Scalar_type small_value = 1e-5f;

	Constraint(std::initializer_list<Index_type> indices, Scalar_type stiff) 
		: indices(indices), stiffness(stiff) {
		if (stiff > small_value) alpha = 1.0f / stiff;
		else alpha = 1e5;
	}
	virtual void resolve(Vector_type& position, const Vector_type& inv_mass, Scalar_type dt) = 0;
};


class EdgeConstraint : public Constraint {
public:

	Scalar_type l_0{};

	EdgeConstraint(std::initializer_list<Index_type> indices, Scalar_type stiff,
		const Vector_type& pos);

	void resolve(Vector_type& position, const Vector_type& inv_mass, Scalar_type dt) override;
};


class TetVolumeConstraint : public Constraint {
public:

	Scalar_type v_0{}; // 6 * V_0

	TetVolumeConstraint(std::initializer_list<Index_type> indices, Scalar_type stiff,
		const Vector_type& pos);

	void resolve(Vector_type& position, const Vector_type& inv_mass, Scalar_type dt) override;
};

}






