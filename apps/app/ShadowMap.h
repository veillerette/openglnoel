#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include <iostream>
#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include "Shader.h"
#include <functional>
#include "TrackballCamera.h"
#include <glmlv/glmlv.hpp>

#define SHADOW_MAP_RES 4096

class ShadowMap {
public:
	ShadowMap(int res = SHADOW_MAP_RES);

	~ShadowMap();

	void reCreate(int res);

	void bind(Shader *shadingPass, TrackballCamera *cam);

	void compute(std::function<void()> render, glm::vec3 light);

	void gui();
private:
	GLuint m_directionalSMTexture;
	GLuint m_directionalSMFBO;
	GLuint m_directionalSMSampler;
	int32_t m_nDirectionalSMResolution;

	glm::mat4 dirLightViewMatrix;
	glm::mat4 dirLightProjMatrix;

	Shader *shader;

	void init();

public:
	Shader *getShader() const;

private:
	void initTexture();
	void initFramebuffer();
	void initSampler();
	void buildProgram();
	void clean();
	glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians);
};


#endif //SHADOWMAP_H
