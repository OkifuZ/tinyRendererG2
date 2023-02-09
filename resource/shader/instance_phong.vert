#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aOffset;
layout (location = 4) in vec3 aColor;
layout (location = 5) in float aScale;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
// out vec2 TexCoords;

// uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 model = mat4(1.0f) * aScale;
    model[3] = vec4(aOffset, 1.0f);

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  

    Color = aColor;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}