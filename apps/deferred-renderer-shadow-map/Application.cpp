#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glmlv;


void APIENTRY
MessageCallback(GLenum source, GLenum type,
				GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	fprintf(stderr,
			"GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
}

template<typename T>
std::pair<GLuint, unsigned int> constructObject(T &geometry) {
	GLuint
			vbo, ibo, vao;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3f3f2f) * geometry.vertexBuffer.size(), geometry.vertexBuffer.data(),
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer.size() * sizeof(uint32_t), geometry.indexBuffer.data(),
				 GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f),
						  (const GLvoid *) offsetof(Vertex3f3f2f, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f),
						  (const GLvoid *) offsetof(Vertex3f3f2f, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f),
						  (const GLvoid *) offsetof(Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return std::pair<GLuint, unsigned int>(vao, geometry.indexBuffer.size());
}

GLuint genQuad() {
	GLuint
			vbo, ibo, vao;

	GLfloat vertex[8] = {-1, -1, 1, -1, -1, 1, 1, 1};
	GLuint
			indices[6] = {0, 1, 2, 1, 2, 3};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 8, vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return vao;
}

GLuint importImage(const fs::path &path) {
	GLuint
			tex;
	std::cout << "import image..." << std::endl;
	Image2DRGBA img = readImage(path);
	std::cout << "image imported !" << img.width() << " " << img.height() << std::endl;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				 img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::cout << "texture ok !" << std::endl;

	return tex;
}


enum GBufferTextureType {
	GPosition = 0,
	GNormal,
	GAmbient,
	GDiffuse,
	GGlossyShininess,
	GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
	GBufferTextureCount
};

typedef struct ShadowMap {
	GLuint m_directionalSMTexture;
	GLuint m_directionalSMFBO;
	GLuint m_directionalSMSampler;
	int32_t m_nDirectionalSMResolution;
	GLProgram program;
	GLint idProjMatrix;

	ShadowMap() :
			m_directionalSMTexture(0), m_directionalSMFBO(0), m_directionalSMSampler(0),
			m_nDirectionalSMResolution(512) {
	}
	void compute(glm::mat4 lightProj, glm::mat4 lightView, std::function<void()> render) {
		program.use();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
		glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(idProjMatrix, 1, GL_FALSE, glm::value_ptr(lightProj * lightView));

		render();

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
} ShadowMap;


void initShadowMap(ShadowMap &map) {
	glGenTextures(1, &map.m_directionalSMTexture);
	glBindTexture(GL_TEXTURE_2D, map.m_directionalSMTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F,
				   map.m_nDirectionalSMResolution, map.m_nDirectionalSMResolution);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &map.m_directionalSMFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, map.m_directionalSMFBO);
	glBindTexture(GL_TEXTURE_2D, map.m_directionalSMTexture);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map.m_directionalSMTexture, 0);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR creating framebuffer of the shadowmap " << std::endl;;
		exit(EXIT_FAILURE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenSamplers(1, &map.m_directionalSMSampler);
	glSamplerParameteri(map.m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(map.m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(map.m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(map.m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);



	map.program = buildProgram({loadShader("bin/shaders/deferred-renderer-shadow-map/SM.vs.glsl"),
										 loadShader("bin/shaders/deferred-renderer-shadow-map/SM.fs.glsl")});
	map.idProjMatrix = glGetUniformLocation(map.program.glId(), "uDirLightViewProjMatrix");
}


int Application::run() {
	/*auto geo1 = makeCube();
	auto cube = constructObject(geo1);
	 */
	auto geo2 = makeSphere(32);
	auto sphere = constructObject(geo2);


	ShadowMap shadowMap;
	initShadowMap(shadowMap);


	SceneData data;
	loadAssimpScene("../scenes/assets/sponza.obj", data, true);
	auto scene = constructObject(data);


	std::vector<GLuint> myTextures;
	for (auto &img : data.textures) {
		GLuint
				tmp_tex;
		glGenTextures(1, &tmp_tex);
		glBindTexture(GL_TEXTURE_2D, tmp_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
					 img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		myTextures.push_back(tmp_tex);
	}

	GLuint
			m_GBufferTextures[GBufferTextureCount];
	GLenum
			drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
							 GL_COLOR_ATTACHMENT4, GL_DEPTH_ATTACHMENT};
	const GLenum m_GBufferTextureFormat[GBufferTextureCount] =
			{GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F};
	glGenTextures(GBufferTextureCount, m_GBufferTextures);

	for (int i = 0; i < GBufferTextureCount; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i],
					   m_nWindowWidth, m_nWindowHeight);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	GLuint
			fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	for (int i = 0; i < GDepth; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
	}
	glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDepth]);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GDepth], 0);
	glDrawBuffers(5, drawBuffers);
	std::cout << "check status : " << glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) << " complete = "
			  << GL_FRAMEBUFFER_COMPLETE << std::endl;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GLProgram shaderPass = buildProgram({loadShader("bin/shaders/deferred-renderer/ShadingPass.vs.glsl"),
										 loadShader("bin/shaders/deferred-renderer/ShadingPass.fs.glsl")});
	GLint id_uLightDir_vs = glGetUniformLocation(shaderPass.glId(), "uLightDir_vs");
	GLint gBuffer1 = glGetUniformLocation(shaderPass.glId(), "uGPosition");
	GLint gBuffer2 = glGetUniformLocation(shaderPass.glId(), "uGNormal");
	GLint gBuffer3 = glGetUniformLocation(shaderPass.glId(), "uGAmbient");
	GLint gBuffer4 = glGetUniformLocation(shaderPass.glId(), "uGDiffuse");
	GLint gBuffer5 = glGetUniformLocation(shaderPass.glId(), "uGlossyShininess");
	GLint idSHProjMatrix = glGetUniformLocation(shaderPass.glId(), "uDirLightViewProjMatrix");
	GLint idSM = glGetUniformLocation(shaderPass.glId(), "uDirLightShadowMap");
	GLint idBias = glGetUniformLocation(shaderPass.glId(), "uDirLightShadowMapBias");

	GLProgram prog = buildProgram({loadShader("bin/shaders/deferred-renderer/geometryPass.vs.glsl"),
								   loadShader("bin/shaders/deferred-renderer/geometryPass.fs.glsl")});
	GLint id_mvp = glGetUniformLocation(prog.glId(), "uMVPMatrix");
	GLint id_mv = glGetUniformLocation(prog.glId(), "uMVMatrix");
	GLint id_norm = glGetUniformLocation(prog.glId(), "uNormalMatrix");
	GLint id_uKd = glGetUniformLocation(prog.glId(), "uKd");
	GLint id_uKs = glGetUniformLocation(prog.glId(), "uKs");
	GLint id_uShininess = glGetUniformLocation(prog.glId(), "uShininess");
	GLint id_uLightIntensity = glGetUniformLocation(prog.glId(), "uLightIntensity");
	GLint id_tex = glGetUniformLocation(prog.glId(), "uTexture");
	GLint id_tex2 = glGetUniformLocation(prog.glId(), "uTexture2");
	GLint id_tex3 = glGetUniformLocation(prog.glId(), "uTexture3");
	GLint id_tex4 = glGetUniformLocation(prog.glId(), "uTexture4");

	glm::mat4 ProjMatrix, MVMatrix, ViewMatrix, NormalMatrix, MVPMatrix;
	const auto sceneDiagonalSize = glm::length(data.bboxMax - data.bboxMin);

	ProjMatrix = glm::perspective(glm::radians(70.f), m_nWindowWidth * 1.0f / m_nWindowHeight, 0.1f, 400.f);

	ViewController view(m_GLFWHandle.window(), 10.f);
	view.setSpeed(sceneDiagonalSize * 0.02f);


	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float shininess = 0.4f;
	auto KD = glm::vec3(0.5075, 0.5075, 0.5075);
	auto KS = glm::vec3(0.508273, 0.508273, 0.508273);
	auto intensity = glm::vec3(0.1922, 0.1922, 0.1922);

	int gSelect = (int) GPosition;

	GLuint
			quadVao = genQuad();
	const auto fbSize = m_GLFWHandle.framebufferSize();

	auto indexOffset = 0;

	static const auto computeDirectionVectorUp = [](float phiRadians, float thetaRadians)
	{
		const auto cosPhi = glm::cos(phiRadians);
		const auto sinPhi = glm::sin(phiRadians);
		const auto cosTheta = glm::cos(thetaRadians);
		return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
	};

	auto RENDER_FOR_SHADOW = [&]() {
		glBindVertexArray(scene.first);
		indexOffset = 0;
		int i = 0;
		for (const auto indexCount: data.indexCountPerShape) {
			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid *) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;
			i++;
		}
		glBindVertexArray(0);
	};
	auto MAIN_RENDER_SCENE = [&]() {
		glViewport(0, 0, fbSize.x, fbSize.y);
		MVMatrix = glm::mat4(1.0f);
		MVMatrix = glm::scale(MVMatrix, glm::vec3(0.1, 0.1, 0.1));
		ViewMatrix = view.getViewMatrix();
		MVMatrix = ViewMatrix * MVMatrix;
		NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
		MVPMatrix = ProjMatrix * MVMatrix;

		glUniformMatrix4fv(id_mvp, 1, GL_FALSE, glm::value_ptr(MVPMatrix));
		glUniformMatrix4fv(id_mv, 1, GL_FALSE, glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(id_norm, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
		glBindVertexArray(scene.first);
		indexOffset = 0;
		int i = 0;
		for (const auto indexCount: data.indexCountPerShape) {
			if (data.materials[data.materialIDPerShape[i]].KdTextureId != -1) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].KdTextureId]);
				glUniform3fv(id_uKd, 1, glm::value_ptr(glm::vec3(-1, -1, -1)));
			} else {
				glUniform3fv(id_uKd, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Kd));
			}
			if (data.materials[data.materialIDPerShape[i]].KsTextureId != -1) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].KsTextureId]);
				glUniform3fv(id_uKs, 1, glm::value_ptr(glm::vec3(-1, -1, -1)));
			} else {
				glUniform3fv(id_uKs, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Ks));
			}

			if (data.materials[data.materialIDPerShape[i]].KaTextureId != -1) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].KaTextureId]);
				glUniform3fv(id_uLightIntensity, 1, glm::value_ptr(glm::vec3(-1, -1, -1)));
			} else {
				glUniform3fv(id_uLightIntensity, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Ka));
			}

			if (data.materials[data.materialIDPerShape[i]].shininessTextureId != -1) {
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].shininessTextureId]);
				glUniform1f(id_uShininess, -1.f);
			} else {
				glUniform1f(id_uShininess, data.materials[data.materialIDPerShape[i]].shininess);
			}

			glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid *) (indexOffset * sizeof(GLuint)));
			indexOffset += indexCount;
			i++;
		}
		glBindVertexArray(0);
	};

	bool reloadShadowMap = true;
	float deg1 = 5.0, deg2 = 35.0;

	// Loop until the user closes the window
	for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount) {
		//std::cout << "frame " << iterationCount << std::endl;
		const auto seconds = glfwGetTime();
		glClearColor(1.0, 0.7, 0.7, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const auto sceneCenter = 0.5f * (data.bboxMin + data.bboxMax);
		const float sceneRadius = sceneDiagonalSize * 0.5f;


		auto lightDir = computeDirectionVectorUp(glm::radians(deg1), glm::radians(deg2));
		const auto dirLightViewMatrix = glm::lookAt(sceneCenter + lightDir * sceneRadius, sceneCenter, lightDir);
		const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);

		if(reloadShadowMap) {
			shadowMap.compute(dirLightProjMatrix, dirLightViewMatrix, RENDER_FOR_SHADOW);
			//reloadShadowMap = false;
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		// Put here rendering code
		glClearColor(1.0, 0.7, 0.7, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog.use();
		glUniform1i(id_tex, 0);
		glUniform1i(id_tex2, 1);
		glUniform1i(id_tex3, 2);
		glUniform1i(id_tex4, 3);

		MAIN_RENDER_SCENE();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		shaderPass.use();
		glUniform1i(gBuffer1, 0);
		glUniform1i(gBuffer2, 1);
		glUniform1i(gBuffer3, 2);
		glUniform1i(gBuffer4, 3);
		glUniform1i(gBuffer5, 4);
		glUniform3fv(id_uLightDir_vs, 1, glm::value_ptr(ViewMatrix * glm::vec4(lightDir, 1)));
		glBindSampler(5, shadowMap.m_directionalSMSampler);
		glUniform1i(idSM, 5);
		glUniform1f(idBias, 0.0);
		const auto rcpViewMatrix = view.getRcpViewMatrix(); // Inverse de la view matrix de la caméra
		glUniformMatrix4fv(idSHProjMatrix, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix * rcpViewMatrix));



		glBindVertexArray(quadVao);
		for (int i = 0; i < GDepth; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		}
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowMap.m_directionalSMTexture);
		glBindSampler(5, shadowMap.m_directionalSMSampler);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		for (int i = 0; i < GDepth; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glmlv::imguiNewFrame();
		{
			ImGui::Begin("GUI");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
						ImGui::GetIO().Framerate);
			ImGui::Separator();
			ImGui::SliderFloat("shininess", &shininess, 0.01f, 40.0f, "%.3f");
			ImGui::ColorEdit3("KD", glm::value_ptr(KD));
			ImGui::ColorEdit3("KS", glm::value_ptr(KS));
			ImGui::ColorEdit3("Intensity", glm::value_ptr(intensity));
			ImGui::Separator();
			ImGui::Text(glm::to_string(glm::vec3(glm::inverse(ViewMatrix) * glm::vec4(0, 0, 0, 1))).c_str());
			ImGui::Separator();
			ImGui::Text(("Number vertex : " + std::to_string(indexOffset)).c_str());
			ImGui::Separator();
			ImGui::SliderInt("colorAttach", &gSelect, 0, GBufferTextureCount);
			ImGui::SliderFloat("Deg1", &deg1, 0, 360);
			ImGui::SliderFloat("Deg2", &deg2, 0, 360);
			ImGui::End();
		}

		glmlv::imguiRenderFrame();

		m_GLFWHandle.swapBuffers(); // Swap front and back buffers

		glfwPollEvents(); // Poll for and process events

		auto ellapsedTime = glfwGetTime() - seconds;
		auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		if (!guiHasFocus) {
			view.update(ellapsedTime);
			if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_ESCAPE)) {
				exit(EXIT_SUCCESS);
			}
		}
	}

	return 0;
}

Application::Application(int argc, char **argv) :
		m_AppPath{glmlv::fs::path{argv[0]}},
		m_AppName{m_AppPath.stem().string()},
		m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
		m_ShadersRootPath{m_AppPath.parent_path() / "shaders"} {
	//ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, nullptr);

}
