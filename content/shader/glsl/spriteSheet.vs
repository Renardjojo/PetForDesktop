#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) out vec2 TexCoord;
layout (binding = 0) uniform ScreenParam
{
  vec4 uClipSpacePosSize;
  vec4 uScaleOffSet;
} screenParam;

void main()
{
	gl_Position = vec4((aPos.xy * screenParam.uClipSpacePosSize.zw + screenParam.uClipSpacePosSize.xy) * 2.0 - 1.0, 1.0, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y) * screenParam.uScaleOffSet.xy + screenParam.uScaleOffSet.zw;
}