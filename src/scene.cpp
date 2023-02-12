#include "utils_scene.h"
#include "scene.h"

#include <iostream>
#include <string>


Scene::Scene(const std::string& window_name, unsigned int particle_num,
    unsigned int scr_width, unsigned int scr_height,
    bool cuda_gl_interop, cudaGraphicsResource_t* cu_offsetBO_handle_ptr) {

    SCR_HEIGHT = scr_height;
    SCR_WIDTH = scr_width;
    this->name = window_name;
    window = render_init(SCR_WIDTH, SCR_HEIGHT, window_name.c_str());
    this->particle_size = particle_num;

    shaderProgram = load_shader("../shader/simple.vert", "../shader/simple.frag");


    // about sphere, maybe we can do a hierarchy
    // we also want these be moved to set_particles(first call)

    float vertices_buffer[] = {
         1.0f,  1.0f,  // top right
         1.0f, -1.0f,  // bottom right
        -1.0f, -1.0f,  // bottom left
        -1.0f,  1.0f   // top left 
    };


    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceOffsetBO);
    glGenBuffers(1, &instanceColorBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_buffer), vertices_buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
   
    glBindBuffer(GL_ARRAY_BUFFER, instanceOffsetBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * particle_num, NULL, GL_DYNAMIC_COPY);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

    if (cuda_gl_interop) {
        if (cu_offsetBO_handle_ptr != nullptr) {
            checkCudaErrors(cudaGraphicsGLRegisterBuffer(
                cu_offsetBO_handle_ptr, this->instanceOffsetBO, 
                cudaGraphicsRegisterFlagsNone));
        }
        else {
            printf("ERROR in Scene init\n");
            exit(-1);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceColorBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * particle_num, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Scene::~Scene() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
}

void Scene::render() {

    processInput(window); // shall be moved
    // view_mat = camera.get_view()
    // proj_mat = camera.get_proj()
    // use_shader(shader)
    // setMat4_shader(view_mat)
    // setMat4_shader(proj_mat)

    // change shader

    glClearColor(background_color.x, background_color.y, background_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    use_shader(shaderProgram);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particle_size);
    glfwSwapBuffers(window);
    glfwPollEvents();

    // window
    display_fps();
}

void Scene::set_particles(bool use_gl_cuda_interop, float* data, unsigned int length, float radius, float* color_buffer) {
    if (use_gl_cuda_interop) {
        _cu_set_particles(length, radius, color_buffer);
    }
    else {
        _set_particles(data, length, radius, color_buffer);
    }
}

void Scene::_set_particles(glm::vec2* data, unsigned int length, float radius, glm::vec3* color_buffer) {
    float scale = radius / (float)(this->SCR_WIDTH) * 2;

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    updateBuffer(instanceOffsetBO, 0, data, length * sizeof(glm::vec2), GL_ARRAY_BUFFER);
    updateBuffer(instanceColorBO, 0, color_buffer, length * sizeof(glm::vec3), GL_ARRAY_BUFFER);
    use_shader(shaderProgram);
    setFloat_shader(shaderProgram, "scale", scale);
}

void Scene::_cu_set_particles(unsigned int length, float radius, glm::vec3* color_buffer) {
    float scale = radius / (float)(this->SCR_WIDTH) * 2;

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    updateBuffer(instanceColorBO, 0, color_buffer, length * sizeof(glm::vec3), GL_ARRAY_BUFFER);

    use_shader(shaderProgram);
    setFloat_shader(shaderProgram, "scale", scale);
}

void Scene::_cu_set_particles(unsigned int length, float radius, float* color_buffer) {
    _cu_set_particles(length, radius, (glm::vec3*)color_buffer);
}

void Scene::_set_particles(float* data, unsigned int length, float radius, float* color_buffer) {
    _set_particles((glm::vec2*)data, length, radius, (glm::vec3*)color_buffer);
}

void Scene::set_background_color(glm::vec3 color) {
    this->background_color = color;
}

bool Scene::should_quit() {
    return glfwWindowShouldClose(window);
}

// time
#include <chrono>
static float _get_FPS() {
    static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);
    oldTime = std::chrono::high_resolution_clock::now();
    float fps = 1e6f / interval.count();
    return fps;
}

void Scene::display_fps() {
    static std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::high_resolution_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - oldTime);

    float fps = _get_FPS();
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