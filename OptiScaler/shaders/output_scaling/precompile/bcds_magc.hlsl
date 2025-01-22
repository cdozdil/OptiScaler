cbuffer Params : register(b0)
{
    int _SrcWidth; // Source texture width
    int _SrcHeight; // Source texture height
    int _DstWidth; // Destination texture width
    int _DstHeight; // Destination texture height
}

Texture2D<float4> InputTexture : register(t0); // Source texture
RWTexture2D<float4> OutputTexture : register(u0); // Downsampled target texture

// Luminance computation using perceptual weights
float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// MAGC Kernel definition
float magcKernel(float x)
{
    x = abs(x);

    if (x <= 1.0)
    {
        return 1.0 - 2.0 * x * x + x * x * x;
    }
    else if (x <= 2.0)
    {
        return 4.0 - 8.0 * x + 5.0 * x * x - x * x * x;
    }

    return 0.0;
}

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Target coordinates in the downsampled texture
    uint2 targetCoords = dispatchThreadID.xy;

    // Ensure thread is within bounds of the destination texture
    if (targetCoords.x >= _DstWidth || targetCoords.y >= _DstHeight)
        return;

    // Compute scaling factor and source position
    float2 scale = float2(_SrcWidth, _SrcHeight) / float2(_DstWidth, _DstHeight);
    float2 sourcePos = float2(targetCoords) * scale;

    // Base source position and fractional offset
    int2 sourceBase = int2(sourcePos);
    float2 fraction = frac(sourcePos);

    // Accumulators for color, weight, and luminance
    float4 color = 0.0;
    float totalWeight = 0.0;
    float avgLuminance = 0.0;

    // First pass: Compute average luminance in the 4x4 neighborhood
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            int2 sampleCoords = sourceBase + int2(dx, dy);
            sampleCoords = clamp(sampleCoords, int2(0, 0), int2(_SrcWidth - 1, _SrcHeight - 1));

            float4 sampleColor = InputTexture.Load(int3(sampleCoords, 0));
            avgLuminance += luminance(sampleColor.rgb);
        }
    }
    avgLuminance /= 16.0; // Normalize luminance by the 4x4 sample count

    // Second pass: Weighted color accumulation with luminance-aware clamping
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            int2 sampleCoords = sourceBase + int2(dx, dy);
            sampleCoords = clamp(sampleCoords, int2(0, 0), int2(_SrcWidth - 1, _SrcHeight - 1));

            float4 sampleColor = InputTexture.Load(int3(sampleCoords, 0));

			// Compute the current luminance of the sample
            float currentLuminance = luminance(sampleColor.rgb);
			
			// Scale the color to match the average luminance if it deviates too much
            float luminanceDeviation = abs(currentLuminance - avgLuminance);
            if (luminanceDeviation > 0.5) // Threshold for clamping
            {
                float luminanceScale = avgLuminance / max(currentLuminance, 1e-5); // Avoid division by zero
                sampleColor.rgb *= luminanceScale; // Scale brightness while preserving hue/saturation
            }

            // MAGC kernel weights
            float weightX = magcKernel(float(dx) - fraction.x);
            float weightY = magcKernel(float(dy) - fraction.y);
            float weight = weightX * weightY;

            // Accumulate weighted color and weight
            color += sampleColor * weight;
            totalWeight += weight;
        }
    }

    // Normalize accumulated color by total weight
    color /= totalWeight;

    // Output the final downsampled color
    OutputTexture[dispatchThreadID.xy] = color;
}