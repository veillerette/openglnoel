#ifndef SCENE_H
#define SCENE_H

#include <tiny_gltf.h>
#include <iostream>
#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>

#include "Deferred.h"
#include "Skin.h"
#include <glmlv/Image2DRGBA.hpp>
#include "Shader.h"

class SceneDescriptor {
public:
	SceneDescriptor(const char *path, Shader * sShader);

	void drawModel(Deferred *renderer);

	void debug();

	void bindScene();

	void activeTexturing();

	void desactiveTexturing();
    void refreshPoses();

	const glm::vec3 getMin() const;

	const glm::vec3 getMax() const;

	const glm::vec3 getCenter() const;

	const float size() const;

	Skin & animData();

private:
	tinygltf::Model model;
	std::map<int, GLuint> vaos; // one vao per mesh
	std::map<int, GLuint> textures; // gl texture by id
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 center;
	std::map<int, int> rig; //node pose associated to a mesh
	Skin skin;
	bool activeTextures;
	Shader *shadowShader;

	void drawVertices(int i, Deferred *renderer);

	void drawNodes(tinygltf::Node &node, Deferred *renderer);

	GLuint loadMesh(tinygltf::Mesh &mesh);

	void loadOneVBO(GLuint *vbo, tinygltf::Accessor access);

	GLuint loadTexture(tinygltf::Texture texture);

	void loadAllTextures();

	void loadPoses();

	void computePosesTransforms();

	void computeBox();

};


#endif //SCENE_H


