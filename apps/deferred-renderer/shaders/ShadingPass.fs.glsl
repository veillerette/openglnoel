#version 330

out vec3 fFragColor;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uLightDir_vs;

vec3 blinnPhong(vec3 d, vec3 s, vec3 a, float sh, vec3 vPosition_vs, vec3 vNormal_vs)
{
	vec3 w0 = normalize(-vPosition_vs);
	vec3 wi = normalize(uLightDir_vs-vPosition_vs);
	vec3 halfv = normalize((w0 + wi) / 2);
	return a * (normalize(d) *
			dot(normalize(wi),normalize(vNormal_vs)) +
			normalize(a) * (pow(dot(halfv, normalize(vNormal_vs)), sh)));
}

void main() {
	vec3 d = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));
	vec3 a = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));
	vec4 tmp = texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0);
	float sh = tmp.a;
	vec3 s = tmp.rgb;
	vec3 pos = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));
	vec3 norm = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));
	fFragColor = blinnPhong(d, s, a, sh, pos, norm);
//	fFragColor = vNormal_vs;
}
