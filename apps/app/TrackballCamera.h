#ifndef DEF_HEADER_TRACKBALL
#define DEF_HEADER_TRACKBALL


#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glmlv/GLFWHandle.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.h"

class TrackballCamera : public Camera {
private:
    float _distance;
    float _angleX;
    float _angleY;
    float _speed;

    bool mouseDone;
    bool mouseRight;
    glm::mat4 origin;

    double tmpX;
    double tmpY;

public:

    TrackballCamera(float distance, float speed = 1.f, float angleX = 0, float angleY = glm::pi<float>());

    void moveFront(float delta);

    void rotateLeft(float degrees);

    float get_angleX() const;

    float get_angleY() const;

    void rotateUp(float degrees);

    void setOrigin(glm::mat4 _origin);

    void setDistance(float distance);

    void setSpeed(float speed);

    glm::mat4 getViewMatrix() const override;

    glm::vec4 getPosition() const override;

    void gestEvent(GLFWwindow *event) override;

};

#endif