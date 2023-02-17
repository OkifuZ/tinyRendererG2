#pragma once
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "snowflake.h"

#include <vector>
#include <memory>

class Entity;
typedef std::shared_ptr<Entity> Entity_ptr;
typedef std::shared_ptr<const Entity> Entity_const_ptr;
class InstanceData;
typedef std::shared_ptr<InstanceData> InstanceData_ptr;

class InstanceData {
public:
    std::vector<float> offset;
    std::vector<float> color;
    std::vector<float> scale;
    
    bool offset_dirty = false;
    bool color_dirty = false;
    bool scale_dirty = false;

    std::string bound_entity = "";

    size_t instance_num = 0;
    bool used() { return !offset.empty() || !color.empty() || !scale.empty(); }
};



class Entity {
public:
    Snowflake_type mesh_uuid = 0L;
    Snowflake_type material_uuid = 0L;
    InstanceData instance_data;

    std::string name = "";
    Transform transform;

    bool wireframe = false;
    float linewidth = 3.0f;
    bool cullface = true;

    // single mesh
    Entity() {}

    Entity(const std::string& name, Snowflake_type mesh_uuid, Snowflake_type material_uuid, Transform& transform) :
    name(name), mesh_uuid(mesh_uuid), material_uuid(material_uuid), transform(transform) {}

    ~Entity() {
        printf("Entity[%s] deleted...\n", name.c_str());
    }

    void centerlize_transform();
    glm::vec3 get_center() const;
    // for simulation
    void centerlize_vert(); 

    std::vector<float>& vdata();
    std::vector<float>& ndata();
    std::vector<float>& odata(); // instacne.offset

    const std::vector<float>& vdata_c() const;
    const std::vector<float>& ndata_c() const;
    const std::vector<float>& odata_c() const;
    const std::vector<uint32_t>& edgedata_c() const;
    const std::vector<uint32_t>& tetdata_c() const;
};

std::vector<float> transformed_data(Entity_const_ptr entity);

Entity_ptr get_bound_gizmo(Entity_const_ptr entity);
Entity_ptr get_bound_entity_for_gizmo(Entity_const_ptr gizmo);

void update_ent_mesh_vert(Entity_ptr& ent, float* data, size_t size);