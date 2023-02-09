#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "utils_scene.h"

//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}



GLFWwindow* render_init(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT, const char* title, bool hide_window, glm::vec3 bgcolor) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, hide_window ? GLFW_FALSE : GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glClearColor(bgcolor.x, bgcolor.y, bgcolor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return window;
}


unsigned int load_shader(const std::string& vert_shader_path, const std::string& frag_shader_path) {

    auto read_file_as_str_lambda = [](const std::string& fna) -> std::string {
        std::ifstream t(fna);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    };

    const std::string vertexShaderSource = read_file_as_str_lambda(vert_shader_path);
    //std::cout << vertexShaderSource << std::endl;
    auto vertexShaderSource_cstr = vertexShaderSource.c_str();
    const std::string fragmentShaderSource = read_file_as_str_lambda(frag_shader_path);
    //std::cout << fragmentShaderSource << std::endl;
    auto fragmentShaderSource_cstr = fragmentShaderSource.c_str();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource_cstr, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource_cstr, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void use_shader(unsigned int shader_id) {
    glUseProgram(shader_id);
}

void setFloat_shader(unsigned int shader_id, const std::string& var_name, float value) {
    glUniform1f(glGetUniformLocation(shader_id, var_name.c_str()), value);
}
void setInt_shader(unsigned int shader_id, const std::string& var_name, int value)
{
    glUniform1i(glGetUniformLocation(shader_id, var_name.c_str()), value);
}
void setVec2_shader(unsigned int shader_id, const std::string& var_name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(shader_id, var_name.c_str()), 1, &value[0]);
}
void setVec2_shader(unsigned int shader_id, const std::string& var_name, float x, float y)
{
    glUniform2f(glGetUniformLocation(shader_id, var_name.c_str()), x, y);
}
void setVec3_shader(unsigned int shader_id, const std::string& var_name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(shader_id, var_name.c_str()), x, y, z);
}
void setVec3_shader(unsigned int shader_id, const std::string& var_name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(shader_id, var_name.c_str()), 1, &value[0]);
}
void setVec4_shader(unsigned int shader_id, const std::string& name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(shader_id, name.c_str()), 1, &value[0]);
}
void setVec4_shader(unsigned int shader_id, const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shader_id, name.c_str()), x, y, z, w);
}
void setMat2_shader(unsigned int shader_id, const std::string& name, const glm::mat2& mat)
{
    glUniformMatrix2fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void setMat3_shader(unsigned int shader_id, const std::string& name, const glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void setMat4_shader(unsigned int shader_id, const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}


void updateBuffer(unsigned int& id, unsigned int offset, void* data, unsigned int size, unsigned int type) {
    glBindBuffer(type, id);
    glBufferSubData(type, offset, size, data);
}
//updateBuffer(vbo, 0, vertex.data(), sizeof(uint)* vertex.size(), GL_ARRAY_BUFFER);


//std::string str_strip(std::string const& original, std::vector<char> ch_list)
//{
//    auto isNotSpace = [&ch_list](char c) { std::find(ch_list.begin(), ch_list.end(), c) == ch_list.end(); };
//    std::string::const_iterator right = std::find_if(original.rbegin(), original.rend(), isNotSpace).base();
//    std::string::const_iterator left = std::find_if(original.begin(), right, isNotSpace);
//    return std::string(left, right);
//}

