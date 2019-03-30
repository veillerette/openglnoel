#include "TrackballCamera.h"

static int BERK_SCROLL_Y = 0;

TrackballCamera::TrackballCamera(float distance, float speed, float angleX, float angleY)
		: _distance(distance), _speed(speed), _angleX(angleX), _angleY(angleY), mouseDone(false), origin(glm::mat4(1.0f)) {
}

void TrackballCamera::moveFront(float delta) {
	_distance -= delta * _speed;
}

void TrackballCamera::rotateLeft(float degrees) {
	_angleY -= degrees;
}

void TrackballCamera::rotateUp(float degrees) {
	_angleX -= degrees;
}

void TrackballCamera::setOrigin(glm::mat4 _origin) {
	origin = _origin;
}

void TrackballCamera::setDistance(float distance) {
	_distance = distance;
}

void TrackballCamera::setSpeed(float speed) {
	if (speed != 0) {
		_speed = speed;
	}
}

glm::mat4 TrackballCamera::getViewMatrix() const {
	glm::mat4 res(1.0f);
	res = glm::translate(res, glm::vec3(0.0, 0.0, -_distance));
	res = glm::rotate(res, _angleX, glm::vec3(1.0, 0.0, 0.0));
	res = glm::rotate(res, _angleY, glm::vec3(0.0, 1.0, 0.0));

	return res * (glm::inverse(origin));
}

glm::vec4 TrackballCamera::getPosition() const {
	return glm::inverse(getViewMatrix()) * glm::vec4(0, 0, 0, 1);
}

void TrackballCamera::gestEvent(GLFWwindow *window) {
	auto fct = [](GLFWwindow *win, double x, double y) {
		BERK_SCROLL_Y = y;
	};
	glfwSetScrollCallback(window, fct); // IT IS CREEPY

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (mouseDone) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			rotateLeft(-(x - tmpX) / 400.0f);
			rotateUp(-(y - tmpY) / 400.0f);
			tmpX = x;
			tmpY = y;
		} else {
			mouseDone = true;
			glfwGetCursorPos(window, &tmpX, &tmpY);
		}
	} else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		mouseDone = false;
	}

	if(BERK_SCROLL_Y != 0) {
		this->moveFront(BERK_SCROLL_Y * (_distance * 10 / 250.0));
		BERK_SCROLL_Y = 0;
	}

}

float TrackballCamera::get_angleX() const {
	return _angleX;
}

float TrackballCamera::get_angleY() const {
	return _angleY;
}
