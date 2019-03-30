#include "Bloom.h"



Bloom::Bloom(Window *win, unsigned int iter) :
		iter(iter), window(win){
	quad = Geometry::generateQuad();
	screenShader = new Shader("bin/shaders/app/bloom.vs.glsl", "bin/shaders/app/bloom.fs.glsl");
	screenFinal = new Shader("bin/shaders/app/bloom.vs.glsl", "bin/shaders/app/bloomFinal.fs.glsl");
	init();
}

Bloom::~Bloom() {

}

void Bloom::init() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(2, tex);
	genLinearTexture(tex[0], GL_RGB, 0, window->getWidth(), window->getHeight());
	genLinearTexture(tex[1], GL_RGB16F, 1, window->getWidth(), window->getHeight());

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window->getWidth(), window->getHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glDrawBuffers(2, colors);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	initBlur();
}

void Bloom::initBlur() {
	glGenFramebuffers(2, fboblur);
	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboblur[i]);
		glGenTextures(1, &texblur[i]);
		glBindTexture(GL_TEXTURE_2D, texblur[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, colors[0], GL_TEXTURE_2D, texblur[i], 0);
	}
	GLuint rboblur;
	glGenRenderbuffers(1, &rboblur);
	glBindRenderbuffer(GL_RENDERBUFFER, rboblur);
	glDrawBuffers(1, colors);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Bloom::genLinearTexture(GLuint tex, GLint format, GLuint i, int width, int height) {
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, colors[i], GL_TEXTURE_2D, tex, 0);
}

void Bloom::bind() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Bloom::render() {
	for(int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboblur[0]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	quad->enable();
	for (int i = 0; i < iter; i++) {
		for (int j = 0; j < 2; j++) {
			glBindFramebuffer(GL_FRAMEBUFFER, fboblur[j]);
			glViewport(0, 0, textureWidth, textureHeight);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glDisable(GL_MULTISAMPLE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			screenShader->use();
			screenShader->uniformValue("uTexture_lum", 0);
			if (j == 0) {
				screenShader->uniformValue("hori", 1);
			} else {
				screenShader->uniformValue("hori", 0);
			}
			if (i == 0 && j == 0) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tex[1]);
			} else {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texblur[1 - j]);
			}
			quad->draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glViewport(0, 0, window->getWidth(), window->getHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	screenFinal->use();
	screenFinal->uniformValue("uTexture", 0);
	screenFinal->uniformValue("uTexture_lum", 1);
	quad->enable();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glActiveTexture(GL_TEXTURE1);
	if(iter > 0) {
		glBindTexture(GL_TEXTURE_2D, texblur[1]);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	quad->draw();
	quad->disable();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Bloom::gui() {
	ImGui::SliderInt("iterations", &iter, 0, 10);
}
