#ifndef DEF_HEADER_SHADER
#define DEF_HEADER_SHADER


#include <glm/glm.hpp>
#include <glmlv/GLProgram.hpp>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

class Shader {
public:
    Shader(const char *vertexPath, const char *fragmentPath);

    ~Shader();

    void use();

    void uniformMatrix(std::string key, glm::mat4 & mat);
    void uniformMatrix(std::string key, glm::mat3 & mat);

    void uniformValue(std::string key, GLfloat value);
    void uniformValue(std::string key, GLint value);

    void uniformVector(std::string key, glm::vec3 vec);
    void uniformVector(std::string key, glm::vec4 vec);

private:
	glmlv::GLProgram program;
    std::unordered_map<std::string, GLint> locations;

    GLint getLocation(std::string & location);
};


#endif // DEF_HEADER_SHADER
