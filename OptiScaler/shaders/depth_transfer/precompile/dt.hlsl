Texture2D<float> SourceTexture : register(t0);
RWTexture2D<float> DestinationTexture : register(u0);

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));
    DestinationTexture[dispatchThreadID.xy] = srcColor;
}