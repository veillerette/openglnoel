#include "Window.h"


Window::Window(const char **argv, uint32_t width,
			   uint32_t height, std::string windowName, uint32_t fps) : window(nullptr),
														  glFilled(true), height(height), width(width),
														  appPath(glmlv::fs::path(
																  argv[0])), appName(appPath.stem().string()),
																  fpsManager(fps), fps(fps){
	window = new glmlv::GLFWHandle((int) width, (int) height, windowName.c_str());
	if (nullptr == window) {
		std::cerr << "Unable to open window" << std::endl;
		exit(1);
	}

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Window::~Window() {
	if (nullptr != window) {
		delete window;
	}
}

uint32_t Window::getWidth() const {
	return width;
}

uint32_t Window::getHeight() const {
	return height;
}

void Window::run(std::function<bool(GLFWwindow *)> play, std::function<void()> draw, std::function<void()> gui) {
	bool cont = true;

	while (cont) {
		{ // render
			fpsManager.setControl(fps);
			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glViewport(0, 0, width, height);

			glClearColor(1.0, 0.7, 0.7, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			draw();

			glmlv::imguiNewFrame();
			ImGui::Begin("main GUI");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
						ImGui::GetIO().Framerate);
			ImGui::SliderInt("Goal FPS", (int *)&fps, 10, 3000);
			ImGui::Separator();
			gui();
			ImGui::End();

			flip();
		}
		{ // events
			glfwPollEvents();
			if (glfwGetKey(window->window(), GLFW_KEY_ESCAPE) || glfwWindowShouldClose(window->window())) {
				cont = false;
			}

			handlerTab();
			cont = cont && play(window->window());
		}
	}

}

glm::mat4 Window::getProjectionMatrix() {
	return glm::perspective(glm::radians(70.f), width * 1.0f / height, 0.1f, 890.f);
}

void Window::flip() {
	fpsManager.waitUntilNextFrame();

	if(fpsManager.canMaj()) {
		fpsManager.maj();
	}

	glmlv::imguiRenderFrame();
	window->swapBuffers();
}

GLFWwindow *Window::getWindow() {
	return window->window();
}

void Window::handlerTab() {
	if ((glfwGetKey(window->window(), GLFW_KEY_TAB) == GLFW_PRESS && glFilled)
	|| (glfwGetKey(window->window(), GLFW_KEY_TAB) == GLFW_RELEASE && !glFilled)) {
		glFilled = !glFilled;
	}
}

bool Window::filled() {
	return glFilled;
}
