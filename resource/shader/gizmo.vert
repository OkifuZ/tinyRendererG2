#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec3 aOffset;

out vec3 FragPos;
out vec3 Normal;
// out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float scale;

void main()
{
    vec4 real_offset = model * vec4(aOffset, 1.0f);

    mat4 _model = mat4(1.0f) * scale;
    _model[3] = real_offset;

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  

    gl_Position = projection * view * vec4(FragPos, 1.0);
}