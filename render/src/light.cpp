#include "light.h"
#include "resource_manager.h"
#include "shader.h"

void inject_directLight_to_shader(const std::string& descriptor, DirectLight& direct_light, Snowflake_type shader_uuid) {
	Shader_ptr shader = resource_manager_global.get_shader_by_uuid(shader_uuid);
	if (!shader) return;

	shader->use_shader();
	shader->use_shader();
	shader->setVec3(descriptor + ".ambient",   direct_light.light_common_attr.ambient);
	shader->setVec3(descriptor + ".diffuse",   direct_light.light_common_attr.diffuse);
	shader->setVec3(descriptor + ".specular",  direct_light.light_common_attr.specular);
	shader->setVec3(descriptor + ".direction", direct_light.get_direction());
}


void inject_pointLight_to_shader(const std::string& descriptor, PointLight& point_light, Snowflake_type shader_uuid) {
	Shader_ptr shader = resource_manager_global.get_shader_by_uuid(shader_uuid);
	if (!shader) return;

	shader->use_shader();
	shader->setVec3(descriptor + ".ambient", point_light.light_common_attr.ambient);
	shader->setVec3(descriptor + ".diffuse", point_light.light_common_attr.diffuse);
	shader->setVec3(descriptor + ".specular", point_light.light_common_attr.specular);
	shader->setVec3(descriptor + ".position", point_light.get_position());
	shader->setFloat(descriptor + ".constant", point_light.constant);
	shader->setFloat(descriptor + ".linear", point_light.linear);
	shader->setFloat(descriptor + ".quadratic", point_light.quadratic);
}
