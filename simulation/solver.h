#pragma once
#include "simulation_type.h"
#include "phymesh.h"
#include "constraint.h"

#include <memory>

class PD_solver {
public:
	std::unique_ptr<PhyMesh> phymesh_ptr; // which type is better?
};