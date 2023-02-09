#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cuda_gl_interop.h>
#include <glm/glm.hpp>

#include "mesh.h"

#include "cu_hack.h"

#include <string>
#include <map>

class Scene3D {
    unsigned int SCR_WIDTH = 1200;
    unsigned int SCR_HEIGHT = 900;
    std::string name;

    unsigned int shaderProgram = -1;

    std::map<std::string, MeshObject_ptr> meshes;

    

    glm::vec3 background_color{ 0.2f, 0.3f, 0.3f };


    void _set_particles(glm::vec2* data, unsigned int length, float radius, glm::vec3* color_buffer);
    void _set_particles(float* data, unsigned int length, float radius, float* color_buffer);
    void _cu_set_particles(unsigned int length, float radius, glm::vec3* color_buffer);
    void _cu_set_particles(unsigned int length, float radius, float* color_buffer);


public:

    void display_fps();
    GLFWwindow* window = nullptr;
    // settings
    Scene3D(const std::string& window_name, unsigned int scr_width = 1200, unsigned int scr_height = 900);

    /*Scene3D(const Scene3D&) = delete;
    Scene3D& operator=(const Scene3D&) = delete;*/

    ~Scene3D();

    void load_model(const std::string& unique_name, const std::string& obj_file_path);

    // draw circles
    void set_particles(
        bool use_gl_cuda_interop,
        float* data, unsigned int length,
        float radius, float* color_buffer);

    // background
    void set_background_color(glm::vec3 color);

    // render
    void render();

    bool should_quit();

};
