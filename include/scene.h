#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include "helper_cuda.h"

#include <string>

class Scene {
    unsigned int SCR_WIDTH = 800;
    unsigned int SCR_HEIGHT = 800;
    std::string name;

    GLFWwindow* window = nullptr;
    unsigned int shaderProgram = -1;

    unsigned int VAO = -1, VBO = -1, EBO = -1, instanceOffsetBO = -1, instanceColorBO = -1;
    unsigned int particle_size = -1;

    glm::vec3* single_color_buffer = nullptr;
    glm::vec3 background_color{ 0.2f, 0.3f, 0.3f };

    void display_fps();

    void _set_particles(glm::vec2* data, unsigned int length, float radius, glm::vec3* color_buffer);
    void _cu_set_particles(unsigned int length, float radius, glm::vec3* color_buffer);
    void _cu_set_particles(unsigned int length, float radius, float* color_buffer);
    void _set_particles(float* data, unsigned int length, float radius, float* color_buffer);

    
public:
    // settings
    Scene(const std::string& window_name, unsigned int particle_num, 
        unsigned int scr_width = 800, unsigned int scr_height = 800,
        bool cuda_gl_interop = false, cudaGraphicsResource_t* cu_offsetBO_handle_ptr = nullptr);
    
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;


    ~Scene();

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
