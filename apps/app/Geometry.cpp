#include "Geometry.h"

Geometry *Geometry::generateQuad() {
	GLuint
			vbo, ibo, vao;

	GLfloat vertex[16] = {-1, -1, 0, 0,
					  	  1, -1, 1, 0,
					  	  -1, 1, 0, 1,
					  	  1, 1, 1, 1};
	GLuint indices[6] = {0, 1, 2, 1, 2, 3};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 16, vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(sizeof(GLfloat) * 2));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return new Geometry(vbo, ibo, vao, 6);
}

Geometry::~Geometry() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &ibo);
}

Geometry::Geometry(GLuint vbo, GLuint ibo, GLuint vao, unsigned int size) : vbo(vbo), ibo(ibo), vao(vao), size(size) {

}

void Geometry::enable() {
	glBindVertexArray(vao);
}

void Geometry::disable() {
	glBindVertexArray(0);
}

void Geometry::draw() {
	enable();
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
	disable();
}
