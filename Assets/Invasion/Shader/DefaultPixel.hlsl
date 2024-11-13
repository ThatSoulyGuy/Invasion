
Texture2D diffuse : register(t0);
SamplerState samplerState : register(s0);

struct PixelData
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

float4 Main(PixelData input) : SV_TARGET
{
    return diffuse.Sample(samplerState, input.uvs) * input.color;
}