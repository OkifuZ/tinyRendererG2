#version 330 core

in vec3 FragPos;  
in vec3 Normal;  
in vec3 Color;
// in vec2 TexCoords;

out vec4 FragColor;

struct Material {
    float shininess;
}; 

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

#define MAX_POINT_LIGHTS 4  
#define MAX_DIRECT_LIGHTS 4  

// functions declaration
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir); 
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirLight directLights[MAX_DIRECT_LIGHTS];
uniform int num_pointLights;
uniform int num_directLights;


uniform vec3 viewPos;
uniform Material material;

  
void main()
{
  // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);

    // phase 1: Directional lighting
    for(int i = 0; i < num_directLights; i++)
        result += CalcDirLight(directLights[i], norm, viewDir);
    // phase 2: Point lights
    for(int i = 0; i < num_pointLights; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
    
   FragColor = vec4(result, 1.0);
}  


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * Color;
    vec3 diffuse  = light.diffuse  * diff * Color;
    vec3 specular = light.specular * spec * Color;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * Color;
    vec3 diffuse  = light.diffuse  * diff * Color;
    vec3 specular = light.specular * spec * Color;
    return (ambient + diffuse + specular);
}  