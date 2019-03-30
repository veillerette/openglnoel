#ifndef OPENGLNOEL_GEOMETRY_H
#define OPENGLNOEL_GEOMETRY_H

#include <glm/glm.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/simple_geometry.hpp>

class Geometry {
public:
	~Geometry();

	static Geometry *generateQuad();


	template<typename T>
	static Geometry *constructObject(T &geometry);

	void enable();

	void disable();

	void draw();

private:
	GLuint vbo, ibo, vao;
	unsigned int size;

	Geometry(GLuint vbo, GLuint ibo, GLuint vao, unsigned int size);
};


template<typename T>
Geometry *Geometry::constructObject(T &geometry) {
	GLuint
			vbo, ibo, vao;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glmlv::Vertex3f3f2f) * geometry.vertexBuffer.size(), geometry.vertexBuffer.data(),
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f),
						  (const GLvoid *) offsetof(glmlv::Vertex3f3f2f, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f),
						  (const GLvoid *) offsetof(glmlv::Vertex3f3f2f, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f),
						  (const GLvoid *) offsetof(glmlv::Vertex3f3f2f, texCoords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return new Geometry(vbo, ibo, vao, geometry.indexBuffer.size());
}


#endif //GEOMETRY_H
