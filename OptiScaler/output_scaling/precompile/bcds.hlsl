cbuffer Params : register(b0)
{
    int _SrcWidth;
    int _SrcHeight;
    int _DstWidth;
    int _DstHeight;
};

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

float bicubic_weight(float x)
{
    float a = -0.5f;
    float absX = abs(x);

    if (absX <= 1.0f)
        return (a + 2.0f) * absX * absX * absX - (a + 3.0f) * absX * absX + 1.0f;
    else if (absX < 2.0f)
        return a * absX * absX * absX - 5.0f * a * absX * absX + 8.0f * a * absX - 4.0f * a;
    else
        return 0.0f;
}

[numthreads(32, 32, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= _DstWidth || DTid.y >= _DstHeight)
        return;

    float2 uv = float2(DTid.x / (_DstWidth - 1.0f), DTid.y / (_DstHeight - 1.0f));
    float2 pixel = uv * float2(_SrcWidth, _SrcHeight);
    float2 texel = floor(pixel);
    float2 t = pixel - texel;
    t = t * t * (3.0f - 2.0f * t);

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int y = -1; y <= 2; y++)
    {
        for (int x = -1; x <= 2; x++)
        {
            float4 color = InputTexture.Load(int3(texel.x + x, texel.y + y, 0));
            float weight = bicubic_weight(x - t.x) * bicubic_weight(y - t.y);
            result += color * weight;
        }
    }

    OutputTexture[DTid.xy] = result;
}