cbuffer Params : register(b0)
{
    float Bias;
};

Texture2D<float3> Source : register(t0);
RWTexture2D<float3> Dest : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
  float3 src = Source.Load(int3(DTid.x, DTid.y, 0)).rgb;
  src.r *= Bias;
  Dest[DTid.xy] = src;
}