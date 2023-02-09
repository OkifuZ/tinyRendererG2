#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}

void processInput(GLFWwindow* window);

GLFWwindow* render_init(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT, const char* title, bool hide_window = false, glm::vec3 bgcolor = { 0.2f,0.05f,0.05f });

unsigned int load_shader(const std::string& vert_shader_path, const std::string& frag_shader_path);

void use_shader(unsigned int shader_id);

void setFloat_shader(unsigned int shader_id, const std::string& var_name, float value);
void setInt_shader(unsigned int shader_id, const std::string& var_name, int value);
void setVec2_shader(unsigned int shader_id, const std::string& var_name, const glm::vec2& value);
void setVec2_shader(unsigned int shader_id, const std::string& var_name, float x, float y);
void setVec3_shader(unsigned int shader_id, const std::string& var_name, float x, float y, float z);
void setVec3_shader(unsigned int shader_id, const std::string& var_name, const glm::vec3& value);
void setVec4_shader(unsigned int shader_id, const std::string& name, const glm::vec4& value);
void setVec4_shader(unsigned int shader_id, const std::string& name, float x, float y, float z, float w);
void setMat2_shader(unsigned int shader_id, const std::string& name, const glm::mat2& mat);
void setMat3_shader(unsigned int shader_id, const std::string& name, const glm::mat3& mat);
void setMat4_shader(unsigned int shader_id, const std::string& name, const glm::mat4& mat);

void updateBuffer(unsigned int& id, unsigned int offset, void* data, unsigned int size, unsigned int type);
// updateBuffer(vbo, 0, vertex.data(), sizeof(uint)* vertex.size(), GL_ARRAY_BUFFER);
