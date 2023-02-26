#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aOffset;
layout (location = 3) in vec3 aColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 objectColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	FragPos = vec3(model * vec4(aPos + aOffset, 1.0f));
	gl_Position = projection * view * vec4(FragPos, 1.0f);
	Normal = aNormal;
	objectColor = aColor;
}