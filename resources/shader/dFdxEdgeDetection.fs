#version 330 core

// based on https://www.shadertoy.com/view/Xly3DV
out float FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D uTexture;
uniform vec2 resolution;

void main()
{
    // avoid sample but loose too much data
    //vec3 dy = dFdy(texture(uTexture, TexCoord).rgb);
    //float diffY = (abs(dy.x) + abs(dy.y) + abs(dy.z)) / 3.0;
    //FragColor = float(diffY > 0.05);

    vec3 col = texture(uTexture, TexCoord).rgb;
    vec2 uvColY = vec2(TexCoord.x, TexCoord.y + 1.0 / resolution.y);
    vec3 colY = texture(uTexture, uvColY).rgb;
    vec3 dy = colY - col;
    float diffY = (abs(dy.x) + abs(dy.y) + abs(dy.z)) / 3.0;
    FragColor = float(diffY > 0.05);
}