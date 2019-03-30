#include "Shader.h"

#include <string>

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
	program = glmlv::buildProgram({glmlv::loadShader(vertexPath),
								glmlv::loadShader(fragmentPath)});

	use();
}

Shader::~Shader() {
}

GLint Shader::getLocation(std::string & location) {
    if(locations.find(location) == locations.end()) {
        locations[location] = glGetUniformLocation(program.glId(), location.c_str());
    }
    return locations[location];
}

void Shader::use() {
    program.use();
}

void Shader::uniformMatrix(std::string key, glm::mat4 & mat) {
    glUniformMatrix4fv(getLocation(key), 1, GL_FALSE, glm::value_ptr(mat));

}

void Shader::uniformValue(std::string key, GLfloat value) {
    glUniform1f(getLocation(key), value);
}

void Shader::uniformValue(std::string key, GLint value) {
    glUniform1i(getLocation(key), value);
}

void Shader::uniformMatrix(std::string key, glm::mat3 & mat) {
    glUniformMatrix3fv(getLocation(key), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::uniformVector(std::string key, glm::vec3 vec) {
    glUniform3fv(getLocation(key), 1, glm::value_ptr(vec));
}

void Shader::uniformVector(std::string key, glm::vec4 vec) {
    glUniform4fv(getLocation(key), 1, glm::value_ptr(vec));
}