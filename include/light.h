#pragma once

#include "transform.h"
#include "snowflake.h"

#include <glm/glm.hpp>
#include <memory>
#include <string>


class PointLight;
class DirectLight;
typedef std::shared_ptr<PointLight>  PointLight_ptr;
typedef std::shared_ptr<DirectLight> DirectLight_ptr;



struct LightCommonAttr {
	glm::vec3 ambient{ 0.2, 0.2, 0.2 };
	glm::vec3 diffuse{ 0.5, 0.5, 0.5 };
	glm::vec3 specular{ 1.0, 1.0, 1.0 };
};


class DirectLight {
public:

	std::string name = "";
	LightCommonAttr light_common_attr;
	Transform transform;

	DirectLight() {}
	DirectLight(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular) : light_common_attr({ ambient, diffuse, specular }) {}
	DirectLight(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular, Transform& transform) :
		light_common_attr({ ambient, diffuse, specular }), transform(transform) {}

	glm::vec3 get_direction() const {
		//glm::mat3 rot = quat_to_mat3(transform.rotationQ);
		//glm::vec3 res = rot * ini_direction;
		return transform.get_front();
	}

	glm::vec3 get_position() const {
		return transform.translate;
	}
};

void inject_directLight_to_shader(const std::string& descriptor, DirectLight& dir_light, Snowflake_type shader_uuid);


class PointLight  {
public:

	std::string name = "";
	LightCommonAttr light_common_attr;
	Transform transform;

	float constant = 1.0f;
	float linear = 0.22f;
	float quadratic = 0.2f;
	/*
		Attenuation

		Distance	Constant	Linear	Quadratic
		7			1.0			0.7		1.8
		13			1.0			0.35	0.44
		20			1.0			0.22	0.20
		32			1.0			0.14	0.07
		50			1.0			0.09	0.032
		65			1.0			0.07	0.017
		100			1.0			0.045	0.0075
		160			1.0			0.027	0.0028
		200			1.0			0.022	0.0019
		325			1.0			0.014	0.0007
		600			1.0			0.007	0.0002
		3250		1.0			0.0014	0.000007
	*/

	PointLight() {}
	PointLight(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular) : light_common_attr({ ambient, diffuse, specular }) {}
	PointLight(glm::vec3& ambient, glm::vec3& diffuse, glm::vec3& specular, Transform& transform) :
		light_common_attr({ ambient, diffuse, specular }), transform(transform) {}

	glm::vec3 get_position() const {
		return transform.translate;
	}

};

void inject_pointLight_to_shader(const std::string& descriptor, PointLight& point_light, Snowflake_type shader_uuid);

