#include "ShadowMap.h"

ShadowMap::ShadowMap(int res) : m_directionalSMTexture(0), m_directionalSMFBO(0),
					m_directionalSMSampler(0), m_nDirectionalSMResolution(res),
					shader(nullptr) {
	init();
}

void ShadowMap::init() {
	initTexture();
	initFramebuffer();
	initSampler();
	buildProgram();
}

void ShadowMap::clean() {
	glDeleteTextures(1, &m_directionalSMTexture);
	glDeleteSamplers(1, &m_directionalSMSampler);
	glDeleteFramebuffers(1, &m_directionalSMFBO);
}

void ShadowMap::reCreate(int res) {
	clean();
	m_nDirectionalSMResolution = res;
	initTexture();
	initSampler();
	initFramebuffer();
}

ShadowMap::~ShadowMap() {
	clean();
	delete shader;
}

void ShadowMap::initTexture() {
	glGenTextures(1, &m_directionalSMTexture);
	glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F,
				   m_nDirectionalSMResolution, m_nDirectionalSMResolution);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ShadowMap::initFramebuffer() {
	glGenFramebuffers(1, &m_directionalSMFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_directionalSMFBO);
	glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			m_directionalSMTexture, 0);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR creating framebuffer for the shadowmap " << std::endl;;
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::initSampler() {
	glGenSamplers(1, &m_directionalSMSampler);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

void ShadowMap::buildProgram() {
	shader = new Shader("bin/shaders/app/SM.vs.glsl",
	"bin/shaders/app/SM.fs.glsl");

	if(nullptr == shader) {
		std::cout << "ERROR loading shader for the shadowmap" << std::endl;
		exit(EXIT_FAILURE);
	}
}

glm::vec3 ShadowMap::computeDirectionVector(float phiRadians, float thetaRadians)
{
	const auto cosPhi = glm::cos(phiRadians);
	const auto sinPhi = glm::sin(phiRadians);
	const auto cosTheta = glm::cos(thetaRadians);
	return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
}


void ShadowMap::compute(std::function<void()> render, glm::vec3 light) {
	float sceneRadius = 300;
	const auto dirLightUpVector = computeDirectionVector(glm::radians(glm::pi<float>()), glm::radians(glm::pi<float>()));
	dirLightViewMatrix = glm::lookAt(light, glm::vec3(0,0,0), dirLightUpVector);
	dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

	shader->use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
	glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto matrix = dirLightProjMatrix * dirLightViewMatrix;
	shader->uniformMatrix("uDirLightViewProjMatrix", matrix);

	render();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShadowMap::bind(Shader *shadingPass, TrackballCamera *cam) {
	shadingPass->uniformValue("uDirLightShadowMap", 10);
	glActiveTexture(GL_TEXTURE10);
	glBindSampler(10, m_directionalSMSampler);
	glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);

	const auto rcpViewMatrix = glm::inverse(cam->getViewMatrix());
	auto tmp = dirLightProjMatrix * dirLightViewMatrix * rcpViewMatrix;
	shadingPass->uniformMatrix("uDirLightViewProjMatrix", tmp);

}

Shader *ShadowMap::getShader() const {
	return shader;
}
