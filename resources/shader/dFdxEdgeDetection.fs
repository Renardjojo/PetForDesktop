#version 330 core

// based on https://www.shadertoy.com/view/Xly3DV
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D uTexture;

void main()
{
    vec3 dy = dFdy(texture(uTexture, TexCoord).rgb);
    float diffY = (abs(dy.x) + abs(dy.y) + abs(dy.z)) / 3.0;
    FragColor = vec4(diffY > 0.05);
}