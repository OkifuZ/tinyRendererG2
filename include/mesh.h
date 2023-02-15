#pragma once

#include "rhi_buffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>
#include <memory>
#include <map>


struct MeshDataContainer;
typedef std::shared_ptr<MeshDataContainer> MeshDataContainer_ptr;
typedef std::shared_ptr<const MeshDataContainer> MeshDataContainer_const_ptr;
class MeshObject;
typedef std::shared_ptr<MeshObject> MeshObject_ptr;


struct MeshDataContainer {
    std::vector<float> verts;
    size_t verts_num = 0;
    std::vector<float> texts;
    size_t uv_num = 0;
    std::vector<float> norms;
    size_t norm_num = 0;
    std::vector<uint32_t> faces_vertID;
    std::vector<uint32_t> faces_textID;
    std::vector<uint32_t> faces_normID;
    size_t face_num = 0;
    std::vector<uint32_t> tets_vertID;
    size_t tet_num = 0;
    std::string name_in_objfile = "";


    bool verts_dirty = false;
    bool texts_dirty = false;
    bool norms_dirty = false;
    bool face_dirty = false;

    // this is not a good design, unsafe
    //VAO_ptr vao = nullptr;

    MeshDataContainer() {}

    MeshDataContainer(std::vector<float>& verts, std::vector<float>& texts, std::vector<float>& norms,
        std::vector<uint32_t>& faces_vertID, std::vector<uint32_t>& faces_textID, std::vector<uint32_t>& faces_normID, 
        std::vector<uint32_t>& tets_vertID,
        std::string& name_in_objfile);

    bool loaded() { return verts_num > 0 && face_num > 0 && verts.size() == verts_num * 3; }

    ~MeshDataContainer() {
        printf("MeshDataContainer [%s] deleted...\n", name_in_objfile.c_str());
    }
};


bool save_mesh(MeshDataContainer_ptr mesh, const std::string& fpath);

void load_mesh(MeshDataContainer_ptr mesh, const std::string& fpath);

void rearrange_meshdata(MeshDataContainer_ptr);
