#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "rhi_shader.h"
#include "shader.h"

#include <cstdio>


static std::string nameof_gltype(GLenum type) {
    switch (type)
    {
    case GL_FLOAT:        return "float";
    case GL_FLOAT_VEC2:   return "vec2";
    case GL_FLOAT_VEC3:   return "vec3";
    case GL_FLOAT_VEC4:   return "vec4";
    case GL_DOUBLE:		  return "double";
    case GL_INT:		  return "int";
    case GL_INT_VEC2:	  return "ivec2";
    case GL_INT_VEC3:	  return "ivec3";
    case GL_INT_VEC4:	  return "ivec4";
    case GL_UNSIGNED_INT: return "unsigned int";
    case GL_BOOL:         return "bool";
    case GL_FLOAT_MAT2:   return "mat2";
    case GL_FLOAT_MAT3:   return "mat3";
    case GL_FLOAT_MAT4:   return "mat4";
    }
    return "";
}

unsigned int _compile_shader(const char* vert_shader_content, const char* frag_shader_content) {

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!vertexShader) return 0;
    glShaderSource(vertexShader, 1, &vert_shader_content, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("[_compile_shader()] vertex shader compilation failed\n");
        return 0;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!fragmentShader) return 0;
    glShaderSource(fragmentShader, 1, &frag_shader_content, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("[_compile_shader()] fragment shader compilation failed\n");
        return 0;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    if (!shaderProgram) return 0;
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("[_compile_shader()] linking failed\n");
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void extract_shader_variables(Shader_ptr shader) {
    if (!glIsProgram(shader->shader_id)) return;

    GLint i;
    GLint count;

    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 128; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(shader->shader_id, GL_ACTIVE_ATTRIBUTES, &count);
    for (i = 0; i < count; i++)
    {
        glGetActiveAttrib(shader->shader_id, (GLuint)i, bufSize, &length, &size, &type, name);
        shader->variables.push_back(ShaderVariable{ ShaderVariable::GLAttribute, name, nameof_gltype(type), size });
    }

    glGetProgramiv(shader->shader_id, GL_ACTIVE_UNIFORMS, &count);
    for (i = 0; i < count; i++)
    {
        glGetActiveUniform(shader->shader_id, (GLuint)i, bufSize, &length, &size, &type, name);
        shader->variables.emplace_back(ShaderVariable{ ShaderVariable::GLUniform, name, nameof_gltype(type), size });
    }

}