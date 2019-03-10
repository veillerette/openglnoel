#version 330

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;
in mat4 vNormalMatrix;

out vec3 fFragColor;

uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform vec3 uLightDir_vs;
uniform vec3 uLightIntensity;

uniform sampler2D uTexture;
uniform sampler2D uTexture2;
uniform sampler2D uTexture3;
uniform sampler2D uTexture4;

vec3 blinnPhong(vec3 d, vec3 s, vec3 a, float sh)
{
	vec3 w0 = normalize(-vPosition_vs);
	vec3 wi = normalize(uLightDir_vs-vPosition_vs);
	vec3 halfv = normalize((w0 + wi) / 2);
	return a * (normalize(d) *
			dot(normalize(wi),normalize(vNormal_vs)) +
			normalize(a) * (pow(dot(halfv, normalize(vNormal_vs)), sh)));
}

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
	fFragColor = blinnPhong(d, s, a, sh);
//	fFragColor = vNormal_vs;
}
