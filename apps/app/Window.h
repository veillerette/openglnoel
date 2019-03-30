
#ifndef OPENGLNOEL_WINDOW_H
#define OPENGLNOEL_WINDOW_H

#include <string>
#include <glm/glm.hpp>
#include <functional>
#include <iomanip>
#include <sstream>

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include "FPSManager.h"

class Window {
public:
	Window(const char *argv[], uint32_t width, uint32_t height, std::string windowName, uint32_t fps);

	~Window();

	uint32_t getWidth() const;

	uint32_t getHeight() const;

	void run(std::function<bool(GLFWwindow *)> play, std::function<void()> draw, std::function<void()> gui);

	glm::mat4 getProjectionMatrix();

	GLFWwindow *getWindow();

	void flip();

	bool filled();

private:
	glmlv::GLFWHandle *window;
	const glmlv::fs::path appPath;
	const unsigned int width;
	const unsigned int height;
	const std::string appName;
	const glmlv::fs::path shadersPath;
	uint32_t fps;

	void handlerTab();

	FPSManager fpsManager;


	bool glFilled;
};


#endif //OPENGLNOEL_WINDOW_H
