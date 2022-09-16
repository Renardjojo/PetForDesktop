#version 330 core

// based on https://www.shadertoy.com/view/Xly3DV
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D uTexture;

void main()
{
    FragColor = vec4(pow(texture(uTexture, TexCoord).rgb, vec3(2.2)), 0.0);
}