#include "Application.hpp"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glmlv;

template<typename T>
std::pair<GLuint, unsigned int> constructObject(T & geometry) {
	GLuint vbo, ibo, vao;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3f3f2f) * geometry.vertexBuffer.size(), geometry.vertexBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.indexBuffer.size() * sizeof(uint32_t),geometry.indexBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f), (const GLvoid*)offsetof(Vertex3f3f2f, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f),  (const GLvoid*)offsetof(Vertex3f3f2f, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3f3f2f),  (const GLvoid*)offsetof(Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return std::pair<GLuint, unsigned int>(vao, geometry.indexBuffer.size());
}

GLuint importImage(const fs::path &path) {
	GLuint tex;
	std::cout << "import image..." << std::endl;
	Image2DRGBA img = readImage(path);
	std::cout << "image imported !" << img.width() << " " << img.height() << std::endl;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	img.width(), img.height(), 0, GL_RGBA,  GL_UNSIGNED_BYTE, img.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::cout << "texture ok !" << std::endl;

	return tex;
}

int Application::run()
{
	auto geo1 = makeCube();
	auto geo2 = makeSphere(32);
	auto cube = constructObject(geo1);
	auto sphere = constructObject(geo2);
	SceneData data;
	loadAssimpScene("../scenes/assets/sponza.obj", data, true);
	auto scene = constructObject(data);

	std::vector<GLuint> myTextures;
	for(auto & img : data.textures) {
		GLuint tmp_tex;
		glGenTextures(1, &tmp_tex);
		glBindTexture(GL_TEXTURE_2D, tmp_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		img.width(), img.height(), 0, GL_RGBA,  GL_UNSIGNED_BYTE, img.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		myTextures.push_back(tmp_tex);
	}

	GLProgram prog = buildProgram({loadShader("bin/shaders/forward-renderer/forward.vs.glsl"), loadShader("bin/shaders/forward-renderer/forward.fs.glsl")});
	GLint id_mvp = glGetUniformLocation(prog.glId(), "uMVPMatrix");
	GLint id_mv = glGetUniformLocation(prog.glId(), "uMVMatrix");
	GLint id_norm = glGetUniformLocation(prog.glId(), "uNormalMatrix");
	GLint id_uKd = glGetUniformLocation(prog.glId(), "uKd");
	GLint id_uKs = glGetUniformLocation(prog.glId(), "uKs");
	GLint id_uShininess = glGetUniformLocation(prog.glId(), "uShininess");
	GLint id_uLightDir_vs = glGetUniformLocation(prog.glId(), "uLightDir_vs");
	GLint id_uLightIntensity = glGetUniformLocation(prog.glId(), "uLightIntensity");
	GLint id_tex = glGetUniformLocation(prog.glId(), "uTexture");
	GLint id_tex2 = glGetUniformLocation(prog.glId(), "uTexture2");
	GLint id_tex3 = glGetUniformLocation(prog.glId(), "uTexture3");
	GLint id_tex4 = glGetUniformLocation(prog.glId(), "uTexture4");
	glm::mat4 ProjMatrix, MVMatrix, ViewMatrix, NormalMatrix, MVPMatrix;
	const auto sceneDiagonalSize = glm::length(data.bboxMax - data.bboxMin);

	ProjMatrix = glm::perspective(glm::radians(70.f), m_nWindowWidth * 1.0f / m_nWindowHeight, 0.1f, 400.f);
	prog.use();

	ViewController view(m_GLFWHandle.window(), 10.f);
	view.setSpeed(sceneDiagonalSize * 0.1f);


	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float shininess = 0.4f;
	auto KD = glm::vec3(0.5075, 0.5075, 0.5075);
	auto KS = glm::vec3(0.508273, 0.508273, 0.508273);
	auto intensity = glm::vec3(0.1922, 0.1922, 0.1922);

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

		//////////////////////////////////
		glUniform1i(id_tex, 0);
		glUniform1i(id_tex2, 1);
		glUniform1i(id_tex3, 2);
		glUniform1i(id_tex4, 3);

		glUniform3fv(id_uLightDir_vs, 1, glm::value_ptr(glm::vec3(ViewMatrix * (glm::rotate(glm::mat4(1.f), (float)(seconds), glm::vec3(1,1,0)) * glm::vec4(5, 111, 5, 1)))));
		/////////////////////////////////


	        // Put here rendering code
			const auto fbSize = m_GLFWHandle.framebufferSize();
			glViewport(0, 0, fbSize.x, fbSize.y);
			glClearColor(1.0, 0.7, 0.7, 0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
			auto indexOffset = 0;
			int i = 0;
			for (const auto indexCount: data.indexCountPerShape)
			{
				if(data.materials[data.materialIDPerShape[i]].KdTextureId != -1) {
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,myTextures[data.materials[data.materialIDPerShape[i]].KdTextureId]);
					glUniform3fv(id_uKd, 1, glm::value_ptr(glm::vec3(-1,-1,-1)));
				} else {
					glUniform3fv(id_uKd, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Kd));
				}
				if(data.materials[data.materialIDPerShape[i]].KsTextureId != -1) {
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D,myTextures[data.materials[data.materialIDPerShape[i]].KsTextureId]);
					glUniform3fv(id_uKs, 1, glm::value_ptr(glm::vec3(-1,-1,-1)));
				} else {
					glUniform3fv(id_uKs, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Ks));
				}

				if(data.materials[data.materialIDPerShape[i]].KaTextureId != -1) {
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].KaTextureId]);
					glUniform3fv(id_uLightIntensity, 1, glm::value_ptr(glm::vec3(-1,-1,-1)));
				} else {
						glUniform3fv(id_uLightIntensity, 1, glm::value_ptr(data.materials[data.materialIDPerShape[i]].Ka));
				}

				if(data.materials[data.materialIDPerShape[i]].shininessTextureId != -1) {
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, myTextures[data.materials[data.materialIDPerShape[i]].shininessTextureId]);
					glUniform1f(id_uShininess, -1.f);
				} else {
						glUniform1f(id_uShininess, data.materials[data.materialIDPerShape[i]].shininess);
				}




			    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
			    indexOffset += indexCount;
				i++;
			}
			glBindVertexArray(0);



		/*MVMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4));
		MVMatrix = glm::rotate(MVMatrix, (float)seconds, glm::vec3(0, 1, 0));
		ViewMatrix = view.getViewMatrix();
		MVMatrix = ViewMatrix * MVMatrix;
		NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
		MVPMatrix = ProjMatrix * MVMatrix;

		glUniformMatrix4fv(id_mvp, 1, GL_FALSE, glm::value_ptr(MVPMatrix));
		glUniformMatrix4fv(id_mv, 1, GL_FALSE, glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(id_norm, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

		glBindVertexArray(sphere.first);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawElements(GL_TRIANGLES, sphere.second, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		*/



        // GUI code:
		glmlv::imguiNewFrame();
        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Separator();
			ImGui::SliderFloat("shininess", &shininess, 0.01f, 40.0f, "%.3f");
			ImGui::ColorEdit3("KD", glm::value_ptr(KD));
			ImGui::ColorEdit3("KS", glm::value_ptr(KS));
			ImGui::ColorEdit3("Intensity", glm::value_ptr(intensity));
			ImGui::Separator();
			ImGui::Text(glm::to_string(glm::vec3(glm::inverse(ViewMatrix) * glm::vec4(0,0,0,1))).c_str());
			ImGui::Separator();
			ImGui::Text(("Number vertex : "+std::to_string(indexOffset)).c_str());
			ImGui::End();
        }

		glmlv::imguiRenderFrame();

        glfwPollEvents(); // Poll for and process events

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            view.update(ellapsedTime);
			if(glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_ESCAPE)) {
				exit(EXIT_SUCCESS);
			}
        }



		m_GLFWHandle.swapBuffers(); // Swap front and back buffers
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
	glEnable(GL_MULTISAMPLE);
    //ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file


}
