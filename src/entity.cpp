#include "entity.h"
#include "resource_manager.h"


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