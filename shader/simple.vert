#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

out vec3 fColor;
out vec2 fCenter;

uniform float scale;

void main()
{
	vec2 offset = (aOffset - 0.5f) * 2.0f; // [0, 1]^2 -> [-1, 1]^ 2
	gl_Position = vec4(aPos*scale+offset, 0.0, 1.0);
	fColor = aColor;
	fCenter = aPos;
}