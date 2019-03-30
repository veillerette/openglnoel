#include "NodePose.h"

#include <iostream>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

NodePose::NodePose(glm::vec4 rotation, glm::vec3 scale, glm::vec3 translation) :
    rotation(glm::quat(rotation)), scale(scale), translation(translation), matrix(glm::mat4(1)), transform(glm::mat4(1)),
    init_rotation(rotation), init_scale(scale), init_translation(translation), init_matrix(matrix) {

}

NodePose::NodePose(glm::mat4 matrix) :
    rotation(glm::quat(glm::vec4(0, 0, 0, 1))), scale(glm::vec3(1, 1, 1)), translation(glm::vec3(0, 0, 0)), matrix(matrix), transform(glm::mat4(1)),
    init_rotation(rotation), init_scale(scale), init_translation(translation), init_matrix(matrix) {

}

static glm::quat correct_quaternion(glm::quat q) {
    float a = acos(q.w);
    float s = sin(a);
    if(s != 0) {
        q.x /= s;
        q.y /= s;
        q.z /= s;
    }
    a *= M_PI;
    s = sin(a);
    q.w = cos(a);
    q.x *= s;
    q.y *= s;
    q.z *= s;
    return q;

}

void NodePose::computeLocalTransform() {
    transform = glm::mat4(1);
    transform *= matrix;

    transform *= glm::translate(glm::mat4(1), translation);

    transform *= glm::toMat4(correct_quaternion(rotation)); /* it seems the angle has a PI factor problem */

    transform *= glm::scale(glm::mat4(1), scale);
}

void NodePose::reset() {
    rotation = init_rotation;
    translation = init_translation;
    scale = init_scale;
    matrix = init_matrix;
}