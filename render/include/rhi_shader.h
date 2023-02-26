#pragma once

#include "shader.h"

unsigned int _compile_shader(const char* vert_shader_content, const char* frag_shader_content);

void extract_shader_variables(Shader_ptr shader);