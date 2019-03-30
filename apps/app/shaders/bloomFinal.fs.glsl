#version 330

in vec2 vTexCoords;

layout (location = 0) out vec3 fFragColor;

uniform sampler2D uTexture;
uniform sampler2D uTexture_lum;

void main() {
    fFragColor = texture(uTexture, vTexCoords).xyz + texture(uTexture_lum, vTexCoords).xyz;
}
