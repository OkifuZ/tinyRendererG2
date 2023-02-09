#version 330 core
out vec4 FragColor;

in vec3 fColor;
in vec2 fCenter;

void main()
{
	// why this draw a circle ???
	float rsq = dot(fCenter, fCenter);
	if (rsq > 1)
		discard;
	FragColor = vec4(fColor, 1.0);
}