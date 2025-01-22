cbuffer Params : register(b0)
{
    int _SrcWidth; // Source texture width
    int _SrcHeight; // Source texture height
    int _DstWidth; // Destination texture width
    int _DstHeight; // Destination texture height
};

// Texture resources.
Texture2D<float4> InputTexture : register(t0); // Input texture (source image)
RWTexture2D<float4> OutputTexture : register(u0); // Output texture (downsampled image)

float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// Lanczos kernel function.
float lanczosKernel(float x, float radius, float pi)
{
    if (x == 0.0)
        return 1.0;
    if (x > radius)
        return 0.0;

    x *= pi;
    return (sin(x) / x) * (sin(x / radius) / (x / radius));
}

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Coordinates of the pixel in the target (downsampled) texture.
    uint2 targetCoords = dispatchThreadID.xy;

    // Ensure we are within the target texture bounds.
    if (targetCoords.x >= _DstWidth || targetCoords.y >= _DstHeight)
        return;

    // Calculate scaling factor and source position.
    float2 scale = float2(_SrcWidth, _SrcHeight) / float2(_DstWidth, _DstHeight);
    float2 sourcePos = float2(targetCoords) * scale;

    // Lanczos kernel properties.
    const float lanczosRadius = 3.0; // Typical radius (adjustable for quality/performance)
    const float pi = 3.14159265359;

    // Accumulators for color and weight.
    float4 color = 0.0;
    float totalWeight = 0.0;
    float avgLuminance = 0.0;

    // First pass: Compute average luminance in the 4x4 neighborhood
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            int2 sampleCoords = sourcePos + int2(dx, dy);
            sampleCoords = clamp(sampleCoords, int2(0, 0), int2(_SrcWidth - 1, _SrcHeight - 1));

            float4 sampleColor = InputTexture.Load(int3(sampleCoords, 0));
            avgLuminance += luminance(sampleColor.rgb);
        }
    }
    avgLuminance /= 16.0; // Normalize luminance by the 4x4 sample count

    // Loop through the kernel window.
    for (int y = -int(lanczosRadius); y <= int(lanczosRadius); y++)
    {
        for (int x = -int(lanczosRadius); x <= int(lanczosRadius); x++)
        {
            // Offset in source texture space.
            float2 offset = float2(x, y);
            float2 samplePos = sourcePos + offset;

            // Ensure we sample within bounds.
            samplePos = clamp(samplePos, float2(0, 0), float2(_SrcWidth - 1, _SrcHeight - 1));

            // Fetch the texel.
            float4 sampleColor = InputTexture.Load(int3(samplePos, 0));

			// Compute the current luminance of the sample
            float currentLuminance = luminance(sampleColor.rgb);
			
			// Scale the color to match the average luminance if it deviates too much
            float luminanceDeviation = abs(currentLuminance - avgLuminance);
            if (luminanceDeviation > 0.5) // Threshold for clamping
            {
                float luminanceScale = avgLuminance / max(currentLuminance, 1e-5); // Avoid division by zero
                sampleColor.rgb *= luminanceScale; // Scale brightness while preserving hue/saturation
            }

            // Calculate Lanczos weight.
            float2 dist = abs(samplePos - sourcePos);
            float2 lanczosWeight = lanczosKernel(dist.x, lanczosRadius, pi) *
                                   lanczosKernel(dist.y, lanczosRadius, pi);

            // Accumulate weighted color.
            color += sampleColor * lanczosWeight.x * lanczosWeight.y;
            totalWeight += lanczosWeight.x * lanczosWeight.y;
        }
    }

    // Normalize the color by the total weight to avoid darkening/brightening.
    color /= totalWeight;

    // Write the result to the output texture.
    OutputTexture[dispatchThreadID.xy] = color;
}