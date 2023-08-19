Texture2D<float4> uTexture : register(t0);
SamplerState _uTexture_sampler : register(s0);

static float4 FragColor;
static float2 TexCoord;

struct Input
{
    float2 TexCoord : TEXCOORD0;
};

struct Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = uTexture.Sample(_uTexture_sampler, TexCoord);
}

Output main(Input stage_input)
{
    TexCoord = stage_input.TexCoord;
    frag_main();
    Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
