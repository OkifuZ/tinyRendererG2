#include "entity.h"
#include "resource_manager.h"

#include <glm/glm.hpp>


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

Entity_ptr get_bound_entity_for_gizmo(Entity_const_ptr gizmo) {
    return resource_manager_global.get_entity_by_name(gizmo->instance_data.bound_entity);
}

void Entity::centerlize() {
    MeshDataContainer_const_ptr mesh = resource_manager_global.get_mesh_by_uuid(this->mesh_uuid);
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

