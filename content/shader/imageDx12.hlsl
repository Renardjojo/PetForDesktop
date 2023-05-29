struct VertexInput
{
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput VSMain(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1.0f);
    return output;
}

struct PixelInput
{
    float4 position : SV_POSITION;
};

float4 PSMain(PixelInput input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f); // Output red color
}
