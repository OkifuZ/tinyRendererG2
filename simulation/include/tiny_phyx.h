#pragma once
#include "entity.h"
#include "UI_layout.h"
#include "phymesh.h"
#include "solver.h"
#include "projective_dynamics.h"

#include <memory>
#include <unordered_set>


class Entity;
typedef std::shared_ptr<Entity> Entity_ptr;
class TinyPhyxSole;
using TinyPhyxSole_uptr = std::unique_ptr<TinyPhyxSole>;


class TinyPhyxSole {
public:

	PhyMesh_uptr entity_phymesh = nullptr;
	Entity_ptr entity = nullptr;
	std::vector<float> vdata_ori; // initial vertice data

	bool paused = false;

	TinyPhyxSole() = default;
	TinyPhyxSole(const TinyPhyxSole&) = delete;
	TinyPhyxSole& operator=(const TinyPhyxSole&) = delete;

	virtual void use() = 0;
	virtual std::function<void()> get_reset_foo() = 0;
	virtual std::function<void()> get_physics_tick_foo() = 0;

	virtual void start_grab(const glm::vec3& intersect_pos) = 0;
	virtual void move_grab(const glm::vec3& pos) = 0;
	virtual void end_grab(const glm::vec3& pos) = 0;

	virtual void choose_point(const glm::vec3& intersect_pos) = 0;

	void bind_entity(Entity_ptr entity);
};


class TinyPhyxSole_PD : public TinyPhyxSole {

public:

	// PD
	PD_solver_uptr pd_solver;

	void use() override;

	std::function<void()> get_reset_foo() override;

	std::function<void()> get_physics_tick_foo() override;

	void start_grab(const glm::vec3& intersect_pos) override;
	virtual void move_grab(const glm::vec3& pos) override;
	virtual void end_grab(const glm::vec3& pos) override;
	virtual void choose_point(const glm::vec3& intersect_pos) override;

};


class TinyPhyxSole_PBD : public TinyPhyxSole {

public:

	// PD
	PBD_solver_uptr pbd_solver;

	void use() override;

	std::function<void()> get_reset_foo() override;

	std::function<void()> get_physics_tick_foo() override;



	void start_grab(const glm::vec3& intersect_pos) override;
	virtual void move_grab(const glm::vec3& pos) override;
	virtual void end_grab(const glm::vec3& pos) override;
	virtual void choose_point(const glm::vec3& intersect_pos) override;

private:
	int grabbed_id = -1;
	Scalar_type grabbed_mass{};
	std::unordered_map<int, Scalar_type> choosed_verts{}; // vertID -> vertMass

};
