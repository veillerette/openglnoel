#include "Window.h"
#include "Geometry.h"
#include "Shader.h"
#include "TrackballCamera.h"
#include "Deferred.h"
#include "Scene.h"
#include "ShadowMap.h"
#include <algorithm>

int main(int argc, const char *argv[]) {
	auto *window = new Window(argv, 1920, 1080, "Projet gltf", 60);
	auto *shadowMap = new ShadowMap();
	auto *renderer = new Deferred(window, shadowMap);
	auto *gShader = renderer->getGeometryShader();
	auto *sShader = renderer->getShadingShader();
	double timer = glfwGetTime();
	const char *SCENE_PATH = ((argc > 1) ? argv[1] : ("scenes/drone/scene.gltf"));
	SceneDescriptor *scene = new SceneDescriptor(SCENE_PATH, shadowMap->getShader());
	auto *cam = new TrackballCamera(std::max(scene->size(), 1.0f) * 1.5);
	scene->debug();
	scene->bindScene();

	glm::vec3 center = scene->getCenter();
	cam->setOrigin(glm::translate(glm::mat4(1.0), center));


	window->run([&](auto *win) { // events
		auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		if (!guiHasFocus) {
			cam->gestEvent(win);
		}
		return true;
	}, [&]() { // render
		renderer->render([&]() {
			scene->activeTexturing();
			scene->drawModel(renderer);
		}, [&]() {
			scene->desactiveTexturing();
			scene->drawModel(renderer);
		}, cam);
		scene->refreshPoses();
	}, [&]() { // gui
		ImGui::Separator();
		ImGui::Text(("Camera Position : " + glm::to_string(cam->getPosition())).c_str());
		renderer->gui(scene->animData());
	});

	delete renderer;
	delete window;
	return 0;
}
