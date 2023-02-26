#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <memory>

class Transform;
typedef std::shared_ptr<Transform> Transform_ptr;

glm::mat4x4 quat_to_mat4(glm::quat rotationQ);

glm::mat3x3 quat_to_mat3(glm::quat rotationQ);

glm::vec3 quat_to_euler(glm::quat rotationQ);

glm::quat euler_to_quat(glm::vec3 euler_angles);

glm::quat mat4_to_quat(glm::mat4 rot);

glm::quat mat3_to_quat(glm::mat3 rot);

glm::vec3 mat4_to_euler(glm::mat4 rot);

glm::vec3 mat4_to_euler(glm::mat3 rot);

class Transform {
public:
    glm::vec3 translate = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat rotationQ = glm::identity<glm::quat>();
    glm::vec3 center_offset = glm::vec3(0.0f);

    static const glm::vec3 ini_front;
    static const glm::vec3 ini_up;
    static const glm::vec3 ini_right;

    Transform() = default;
    Transform(glm::vec3 translate, glm::vec3 scale, glm::quat rotationQ) :
        translate(translate), scale(scale), rotationQ(rotationQ) {}
    Transform(glm::vec3 translate, glm::vec3 scale, glm::vec3 eulerAngles) :
        translate(translate), scale(scale), rotationQ(glm::quat(eulerAngles)) {}

    glm::quat to_quaternion(double pitch, double yaw, double roll) // roll (x), pitch (Y), yaw (z)
    {
        // Abbreviations for the various angular functions

        float cr = std::cos(roll * 0.5);
        float sr = std::sin(roll * 0.5);
        float cp = std::cos(pitch * 0.5);
        float sp = std::sin(pitch * 0.5);
        float cy = std::cos(yaw * 0.5);
        float sy = std::sin(yaw * 0.5);

        glm::quat q = glm::identity<glm::quat>();
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
    }

    void yaw(float degrees) {
        // right-hand rule
        // positive degree -> left
        //rotationQ = glm::rotate(rotationQ, glm::radians(degrees), get_up());
        glm::vec3 eu = glm::eulerAngles(rotationQ);
        eu.y = glm::radians(degrees);
        rotationQ = glm::quat(eu);
    }

    void yaw_add(float degrees) {
        // right-hand rule
        // positive degree -> left
        glm::vec3 eu = glm::degrees(glm::eulerAngles(rotationQ));
        eu.y += degrees;
        if (eu.y > 89.9) eu.y = 89.9;
        if (eu.y < -89.9) eu.y = -89.9;
        rotationQ = glm::quat(glm::radians(eu));
    }

    void pitch(float degrees) {
        // right-hand rule
        // positive degree -> up
        //rotationQ = glm::rotate(rotationQ, glm::radians(degrees), get_right());
        glm::vec3 eu = glm::eulerAngles(rotationQ);
        eu.x = glm::radians(degrees);
        rotationQ = glm::quat(eu);
    }

    void pitch_add(float degrees) {
        // right-hand rule
        // positive degree -> up
        //rotationQ = glm::rotate(rotationQ, glm::radians(degrees), get_right());
        glm::vec3 eu = glm::degrees(glm::eulerAngles(rotationQ));
        eu.x += degrees;
        if (eu.x > 89) eu.x = 89;
        if (eu.x < -89) eu.x = -89;
        rotationQ = glm::quat(glm::radians(eu));
    }

    void roll(float degrees) {
        // right-hand rule
        // positive degree -> clockwise
        //rotationQ = glm::rotate(rotationQ, glm::radians(degrees), get_front());
        glm::vec3 eu = glm::eulerAngles(rotationQ);
        eu.z = glm::radians(degrees);
        rotationQ = glm::quat(eu); 
    }

    void roll_add(float degrees) {
        // right-hand rule
        // positive degree -> clockwise
        //rotationQ = glm::rotate(rotationQ, glm::radians(degrees), get_front());
        glm::vec3 eu = glm::eulerAngles(rotationQ);
        eu.z += glm::radians(degrees);
        rotationQ = glm::quat(eu);
    }

    void yaw_pitch_roll(float y_degrees, float p_degrees, float r_degrees) {
        glm::mat4 rot = glm::yawPitchRoll(glm::radians(y_degrees), glm::radians(p_degrees), glm::radians(r_degrees));
        glm::mat4 cur_rot = quat_to_mat4(rotationQ);
        this->rotationQ = glm::quat(cur_rot * rot); // ?
    }

    void fix_rotate(float degrees, char axis) {
        if (axis == 'x' || axis == 'X') {
            
        }
        else if (axis == 'y' || axis == 'Y') {
        
        }
        else if (axis == 'z' || axis == 'Z') {
            
        }
    }

    void look_at(glm::vec3& direction) {
        look_at(this->translate, this->translate + direction);
    }

    void look_at(glm::vec3& source, glm::vec3& target, glm::vec3& up = glm::vec3{ 0,1,0 }) {
        this->translate = source;

        glm::vec3  direction = target - source;
        float      directionLength = glm::length(direction);

        // Check if the direction is valid; Also deals with NaN
        if (!(directionLength > 0.0001))
            return; // Just do nothing for invalid lookat

        // Normalize direction
        direction /= directionLength;

        // Is the normal up (nearly) parallel to direction?
        if (glm::abs(glm::dot(direction, up)) > .9999f) {
            return; // Just do nothing for invalid lookat
        }

        // good
        this->rotationQ = glm::quatLookAt(direction, up);
    }

    glm::mat4 get_model_mat() const {
        glm::mat4 model_mat = glm::mat4(1.0f);
        model_mat = glm::translate(model_mat, this->translate); // right multiply
        model_mat = model_mat * glm::toMat4(rotationQ);
        model_mat = glm::scale(model_mat, this->scale);
        // centerlize should be done in python
        // model_mat = glm::translate(model_mat, -this->center_offset);
        return model_mat;
    }

    glm::vec3 get_front() const {
        return glm::vec3(glm::rotate(rotationQ, glm::vec4(this->ini_front, 0)));
        //return glm::toMat3(rotationQ) * this->ini_front;
    }
    glm::vec3 get_up() const {
        return glm::vec3(glm::rotate(rotationQ, glm::vec4(this->ini_up, 0)));
        //return glm::toMat3(rotationQ) * this->ini_up;
    }
    glm::vec3 get_right() const {
        return glm::vec3(glm::rotate(rotationQ, glm::vec4(this->ini_right, 0)));
        //return glm::toMat3(rotationQ) * this->ini_right;
    }

    glm::vec3 get_pyr() const {

        glm::vec3 euler_angles_r = glm::eulerAngles(rotationQ);
        return glm::degrees(euler_angles_r);
    }

};


