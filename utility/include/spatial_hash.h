#pragma once
#include <vector>
#include <functional>

#include <limits>
#include <memory>

class GridHash;
using GridHash_uptr = std::unique_ptr<GridHash>;


class GridHash {

	const float grid_size;
	size_t table_size;
	size_t num_particles;
	std::vector<int> grid_hashTable{};
	std::vector<int> particle_ids{};
	
	std::function<size_t(int xi, int yi, int zi)> hash_3int_foo;

	float min_x{};
	float min_y{};
	float min_z{};

	size_t hash_coord2tableID(int xi, int yi, int zi) {
		return this->hash_3int_foo(xi, yi, zi) % this->table_size;
	}

	/* this hash sucks
	void set_hash_foo_std() {
		auto hash_int = std::hash<int>{};
		this->hash_3int_foo = [hash_int](int xi, int yi, int zi) {
			size_t hx = hash_int(xi);
			size_t hy = hash_int(yi);
			size_t hz = hash_int(zi);
			return (hx ^ (hy << 1)) ^ (hz << 1);
		};
	}*/

	void set_hash_foo_fantasy() { 
		// this hash can not distinguish (x1, y1, z1), (x2, y2, z2) if 
		// |x1 * y1 * z1| == |x2 * y2 * z2|
		this->hash_3int_foo = [](int xi, int yi, int zi) {
			int h = (xi * 92837111) ^ (yi * 689287499) ^ (zi * 283923481);	// fantasy function
			return std::abs(h);
		};
	}

public:
	GridHash(float grid_size, size_t num_particles, float scale = 3.0f) : 
		grid_size(grid_size), table_size(size_t(scale * num_particles)), num_particles(num_particles) 
	{
		particle_ids.resize(this->num_particles, -1);
		grid_hashTable.resize(this->table_size + 1, 0);
		set_hash_foo_fantasy();
	}

	size_t hash_pos(float x, float y, float z) {
		return hash_coord2tableID(
			// make sure both xi & yi & zi > 0
			std::floor(x - this->min_x / this->grid_size),
			std::floor(y - this->min_y / this->grid_size),
			std::floor(z - this->min_z / this->grid_size)
		);
	}

	void construct(const float* verts_pos, size_t verts_size) {
		if (verts_size < this->num_particles) {
			printf("[Error] hashGrid construct() size miss match\n");
			return;
		}

		this->grid_hashTable.clear();
		this->particle_ids.clear();

		this->min_x = std::numeric_limits<float>::max();
		this->min_y = std::numeric_limits<float>::max();
		this->min_z = std::numeric_limits<float>::max();

		for (size_t i = 0; i < this->num_particles; i++) {
			float x = verts_pos[i * 3 + 0];
			float y = verts_pos[i * 3 + 1];
			float z = verts_pos[i * 3 + 2];
			this->min_x = std::min(x, this->min_x);
			this->min_y = std::min(y, this->min_y);
			this->min_z = std::min(z, this->min_z);
		}

		this->min_x -= 1e-2f;
		this->min_y -= 1e-2f;
		this->min_z -= 1e-2f;

		for (size_t i = 0; i < this->num_particles; i++) {
			float x = verts_pos[i * 3 + 0];
			float y = verts_pos[i * 3 + 1];
			float z = verts_pos[i * 3 + 2];
			size_t tab_id = hash_pos(verts_pos[i * 3 + 0], verts_pos[i * 3 + 1], verts_pos[i * 3 + 2]);
			this->grid_hashTable[tab_id]++;
		}

		int partial_sum = 0;
		for (size_t i = 0; i < this->grid_hashTable.size(); i++) {
			this->grid_hashTable[i] += partial_sum;
			partial_sum = this->grid_hashTable[i];
		}

		for (size_t i = 0; i < this->num_particles; i++) {
			size_t tab_id = hash_pos(verts_pos[i * 3 + 0], verts_pos[i * 3 + 1], verts_pos[i * 3 + 2]);
			size_t part_id = --(this->grid_hashTable[tab_id]);
			this->particle_ids[part_id] = i;
		}

	}

	
	std::vector<size_t> find_grid_vertIDs(float x, float y, float z) {

		size_t tab_id = hash_pos(x, y, z);
		size_t begin_id = this->grid_hashTable[tab_id];
		size_t end_id = this->grid_hashTable[tab_id + 1];
		auto it = this->particle_ids.begin();
		return { it + begin_id, it + end_id };
	}

	std::vector<size_t> find_range_vertIDs(float x, float y, float z, float max_range) {

		int x0 = std::floor(x - max_range);
		int y0 = std::floor(y - max_range);
		int z0 = std::floor(z - max_range);

		int x1 = std::floor(x + max_range);
		int y1 = std::floor(y + max_range);
		int z1 = std::floor(z + max_range);

		std::vector<size_t> ids{};

		for (int xi = x0; xi <= x1; xi++) 
		for (int yi = y0; yi <= y1; yi++) 
		for (int zi = z0; zi <= z1; zi++) {
			size_t tab_id = hash_coord2tableID(xi, yi, zi);
			size_t begin_id = this->grid_hashTable[tab_id];
			size_t end_id = this->grid_hashTable[tab_id + 1];
			auto it = this->particle_ids.begin();
			ids.insert(ids.end(), it + begin_id, it + end_id);
		}

		return ids;
	}

};