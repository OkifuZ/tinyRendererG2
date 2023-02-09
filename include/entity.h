#pragma once
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "snowflake.h"

#include <vector>
#include <memory>

class Entity;
typedef std::shared_ptr<Entity> Entity_ptr;
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

    // single mesh
    Entity() {}

    Entity(const std::string& name, Snowflake_type mesh_uuid, Snowflake_type material_uuid, Transform& transform) :
    name(name), mesh_uuid(mesh_uuid), material_uuid(material_uuid), transform(transform) {}

    ~Entity() {
        printf("Entity[%s] deleted...\n", name.c_str());
    }

    void centerlize();

};

