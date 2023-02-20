#include "entity.h"
#include "resource_manager.h"

#include <glm/glm.hpp>

#include <unordered_set>
#include <limits>


std::vector<float> transformed_data(Entity_const_ptr entity) {
    auto& verts = entity->vdata_c();
    size_t num_verts = verts.size() / 3;

    std::vector<float> result;
    glm::vec3 pos{};
    glm::mat4 model = entity->transform.get_model_mat();
    for (size_t i = 0; i < num_verts; i++) {
        pos = { verts[i * 3 + 0], verts[i * 3 + 1], verts[i * 3 + 2] };
        pos = glm::vec3(model * glm::vec4(pos, 1.0f));
        result.insert(result.end(), {pos.x, pos.y, pos.z});
    }
    return result;
}

Entity_ptr get_bound_gizmo(Entity_const_ptr entity) {
    auto& gizmos = resource_manager_global.filter_entities([&entity](Entity_const_ptr e) {
        if (e->instance_data.bound_entity == entity->name) return true;
        else return false; });
    if (gizmos.empty()) return nullptr;
    return gizmos[0];
}

Entity_ptr get_bound_gizmo(const Entity* entity) {
    auto& gizmos = resource_manager_global.filter_entities([entity](Entity_const_ptr e) {
        if (e->instance_data.bound_entity == entity->name) return true;
        else return false; });
    if (gizmos.empty()) return nullptr;
    return gizmos[0];
}

Entity_ptr get_bound_entity_for_gizmo(Entity_const_ptr gizmo) {
    return resource_manager_global.get_entity_by_name(gizmo->instance_data.bound_entity);
}

void Entity::centerlize_transform() {
    MeshDataContainer_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) return;
    glm::vec3 sum_verts = glm::vec3(0, 0, 0);
    unsigned int num_verts = 0;
    auto& verts = mesh->verts;
    auto num = mesh->verts_num;
    for (int i = 0; i < num; i++) {
        sum_verts = sum_verts + glm::vec3{verts[i * 3 + 0], verts[i * 3 + 1], verts[i * 3 + 2]};
        num_verts++;
    }
    sum_verts = sum_verts / (float)num_verts;
    this->transform.center_offset = sum_verts;
}

glm::vec3 Entity::get_center() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) return glm::vec3{};
    glm::vec3 sum_verts = glm::vec3(0, 0, 0);
    unsigned int num_verts = 0;
    auto& verts = mesh->verts;
    auto num = mesh->verts_num;
    for (int i = 0; i < num; i++) {
        sum_verts = sum_verts + glm::vec3{ verts[i * 3 + 0], verts[i * 3 + 1], verts[i * 3 + 2] };
        num_verts++;
    }
    return sum_verts / (float)num_verts;
}

void Entity::centerlize_vert() {
    // 1. centerlize transform
    // 2. apply transform to all data
    MeshDataContainer_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) return;
    
    // this->centerlize_transform();
    glm::mat4 model = this->transform.get_model_mat();
    
    auto& verts = this->vdata();
    auto num = verts.size() / 3;
    for (int i = 0; i < num; i++) {
        auto new_vert = model * glm::vec4{ verts[i * 3 + 0], verts[i * 3 + 1], verts[i * 3 + 2], 1.0f };
        verts[i * 3 + 0] = new_vert.x;
        verts[i * 3 + 1] = new_vert.y;
        verts[i * 3 + 2] = new_vert.z;
    }
    this->transform = Transform{};

    auto gizmo = get_bound_gizmo(this);
    if (gizmo) gizmo->odata() = verts;
}

std::vector<float>& Entity::vdata() {
    MeshDataContainer_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) { 
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1); 
    }
    mesh->verts_dirty = true;
    return mesh->verts;
}

std::vector<float>& Entity::ndata() {
    MeshDataContainer_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    mesh->norms_dirty = true;
    return mesh->norms;
}

std::vector<float>& Entity::odata() { // instacne.offset 
    this->instance_data.offset_dirty = true;
    return this->instance_data.offset;
}

const std::vector<float>& Entity::vdata_c() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    return mesh->verts;
}

const std::vector<float>& Entity::ndata_c() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    return mesh->norms;
}

const std::vector<float>& Entity::odata_c() const {
    return this->instance_data.offset;
}

const std::vector<uint32_t>& Entity::tetdata_c() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    return mesh->tets_vertID;
}

const std::vector<uint32_t>& Entity::facedata_c() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    return mesh->faces_vertID;
}

const std::vector<uint32_t>& Entity::edgedata_c() const {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
    if (!mesh) {
        printf("oops... I don;t wanna use std::optional\n");
        exit(-1);
    }
    return mesh->edges_vertID;
}

const AABB_data Entity::get_AABB_no_transform() const {
    const auto& verts = this->vdata_c();
    size_t N = verts.size() / 3;
    glm::vec3 min_pos{ std::numeric_limits<float>::max() };
    glm::vec3 max_pos{ std::numeric_limits<float>::min(), };
    glm::vec3 pos{};
    for (size_t i = 0; i < N; i++) {
        pos = { verts[i * 3 + 0],verts[i * 3 + 1] ,verts[i * 3 + 2] };
        if (min_pos.x > pos.x) min_pos.x = pos.x;
        if (min_pos.y > pos.y) min_pos.y = pos.y;
        if (min_pos.z > pos.z) min_pos.z = pos.z;
        if (max_pos.x < pos.x) max_pos.x = pos.x;
        if (max_pos.y < pos.y) max_pos.y = pos.y;
        if (max_pos.z < pos.z) max_pos.z = pos.z;
    }
    return AABB_data{min_pos, max_pos};
}

void update_ent_mesh_vert(Entity_ptr& ent, float* data, size_t size) {
    auto& verts = ent->vdata();
    ent->vdata() = std::vector<float>(data, data + size);
    auto gizmo = get_bound_gizmo(ent);
    if (gizmo) gizmo->odata() = verts;
}


Entity_ptr _generate_lines_entity(std::vector<std::tuple<glm::vec3, glm::vec3>>& lines, const std::string& name) {
    Entity_ptr lines_entity = std::make_shared<Entity>();
    lines_entity->name = name;

    auto& lines_mesh = generate_lines_mesh(lines);
    Snowflake_type mesh_uuid = resource_manager_global.add_mesh(lines_mesh);
    lines_entity->mesh_uuid = mesh_uuid;
    
    RenderMaterial_ptr mat = std::make_shared<RenderMaterial>();
    load_material(*mat, (ResourceManager::material_path / "line.json").string() );
    Snowflake_type mat_uuid = resource_manager_global.add_material(mat);
    lines_entity->material_uuid = mat_uuid;

    return lines_entity;
}

Snowflake_type generate_lines_entity(std::vector<std::tuple<glm::vec3, glm::vec3>>& lines, const std::string& name) {
    auto& ent = _generate_lines_entity(lines, name);
    Snowflake_type ent_uuid = resource_manager_global.add_entity(ent);
    return ent_uuid;
}


