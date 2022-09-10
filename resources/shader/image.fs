#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, TexCoord);
}