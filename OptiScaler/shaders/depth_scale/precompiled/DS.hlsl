cbuffer Params : register(b0)
{
    float DepthScale;
};

// Input texture
Texture2D<float> SourceTexture : register(t0);

// Output texture
RWTexture2D<float> DestinationTexture : register(u0);

// Compute shader thread group size
[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // The dispatchThreadID.xy are the pixel coordinates
    uint2 pixelCoord = dispatchThreadID.xy;

    // Load the pixel value from the source texture
    float srcColor = SourceTexture.Load(int3(pixelCoord, 0));
    float normalizedColor = srcColor / DepthScale;

    // Write the pixel value to the destination texture
    DestinationTexture[pixelCoord] = saturate(normalizedColor);
}
