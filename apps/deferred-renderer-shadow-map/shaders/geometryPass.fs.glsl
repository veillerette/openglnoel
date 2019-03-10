#version 330

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;
in mat4 vNormalMatrix;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform vec3 uLightIntensity;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;

void main() {
	vec3 d = uKd;
	vec3 s = uKs;
	vec3 a = uLightIntensity;
	float sh = uShininess;
	if(d.r == -1) {
		d = vec3(texture(uTexture, vTexCoords));
	}
	if(s.r == -1) {
		s = vec3(texture(uTexture2, vTexCoords));
	}
	if(a.r == -1) {
		a = vec3(texture(uTexture3, vTexCoords));
	}
	if(sh == -1) {
		sh = dot(vec3(texture(uTexture4, vTexCoords)), vec3(0.33, 0.33, 0.33));
	}

	fPosition = vPosition_vs;
	fNormal = vNormal_vs;
	fAmbient = a;
	fDiffuse = d;
	fGlossyShininess = vec4(s, sh);
}
