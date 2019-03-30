#ifndef OPENGLNOEL_BLOOM_H
#define OPENGLNOEL_BLOOM_H

#include "Window.h"
#include "Shader.h"
#include "Geometry.h"

#define ITER_DEFAULT 5
#define POWER_BLOOM 9

class Bloom {
public:
	Bloom(Window *win, unsigned int iter);
	~Bloom();
	void bind();
	void render();
	void gui();
private:
	int iter;
	Window *window;
	const GLuint colors[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	GLuint fbo, tex[2], rbo;
	GLuint fboblur[2], texblur[2];

	Shader *screenShader;
	Shader *screenFinal;
	const int textureWidth = 1244;
	const int textureHeight = 700;

	Geometry *quad;

	void init();
	void initBlur();
	void genLinearTexture(GLuint tex, GLint format, GLuint i, int width, int height);
};


#endif //BLOOM_H
