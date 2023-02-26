#include "transform.h"

const glm::vec3 Transform::ini_front = glm::vec3{ 0,0,-1 };
const glm::vec3 Transform::ini_up = glm::vec3{ 0,1,0 };
const glm::vec3 Transform::ini_right = glm::vec3{ 1,0,0 };

glm::mat4x4 quat_to_mat4(glm::quat rotationQ) {
    return glm::toMat4(rotationQ);
}

glm::mat3x3 quat_to_mat3(glm::quat rotationQ) {
    return glm::toMat3(rotationQ);
}

glm::vec3 quat_to_euler(glm::quat rotationQ) {
    return glm::eulerAngles(rotationQ);
}

glm::quat euler_to_quat(glm::vec3 euler_angles) {
    return  glm::quat(euler_angles);
}

glm::quat mat4_to_quat(glm::mat4 rot) {
    return  glm::quat_cast(rot);
}

glm::quat mat3_to_quat(glm::mat3 rot) {
    return  glm::quat_cast(rot);
}

glm::vec3 mat4_to_euler(glm::mat4 rot) {
    return glm::eulerAngles(glm::quat_cast(rot));
}

glm::vec3 mat4_to_euler(glm::mat3 rot) {
    return glm::eulerAngles(glm::quat_cast(rot));
}

