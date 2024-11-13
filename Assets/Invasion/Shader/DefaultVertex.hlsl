
cbuffer MatrixBuffer : register(b0)
{
    //matrix projectionMatrix;
    //matrix viewMatrix;
    matrix modelMatrix;
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
    
    output.position = worldPosition;
    output.color = float4(input.color, 1.0f);
    output.normal = input.normal;
    output.uvs = input.uvs;
    
    return output;
}