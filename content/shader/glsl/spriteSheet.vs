#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) out vec2 TexCoord;
uniform vec4 uClipSpacePosSize;
uniform vec4 uScaleOffSet;

void main()
{
	gl_Position = vec4((aPos.xy * uClipSpacePosSize.zw + uClipSpacePosSize.xy) * 2.0 - 1.0, 1.0, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y) * uScaleOffSet.xy + uScaleOffSet.zw;
}