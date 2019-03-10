#version 330

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;
in mat4 vNormalMatrix;

layout (location = 0) out vec3 fFragColor;

uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform vec3 uLightPos_vs;
uniform vec3 uLightIntensity;

uniform sampler2D uTexture_day;
uniform sampler2D uTexture_night;
uniform sampler2D uTexture_clouds;

uniform float timer;

vec3 blinnPhong(int way)
{
	vec3 w0 = normalize(-vPosition_vs);
	vec3 wi = normalize(uLightPos_vs-vPosition_vs);
	vec3 halfv = normalize((w0 + wi) / 2);

	return (normalize(uKd) *
		(dot(normalize(wi),normalize(vNormal_vs))) +
		normalize(uKs) * (pow(dot(halfv, normalize(vNormal_vs)), uShininess)));
}

void main() {
	vec3 a = blinnPhong(1);
	vec3 b = blinnPhong(-1);
	float d = timer;
	vec2 tex = vTexCoords;
	tex.x += d;
	fFragColor =  (vec3(texture(uTexture_day, vTexCoords)) * (1-vec3(texture(uTexture_clouds, tex)))
	    + vec3(texture(uTexture_clouds, tex))) * a
	    + vec3(texture(uTexture_night, vTexCoords)) * (1-a);
    fBrightColor = vec3(0,0,0);
}
