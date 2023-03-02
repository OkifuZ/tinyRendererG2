#pragma once
#include "simulation_type.h"
#include "phymesh.h"
#include "constraint_PD.h"
#include "constraint_PBD.h"
#include "util_physics.h"
#include "projective_dynamics.h"
#include "position_based_dynamics.h"
#include "collision_pbd.h"

#include <memory>
#include <iostream>

class PD_solver;
typedef std::unique_ptr<PD_solver> PD_solver_uptr;
class PBD_solver;
typedef std::unique_ptr<PBD_solver> PBD_solver_uptr;


class PD_solver {
private:
	std::unique_ptr<Eigen::SimplicialCholesky<SparseMat_type>> cholesky_decomp_ptr;
	Scalar_type dt{}, dt2{}, dt_inv{}, dt2_inv{};
	const Scalar_type small_value = 1e-4f;

public:

	PhyMesh_rptr phymesh_ptr = nullptr; // which type is better? 
	// I think it is raw pointer, PD_solver doesn't has phymesh_ptr, it "use" phymesh_ptr.
	// If type being unique_ptr, the user of PD_solver would has no elegant access to phymesh_ptr
	
	PD_solver(Scalar_type dt, PhyMesh_rptr phymesh_rptr);

	void reset_phymesh(Scalar_type dt, PhyMesh_rptr phymesh_rptr);

	void set_dt(Scalar_type dt);

	void init();

	void step(int iter_num);

	// for testing
	void mock_step();

};

class PBD_solver {
private:
	
	Scalar_type dt{}, dt2{}, dt_inv{}, dt2_inv{}, dt_s{}, dt_s2{};
	int substep_num{};
	const Scalar_type small_value = 1e-4f;

public:

	bool collision_hash_acc = true;
	Collision_PBD_rptr collision_pbd = nullptr;
	PhyMesh_rptr phymesh_ptr = nullptr; // which type is better? 
	// I think it is raw pointer, PD_solver doesn't has phymesh_ptr, it "use" phymesh_ptr.
	// If type being unique_ptr, the user of PD_solver would has no elegant access to phymesh_ptr

	PBD_solver(Scalar_type dt, int substep_num, PhyMesh_rptr phymesh_rptr);

	void reset_phymesh(Scalar_type dt, PhyMesh_rptr phymesh_rptr);

	void set_dt(Scalar_type dt);

	void init();

	void step();

	// for testing
	void mock_step();

};