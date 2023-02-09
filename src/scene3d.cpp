#include "scene3d.h"
#include "utils_scene.h"
#include "scene.h"
#include "mesh.h"

#include <iostream>
#include <string>


Scene3D::Scene3D(const std::string& window_name, unsigned int scr_width, unsigned int scr_height) {

    SCR_HEIGHT = scr_height;
    SCR_WIDTH = scr_width;
    this->name = window_name;
    window = render_init(SCR_WIDTH, SCR_HEIGHT, window_name.c_str(), false);

    // TODO: add shader from outside
    shaderProgram = load_shader("../shader/simple.vert", "../shader/simple.frag");
    // about sphere, maybe we can do a hierarchy
    // we also want these be moved to set_particles(first call)


}

Scene3D::~Scene3D() {
    // TODO: make shader class better
    // glfwTerminate();
}

void Scene3D::load_model(const std::string& unique_name, const std::string& obj_file_path) {
    //auto obj_data = parse_obj_file(obj_file_path);

}

void Scene3D::render() {
    static bool first_call = true;
    if (first_call) {
        glfwShowWindow(window);
        first_call = false;
    }
    


    //processInput(window); // shall be moved
    // view_mat = camera.get_view()
    // proj_mat = camera.get_proj()
    // use_shader(shader)
    // setMat4_shader(view_mat)
    // setMat4_shader(proj_mat)

    // change shader

    // glClearColor(background_color.x, background_color.y, background_color.z, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);

    // use_shader(shaderProgram);
    /*glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle_size);*/
    // glfwSwapBuffers(window);
    
    // window
}

void Scene3D::set_particles(bool use_gl_cuda_interop, float* data, unsigned int length, float radius, float* color_buffer) {
    if (use_gl_cuda_interop) {
        _cu_set_particles(length, radius, color_buffer);
    }
    else {
        _set_particles(data, length, radius, color_buffer);
    }
}

void Scene3D::_set_particles(glm::vec2* data, unsigned int length, float radius, glm::vec3* color_buffer) {
    float scale = radius / (float)(this->SCR_WIDTH) * 2;

    /*glBindVertexArray(VAO);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    updateBuffer(instanceOffsetBO, 0, data, length * sizeof(glm::vec2), GL_ARRAY_BUFFER);
    updateBuffer(instanceColorBO, 0, color_buffer, length * sizeof(glm::vec3), GL_ARRAY_BUFFER);*/
    use_shader(shaderProgram);
    setFloat_shader(shaderProgram, "scale", scale);
}

void Scene3D::_cu_set_particles(unsigned int length, float radius, glm::vec3* color_buffer) {
    float scale = radius / (float)(this->SCR_WIDTH) * 2;

    /*glBindVertexArray(VAO);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    updateBuffer(instanceColorBO, 0, color_buffer, length * sizeof(glm::vec3), GL_ARRAY_BUFFER);*/

    use_shader(shaderProgram);
    setFloat_shader(shaderProgram, "scale", scale);
}

void Scene3D::_cu_set_particles(unsigned int length, float radius, float* color_buffer) {
    _cu_set_particles(length, radius, (glm::vec3*)color_buffer);
}

void Scene3D::_set_particles(float* data, unsigned int length, float radius, float* color_buffer) {
    _set_particles((glm::vec2*)data, length, radius, (glm::vec3*)color_buffer);
}

void Scene3D::set_background_color(glm::vec3 color) {
    this->background_color = color;
}

bool Scene3D::should_quit() {
    return glfwWindowShouldClose(window);
}



#include <chrono>
void Scene3D::display_fps() {
    static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();

    // TODO: need test
    static auto get_FPS = []() {
        static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);
        oldTime = std::chrono::high_resolution_clock::now();
        float fps = 1e6f / interval.count();
        return fps;
    };

    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);

    float fps = get_FPS();
    if (interval.count() > 1e6f) { // display fps every second
        char title[256];
        title[255] = '\0';
        snprintf(title, 255,
            "%s - [FPS: %3.2f]",
            this->name.c_str(), fps);
        glfwSetWindowTitle(this->window, title);

        oldTime = std::chrono::high_resolution_clock::now();
    }
}