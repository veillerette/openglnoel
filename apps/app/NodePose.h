#ifndef OPENGLNOEL_NODEPOSE_H
#define OPENGLNOEL_NODEPOSE_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct NodePose {
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec3 translation;
    glm::mat4 matrix;

    glm::mat4 transform;

    glm::quat init_rotation;
    glm::vec3 init_scale;
    glm::vec3 init_translation;
    glm::mat4 init_matrix;

    int parent;

    NodePose(glm::vec4 rotation, glm::vec3 scale, glm::vec3 translation);
    NodePose(glm::mat4 matrix);

    void computeLocalTransform();
    void reset();
};

#endif //OPENGLNOEL_NODEPOSE_H
