static float4 gl_Position;
static float3 aPos;
static float2 TexCoord;
static float2 aTexCoord;

struct Input
{
    float3 aPos : POSITION;
    float2 aTexCoord : TEXCOORD;
};

struct Output
{
    float2 TexCoord : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = float4(aPos.xy, 1.0f, 1.0f);
    TexCoord = aTexCoord;
}

Output main(Input stage_input)
{
    aPos = stage_input.aPos;
    aTexCoord = stage_input.aTexCoord;
    vert_main();
    Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.TexCoord = TexCoord;
    return stage_output;
}
