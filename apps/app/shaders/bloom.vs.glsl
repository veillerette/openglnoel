#version 330 core

layout (location = 0) in vec2 aVertexPosition;
layout (location = 2) in vec2 aVertexTexCoords;

out vec2 vTexCoords;

void main()
{
    vTexCoords = aVertexTexCoords;
    gl_Position = vec4(aVertexPosition.x, aVertexPosition.y, 0.0, 1.0);
}

