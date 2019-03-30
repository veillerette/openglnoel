#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexCoords;
layout(location = 3) in vec3 aVertexTangent;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uSkinMatrix;

out vec3 vPosition_vs;
out vec3 vNormal_vs;
out vec2 vTexCoords;
out mat4 vNormalMatrix;
out vec3 vTangent;

void main() {
    vec4 vertexPosition = uSkinMatrix *vec4(aVertexPosition, 1);
    vec4 vertexNormal = uSkinMatrix *vec4(aVertexNormal, 0);

    vPosition_vs = vec3(uMVMatrix * vertexPosition);
    vNormal_vs = vec3(uNormalMatrix * vertexNormal);
    vNormalMatrix = uMVMatrix;

    vTexCoords = aVertexTexCoords;
    vTangent = aVertexTangent;

    gl_Position = uMVPMatrix * vertexPosition;
}
