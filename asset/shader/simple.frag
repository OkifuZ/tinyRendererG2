#version 330 core

in vec3 FragPos;  

out vec4 FragColor;

struct Material {
    vec3 color;   
}; 

uniform Material material;

  
void main()
{
   FragColor = vec4(material.color, 1.0);
}  

