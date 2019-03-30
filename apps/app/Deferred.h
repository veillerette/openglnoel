#ifndef OPENGLNOEL_DEFERRED_H
#define OPENGLNOEL_DEFERRED_H

#include "Window.h"
#include "Geometry.h"
#include "Shader.h"
#include "Geometry.h"
#include "ShadowMap.h"
#include "TrackballCamera.h"
#include "Bloom.h"
#include "Skin.h"

enum GBufferTextureType {
	GPosition = 0,
	GNormal, //1
	GAmbient, //2
	GDiffuse, //3
	GGlossyShininess, //4
	GNormalTexture, //5
	GTangent, //6
	GDepth,
	GBufferTextureCount
};

class Deferred {
public:
	Deferred(Window *window, ShadowMap *map);

	~Deferred();

	Shader *getGeometryShader();

	Shader *getShadingShader();

	void render(std::function <void()> render, std::function <void()> simpleRender, TrackballCamera *cam);

	void gui(Skin & animData);

private:
	Window *window;
	GLuint m_GBufferTextures[GBufferTextureCount];
	GLuint fbo;

	static const GLenum drawBuffers[];

	static const GLenum m_GBufferTextureFormat[GBufferTextureCount];

	Shader *shaderPass;
	Shader *geometryPass;

	float gamma;

	Geometry *quad;
	Bloom *bloom;
	float powerBloom;
	float metal;
	float rough;


	glm::vec3 light;
	bool reloadSM;
	ShadowMap *shadowMap;
	float smBias;
	int smRes;

	int what;

	void initTextures();
	void initFramebuffer();
	void geometryRender(std::function<void()> & render, TrackballCamera *cam);
	void shadingRender(glm::vec3 light, TrackballCamera *cam);
	void renderSpecific(int i);
};


#endif //DEFERRED_H
