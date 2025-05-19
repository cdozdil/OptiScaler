cbuffer Params : register(b0)
{
    uint width;
    uint height;
};

// Input texture
Texture2D<float3> SourceTexture : register(t0);

// Output texture
RWTexture2D<float3> DestinationTexture : register(u0);

// Compute shader thread group size
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixelCoord = uint2(dispatchThreadID.x, height - dispatchThreadID.y);
    float3 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));
    DestinationTexture[pixelCoord] = srcColor;
}