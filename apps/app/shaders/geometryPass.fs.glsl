#version 330

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;
in mat4 vNormalMatrix;
in vec3 vTangent;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;
layout(location = 5) out vec3 fNormalMaps;
layout(location = 6) out vec3 fTangent;

uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uKa;
uniform float uShininess;
uniform vec3 uLightIntensity;

uniform sampler2D uTexture; // diffuse
uniform sampler2D uTexture2; // spec
uniform sampler2D uTexture3; // ambiant
uniform sampler2D uTexture4; // glossy
uniform sampler2D uTexture5; // normal maps

void main() {
	vec3 d = uKd;
	vec4 s;
	vec3 a = uKa;
	float sh = uShininess;
	vec3 nm;
	if(uKd.r == -1) {
	    d = vec3(texture(uTexture, vTexCoords));
	} else {
	    d = uKd;
	}
    s = vec4(texture(uTexture2, vTexCoords));
    a = vec3(texture(uTexture3, vTexCoords));
    nm = vec3(texture(uTexture5, vTexCoords));

	fPosition = vPosition_vs;
	fNormal = vNormal_vs;
	fAmbient = a;
	fDiffuse = d;
	fGlossyShininess = texture(uTexture4, vTexCoords);
	fNormalMaps = nm;
	fTangent = vTangent;
}
