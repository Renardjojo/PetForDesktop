#version 460 core
layout (location = 0)  out vec4 FragColor;
layout (location = 0) in vec2 TexCoord;

// texture samplers
layout (binding = 0) uniform sampler2D uTexture;

void main()
{
    FragColor = vec4(texture(uTexture, TexCoord).r);
}