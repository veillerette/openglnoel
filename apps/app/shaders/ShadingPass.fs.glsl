#version 330


layout (location = 0) out vec3 fFragColor;
layout (location = 1) out vec3 fBrightColor;


uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;
uniform sampler2D uNormalMaps;
uniform sampler2D uTangent;

uniform vec3 uLightDir_vs;
uniform float uGamma;
uniform float uBloomPower;
uniform float uMetallic;
uniform float uRoughness;

uniform mat4 uDirLightViewProjMatrix;
uniform sampler2DShadow uDirLightShadowMap;
uniform float uDirLightShadowMapBias;


const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
const vec3 black = vec3(0,0,0);
float pi = 3.14159265;

float lightCompute(vec3 vPosition_vs, vec3 vNormal_vs, mat3 TBN)
{
    vec3 w0 = normalize(-vPosition_vs);
	vec3 wi = normalize(uLightDir_vs);
	vec3 halv = TBN * normalize((w0+wi)/2);

	return max(dot(normalize(wi),normalize(TBN * vNormal_vs)), 0.0);
}

float Schlick(vec3 v, float k, vec3 N) {
    float up = dot(v, N);
    float down = (up * (1 - k)) + k;
    return up / down;
}

float Trowbridge(float alpha, vec3 N, vec3 H) {
    float up = pow(alpha, 2);
    float down = pi * pow((pow(dot(N,H), 2) * (up - 1)) + 1, 2);
    return up / down;
}

vec3 BRDF(mat3 TBN, vec3 vNormal_vs, vec3 vPosition_vs, vec3 d, float metallic, float roughness) {
    vec3 V = normalize(-vPosition_vs);
	vec3 L = normalize(uLightDir_vs);
	vec3 H = normalize((V+L) / 2);
	vec3 N = normalize(TBN * vNormal_vs);

	vec3 c_diff = mix(d.rgb * (1 - dielectricSpecular.r), black, metallic);
    vec3 F0 = mix(dielectricSpecular, d.rgb, metallic);
    float alpha = pow(roughness, 2);

	vec3 F = F0 + (1 - F0) * pow((1.0 - dot(V,H)), 5);
	float k = roughness * sqrt(2/pi);
    float G = Schlick(L, k, N) * Schlick(V, k, N);
    float D = Trowbridge(alpha, N, H);
    vec3 diffuse = c_diff / pi;

    vec3 fdiffuse = (1 - F) * diffuse;
    vec3 fspecUp = F * G * D;
    float fspecDown = 4 * (dot(N,L) * dot(N, V));
    vec3 fspec = fspecUp / fspecDown;

    return fdiffuse + fspec;
}

float computeVisibility(vec3 pos) {
    vec4 positionInDirLightScreen = uDirLightViewProjMatrix * vec4(pos, 1);
    vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5;
    float dirLightVisibility = textureProj(uDirLightShadowMap, vec4(positionInDirLightNDC.xy, positionInDirLightNDC.z - uDirLightShadowMapBias, 1.0), 0.1);

    return dirLightVisibility;
}

vec3 get_orthonormal(vec3 N) {
	if(N.x != 0 && N.y != 0)
		return normalize(vec3(-N.y, N.x, 0));
	return vec3(N.z, 0, 0);
}

void main() {
	vec3 pos = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));

    if(pos.r == 2 && pos.g == 1 && pos.b == 1) {
        fFragColor = vec3(0.3, 0.4, 0.5);
        fBrightColor = vec3(0);//vec3(0.3, 0.4, 0.5)*5;
        return;
    }

	vec3 N = vec3(texelFetch(uNormalMaps, ivec2(gl_FragCoord.xy), 0));
    vec3 T = vec3(texelFetch(uTangent, ivec2(gl_FragCoord.xy), 0));
	N = (N * 2.0 - 1.0);
	mat3 TBN;

	if(length(N) == 0 || length(T) == 0)
		TBN = mat3(1);
	else {
		N = normalize(N);
		vec3 B = normalize(cross(N, T));

		TBN = transpose(mat3(T, B, N));
	}

	vec3 norm = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));
    // recup composantes from deferred rendering
	vec3 d = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));
	vec3 a = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));

	vec4 tmp = texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0);
	vec3 color = BRDF(TBN, norm, pos, d, uMetallic, uRoughness) * lightCompute(pos, norm, TBN);
	color = a + computeVisibility(pos) * color;
	fFragColor = pow(color, vec3(1.0 / uGamma));
	fBrightColor = a * uBloomPower;
//	fFragColor = vNormal_vs;
}
