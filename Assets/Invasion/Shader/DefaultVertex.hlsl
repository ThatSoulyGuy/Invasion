#include "Common.hlsli"

cbuffer MatrixBuffer : register(b0)
{
    row_major matrix projectionMatrix;
    row_major matrix viewMatrix;
    row_major matrix modelMatrix;
};

struct VertexData
{
    float3 position : POSITION;
    float3 color : COLOR;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

struct PixelData
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uvs : UVS;
};

PixelData Main(VertexData input)
{
    PixelData output;
    
    float4 worldPosition = float4(input.position, 1.0f);
    
    worldPosition = mul(worldPosition, modelMatrix);
    worldPosition = mul(worldPosition, viewMatrix);
    worldPosition = mul(worldPosition, projectionMatrix);
    
    output.position = worldPosition;
    output.color = float4(input.color, 1.0f);
    float3 transformedNormal = mul(input.normal, (float3x3) transpose(Common_Inverse(modelMatrix)));
    output.normal = normalize(transformedNormal);
    output.uvs = input.uvs;
    
    return output;
}