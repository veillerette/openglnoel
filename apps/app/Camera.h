#ifndef DEF_HEADER_CAMERA
#define DEF_HEADER_CAMERA

#include <glm/glm.hpp>
#include <glmlv/GLFWHandle.hpp>

class Camera {
public:
	virtual glm::mat4 getViewMatrix() const = 0;
	virtual glm::vec4 getPosition() const = 0;
	virtual void gestEvent(GLFWwindow *window) = 0;
};

#endif