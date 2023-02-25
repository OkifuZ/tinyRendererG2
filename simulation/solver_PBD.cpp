#include "solver.h"

PBD_solver::PBD_solver(Scalar_type dt, int substep_num, PhyMesh_rptr phymesh_rptr) 
	: phymesh_ptr(phymesh_rptr), substep_num(substep_num) {
	this->set_dt(dt);
}

void PBD_solver::reset(Scalar_type dt, PhyMesh_rptr phymesh_rptr) {
	this->set_dt(dt);
	this->phymesh_ptr = phymesh_rptr;
	this->init();
}

void PBD_solver::set_dt(Scalar_type dt) {
	this->dt = dt;
	if (substep_num) this->dt_s = dt / substep_num;
	else this->dt_s = dt;
	this->dt_s2 = dt_s * dt_s;
	this->dt_inv = Scalar_type{ 1.0 } / dt;
	this->dt2 = dt * dt;
	this->dt2_inv = Scalar_type{ 1.0 } / (dt * dt);
}

void PBD_solver::init() {
	if (!phymesh_ptr || dt == 0) return;
	// seems nothing need to be done
}

void PBD_solver::step() {
	if (!phymesh_ptr || dt == 0) return;

	const auto& constraints = phymesh_ptr->constraints_PBD;
	auto&		position = phymesh_ptr->position;
	auto&		velocity = phymesh_ptr->velocity;
	const auto& mass = phymesh_ptr->mass;
	const auto& fext = phymesh_ptr->f_ext;
	Size_type N = phymesh_ptr->vert_size;

	VectorX_type y(N * 3); // new position
	for (int substep = 0; substep < this->substep_num; substep++) {
		// explicit integration
		for (size_t i = 0; i < N; i++) {
			auto&		y_i = y.block<3, 1>(i * 3, 0);
			const auto& p_i = position.block<3, 1>(i * 3, 0);
			const auto& v_i = velocity.block<3, 1>(i * 3, 0);
			const auto& f_i = fext.block<3, 1>(i * 3, 0);
			Scalar_type mass_i = mass[i];
			y_i = p_i + dt_s * v_i + dt_s * dt_s * f_i / mass_i;
			// printf("v%2d: (%.3f %.3f %.3f)\n", i, y_i.x(), y_i.y(), y_i.z());
		}

		// solve constraints
		// int i = 0;
		for (auto& constraint : constraints) {
			/*if (dynamic_cast<SimPBD::CorotatedConstraint*>(constraint.get())) {
				printf("[%d]\n", i++);
			}*/
			constraint->resolve(y, this->phymesh_ptr->inv_mass, this->dt_s);
		}

		// exit(0);

		// update velocity & position
		phymesh_ptr->velocity = (y - position) / dt_s;
		position = y;

		// dampen & collision & friction 
		// put this inside or outside substep-loop or out?
		for (size_t i = 0; i < N; i++) {
			auto& x = phymesh_ptr->position.block<3, 1>(i * 3, 0);
			auto& v = phymesh_ptr->velocity.block<3, 1>(i * 3, 0);
			if (x[1] < SimPBD::ground_height + small_value && v[1] < 0) { // collision with ground
				x[1] = SimPBD::ground_height + small_value;
				v[0] *= SimPBD::mu_t; v[1] *= -SimPD::mu_n; v[2] *= SimPD::mu_t;
			}
		}

	}
	phymesh_ptr->velocity *= SimPBD::dampen; // dampen

}

// for testing
void PBD_solver::mock_step() {

}