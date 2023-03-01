#pragma once
#include "simulation_type.h"
#include "spatial_hash.h"
#include "phymesh.h"
#include "memory"

class Collision_PBD;
using Collision_PBD_uptr = std::unique_ptr<Collision_PBD>;


class Collision_PBD {
public:

	float friction{};
	float radius{};
	float grid_size{};
	float hash_scale{5.0f};
	float search_radius{0.25f}; // TODO: derive this from max_velocity * dt
	// TODO: float max_velocity = r / dt; // avoid peneration

	std::vector<Scalar_type> rest_pos;

	PhyMesh_rptr phymesh = nullptr;
	GridHash_uptr grid_hash = nullptr;

	Collision_PBD() {}

	void init(PhyMesh_rptr phymesh, Scalar_type friction, Scalar_type radius, 
		Scalar_type grid_size, Scalar_type search_radius, Scalar_type hash_scale);

	void prepare();

	void handle();

};
