#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

class Shader;
typedef std::shared_ptr<Shader> Shader_ptr;


void load_shader(Shader_ptr shader, const std::string& vert_shader_path, const std::string& frag_shader_path);
bool compile_shader(Shader_ptr shader);



class ShaderVariable {
public:
    static const int GLAttribute = 1;
    static const int GLUniform = 2;

    int GLtype = 0;
    std::string varname = "";
    std::string vartype = "";
    int varsize = 0;
};



class Shader {
public:
	unsigned int shader_id = 0;

    std::string shader_fragment_src;
    std::string shader_vertex_src;

    std::vector<ShaderVariable> variables;

    Shader() = default;

    ~Shader();

    Shader(Shader const&) = delete;
    Shader& operator=(Shader const&) = delete;

    void use_shader();
    void setBool(const std::string& name, bool value);
    void setFloat(const std::string& var_name, float value);
    void setInt(const std::string& var_name, int value);
    void setVec2(const std::string& var_name, const glm::vec2& value);
    void setVec2(const std::string& var_name, float x, float y);
    void setVec3(const std::string& var_name, float x, float y, float z);
    void setVec3(const std::string& var_name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat);
    void setMat3(const std::string& name, const glm::mat3& mat);
    void setMat4(const std::string& name, const glm::mat4& mat);

};
