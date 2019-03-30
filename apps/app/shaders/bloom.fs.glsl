#version 330

in vec2 vTexCoords;

layout (location = 0) out vec3 fBrightColor;

uniform sampler2D uTexture_lum;
uniform bool hori;

void main() {
    vec2 tsize = 1.0 / textureSize(uTexture_lum, 0);
    float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

    vec3 r = texture(uTexture_lum, vTexCoords).xyz ;
    r *= weight[0];
    if(hori) {
        for(int i = 1; i < 5; ++i)
        {
            r += texture(uTexture_lum, vTexCoords + vec2(tsize.x * i, 0)).xyz * weight[i];
            r += texture(uTexture_lum, vTexCoords - vec2(tsize.x * i, 0)).xyz * weight[i];
        }
    }
    else {
        for(int i = 1; i < 5; ++i)
        {
            r += texture(uTexture_lum, vTexCoords + vec2(0, tsize.y * i)).xyz * weight[i];
            r += texture(uTexture_lum, vTexCoords - vec2(0, tsize.y * i)).xyz * weight[i];
        }
    }
    fBrightColor = r;
}
