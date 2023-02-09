#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "rhi_shader.h"
#include "shader.h"
#include "file_system.h"

#include <sstream>
#include <fstream>


void load_shader(Shader_ptr shader, const std::string& vert_shader_path, const std::string& frag_shader_path) {
    if (!shader) return;
    shader->shader_vertex_src = read_file_as_str(vert_shader_path);
    printf("load_shader() %s completed\n", vert_shader_path.c_str());
    shader->shader_fragment_src = read_file_as_str(frag_shader_path);
    printf("load_shader() %s completed\n", frag_shader_path.c_str());
}

bool compile_shader(Shader_ptr shader) {
    if (!shader) return false;
    auto vertexShaderSource_cstr = shader->shader_vertex_src.c_str();
    auto fragmentShaderSource_cstr = shader->shader_fragment_src.c_str();
    unsigned int shader_id = _compile_shader(vertexShaderSource_cstr, fragmentShaderSource_cstr);
    if (!shader_id) return false;
    shader->shader_id = shader_id;
    extract_shader_variables(shader);
    
    return true;
}

Shader::~Shader() {
    if (glIsProgram(this->shader_id)) {
        glDeleteShader(this->shader_id);
    }
}

void Shader::use_shader() {
    glUseProgram(shader_id);
}
void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(glGetUniformLocation(shader_id, name.c_str()), (int)value);
}
void Shader::setFloat(const std::string& var_name, float value) {
    glUniform1f(glGetUniformLocation(shader_id, var_name.c_str()), value);
}
void Shader::setInt(const std::string& var_name, int value)
{
    glUniform1i(glGetUniformLocation(shader_id, var_name.c_str()), value);
}
void Shader::setVec2(const std::string& var_name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(shader_id, var_name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& var_name, float x, float y)
{
    glUniform2f(glGetUniformLocation(shader_id, var_name.c_str()), x, y);
}
void Shader::setVec3(const std::string& var_name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(shader_id, var_name.c_str()), x, y, z);
}
void Shader::setVec3(const std::string& var_name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(shader_id, var_name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(shader_id, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shader_id, name.c_str()), x, y, z, w);
}
void Shader::setMat2(const std::string& name, const glm::mat2& mat)
{
    glUniformMatrix2fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(shader_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}


