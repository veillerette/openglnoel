#include "Deferred.h"

#define LOW
#ifdef LOW
#define COLOR_FORMAT GL_RGB16F
#else
#define COLOR_FORMAT GL_RGB32F
#endif

const GLenum Deferred::drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                                        GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
										GL_COLOR_ATTACHMENT6,
                                        GL_DEPTH_ATTACHMENT};

const GLenum Deferred::m_GBufferTextureFormat[GBufferTextureCount] =
		{COLOR_FORMAT, COLOR_FORMAT, COLOR_FORMAT, COLOR_FORMAT, COLOR_FORMAT, COLOR_FORMAT, COLOR_FORMAT, GL_DEPTH_COMPONENT32F};

Deferred::Deferred(Window *window, ShadowMap *map) : window(window), shaderPass(nullptr), geometryPass(nullptr),
quad(nullptr), gamma(2.2), what(0), light(0,0,0), reloadSM(true), shadowMap(map),
smBias(0.001), smRes(SHADOW_MAP_RES), bloom(nullptr), powerBloom(POWER_BLOOM), metal(0.54),
rough(0.24){
	glGenTextures(GBufferTextureCount, m_GBufferTextures);
	glGenFramebuffers(1, &fbo);

	shaderPass = new Shader{"bin/shaders/app/ShadingPass.vs.glsl", "bin/shaders/app/ShadingPass.fs.glsl"};
	geometryPass = new Shader{"bin/shaders/app/geometryPass.vs.glsl", "bin/shaders/app/geometryPass.fs.glsl"};

	initTextures();
	initFramebuffer();

	quad = Geometry::generateQuad();
	light = glm::vec3(-25, -128, -410);

	bloom = new Bloom(window, ITER_DEFAULT);
}

Deferred::~Deferred() {
	delete shaderPass;
	delete geometryPass;
}

void Deferred::initTextures() {
	for (int i = 0; i < GBufferTextureCount; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i],
					   window->getWidth(), window->getHeight());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Deferred::initFramebuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	for (int i = 0; i < GDepth; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDepth]);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);
	glDrawBuffers(7, drawBuffers);

	if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error while creating main deferred framebuffer, exiting...";
		exit(1);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

Shader *Deferred::getGeometryShader() {
	return geometryPass;
}

Shader *Deferred::getShadingShader() {
	return shaderPass;
}

void Deferred::geometryRender(std::function<void()> & render, TrackballCamera *cam) {
	// Put here rendering code
	glClearColor(2.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glViewport(0, 0, window->getWidth(), window->getHeight());

	bool filled = window->filled();
	if (filled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	geometryPass->use();
	glm::mat4 viewMatrix = cam->getViewMatrix();
	glm::mat4 NM = glm::transpose(glm::inverse(viewMatrix));
	glm::mat4 MVP = window->getProjectionMatrix() * viewMatrix;
	geometryPass->uniformMatrix("uMVPMatrix", MVP);
	geometryPass->uniformMatrix("uMVMatrix", viewMatrix);
	geometryPass->uniformMatrix("uNormalMatrix", NM);
	geometryPass->uniformValue("uTexture", 0);
	geometryPass->uniformValue("uTexture2", 1);
	geometryPass->uniformValue("uTexture3", 2);
	geometryPass->uniformValue("uTexture4", 3);
    geometryPass->uniformValue("uTexture5", 4);

	render();
}

void Deferred::shadingRender(glm::vec3 light, TrackballCamera *cam) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glViewport(0, 0, window->getWidth(), window->getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderPass->use();
	shaderPass->uniformValue("uGPosition", 0);
	shaderPass->uniformValue("uGNormal", 1);
	shaderPass->uniformValue("uGAmbient", 2);
	shaderPass->uniformValue("uGDiffuse", 3);
	shaderPass->uniformValue("uGlossyShininess", 4);
    shaderPass->uniformValue("uNormalMaps", 5);
	shaderPass->uniformValue("uTangent", 6);
    shadowMap->bind(shaderPass, cam);
    shaderPass->uniformValue("uDirLightShadowMapBias", smBias);

    auto light_vs = glm::vec3(cam->getViewMatrix() * glm::vec4(light, 1));
	shaderPass->uniformVector("uLightDir_vs", light_vs);
	shaderPass->uniformValue("uGamma", gamma);
	shaderPass->uniformValue("uBloomPower", powerBloom);
	shaderPass->uniformValue("uMetallic", metal);
	shaderPass->uniformValue("uRoughness", rough);

	quad->enable();

	for (int i = 0; i < GDepth; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
	}

	quad->draw();
	quad->disable();
}

void Deferred::render(std::function <void()> render, std::function <void()> simpleRender, TrackballCamera *cam) {
	if(reloadSM) {
		shadowMap->compute(simpleRender, light);
		reloadSM = false;
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	geometryRender(render, cam);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	if(what-1 == -1) {
		bloom->bind();
		shadingRender(light, cam);
		bloom->render();
	} else {
		renderSpecific(what-1);
	}
}

void Deferred::renderSpecific(int i) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glReadBuffer(drawBuffers[i]);
	glBlitFramebuffer(0, 0, window->getWidth(), window->getHeight(), 0, 0, window->getWidth(), window->getHeight(),  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void Deferred::gui(Skin & animData) {
	ImGui::SliderFloat("Gamma", &gamma, 0.1, 4.0);
	const char *buf[] = {"Shading total", "Positions", "Normales", "Emissive", "Diffuse", "Metal-Rough", "NormalMaps", "Tangents"};
	ImGui::ListBox("Affichage", &what, buf, 8);
	ImGui::Separator();
	if(ImGui::SliderFloat3("Light", glm::value_ptr(light), -500, 500)) {
		reloadSM = true;
	}
	ImGui::Separator();
	ImGui::Text("Shadow Map Parameters");
	ImGui::SliderFloat("Bias", &smBias, 0.0, 0.04);
	if(ImGui::SliderInt("Resolution", &smRes, 512, 8192)) {
		shadowMap->reCreate(smRes);
		reloadSM = true;
	}
	ImGui::Separator();
	ImGui::Text("Bloom Parameters");
	bloom->gui();
	ImGui::SliderFloat("Power", &powerBloom, 1, 35);
	ImGui::Separator();
	ImGui::Text("PBR Defaults");
	ImGui::SliderFloat("Metallic", &metal, 0.0, 1.0);
	ImGui::SliderFloat("Roughness", &rough, 0.0, 1.0);
	ImGui::Checkbox("Animate", &(animData.do_anim));
    ImGui::Checkbox("Restart", &(animData.do_restart));
    ImGui::SliderFloat("Speed", &(animData.speed), 0.10, 10.0);
}