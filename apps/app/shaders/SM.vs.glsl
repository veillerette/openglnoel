#version 330

layout(location = 0) in vec3 aPosition;
uniform mat4 uDirLightViewProjMatrix;
uniform mat4 uSkinMatrix;

void main()
{
    gl_Position = uDirLightViewProjMatrix * uSkinMatrix * vec4(aPosition, 1);
}