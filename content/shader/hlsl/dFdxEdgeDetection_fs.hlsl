cbuffer TextureResolution : register(b1)
{
    float2 resolution_resolution : packoffset(c0);
};

Texture2D<float4> uTexture : register(t0);
SamplerState _uTexture_sampler : register(s0);

static float2 TexCoord;
static float FragColor;

struct Input
{
    float2 TexCoord : TEXCOORD0;
};

struct Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    float3 col = uTexture.Sample(_uTexture_sampler, TexCoord).xyz;
    float2 uvColY = float2(TexCoord.x, TexCoord.y + (1.0f / resolution_resolution.y));
    float3 colY = uTexture.Sample(_uTexture_sampler, uvColY).xyz;
    float3 dy = colY - col;
    float diffY = ((abs(dy.x) + abs(dy.y)) + abs(dy.z)) / 3.0f;
    FragColor = float(diffY > 0.0500000007450580596923828125f);
}

Output main(Input stage_input)
{
    TexCoord = stage_input.TexCoord;
    frag_main();
    Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
