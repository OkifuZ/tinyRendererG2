#include "constraint_PBD.h"

SimPBD::EdgeConstraint::EdgeConstraint(std::initializer_list<Index_type> indices, std::initializer_list<Scalar_type> k, Scalar_type stiff,
	const Vector_type& pos)
	: Constraint(indices, k, stiff) {
	const auto& v1 = pos.block<3, 1>(this->indices[0] * 3, 0);
	const auto& v2 = pos.block<3, 1>(this->indices[1] * 3, 0);
	this->l_0 = (v1 - v2).norm();
}

void SimPBD::EdgeConstraint::resolve(Vector_type& position, Scalar_type dt) {
	Index_type v1_ind = this->indices[0];
	Index_type v2_ind = this->indices[1];
	auto& x1 = position.block<3, 1>(v1_ind * 3, 0);
	auto& x2 = position.block<3, 1>(v2_ind * 3, 0);
	Scalar_type l = (x1 - x2).norm();

	// C
	Scalar_type C = l - this->l_0;

	// gradient of C wrt v1&v2
	Vec3_type grad_C_x1 = (x1 - x2) / l;
	Vec3_type grad_C_x2 = (x2 - x1) / l;

	// lambda
	Scalar_type k1 = this->k[0];
	Scalar_type k2 = this->k[1];
	Scalar_type lambda = -C / (k1 + k2 + this->alpha / (dt * dt));

	// delta X & update X
	x1 += (lambda * k1 * grad_C_x1);
	x2 += (lambda * k2 * grad_C_x2);
}

SimPBD::TetVolumeConstraint::TetVolumeConstraint(std::initializer_list<Index_type> indices, std::initializer_list<Scalar_type> k, Scalar_type stiff,
	const Vector_type& pos)
	: Constraint(indices, k, stiff) {
	const Vec3_type& v1 = pos.block<3, 1>(this->indices[0] * 3, 0);
	const Vec3_type& v2 = pos.block<3, 1>(this->indices[1] * 3, 0);
	const Vec3_type& v3 = pos.block<3, 1>(this->indices[2] * 3, 0);
	const Vec3_type& v4 = pos.block<3, 1>(this->indices[3] * 3, 0);
	this->v_0 = (v2 - v1).cross(v3 - v1).dot(v4 - v1);
}

void SimPBD::TetVolumeConstraint::resolve(Vector_type& position, Scalar_type dt) {
	Index_type v1_ind = this->indices[0];
	Index_type v2_ind = this->indices[1];
	Index_type v3_ind = this->indices[2];
	Index_type v4_ind = this->indices[3];
	auto& v1 = position.block<3, 1>(v1_ind * 3, 0);
	auto& v2 = position.block<3, 1>(v2_ind * 3, 0);
	auto& v3 = position.block<3, 1>(v3_ind * 3, 0);
	auto& v4 = position.block<3, 1>(v4_ind * 3, 0);
	Scalar_type v = (v2 - v1).cross(v3 - v1).dot(v4 - v1);

	// C
	Scalar_type C = v - this->v_0;

	// gradient of C wrt v1&v2
	Vec3_type grad_C_x1 = (v4 - v2).cross(v3 - v2);
	Vec3_type grad_C_x2 = (v3 - v1).cross(v4 - v1);
	Vec3_type grad_C_x3 = (v4 - v1).cross(v2 - v1);
	Vec3_type grad_C_x4 = (v2 - v1).cross(v3 - v1);

	// lambda
	Scalar_type k1 = this->k[0];
	Scalar_type k2 = this->k[1];
	Scalar_type k3 = this->k[2];
	Scalar_type k4 = this->k[3];
	Scalar_type grad_C_x1_sq = grad_C_x1.dot(grad_C_x1);
	Scalar_type grad_C_x2_sq = grad_C_x2.dot(grad_C_x2);
	Scalar_type grad_C_x3_sq = grad_C_x3.dot(grad_C_x3);
	Scalar_type grad_C_x4_sq = grad_C_x4.dot(grad_C_x4);
	Scalar_type lambda = -C / 
		(k1 * grad_C_x1_sq + k2 * grad_C_x2_sq + k3 * grad_C_x3_sq + k4 * grad_C_x4_sq + 
			this->alpha / (dt * dt));

	// delta X & update X
	v1 += (lambda * k1 * grad_C_x1);
	v2 += (lambda * k2 * grad_C_x2);
	v3 += (lambda * k3 * grad_C_x3);
	v4 += (lambda * k4 * grad_C_x4);
}

