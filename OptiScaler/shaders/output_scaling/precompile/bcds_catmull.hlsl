cbuffer Params : register(b0)
{
    int _SrcWidth; // Source texture width
    int _SrcHeight; // Source texture height
    int _DstWidth; // Destination texture width
    int _DstHeight; // Destination texture height
}

// Texture resources
Texture2D<float4> InputTexture : register(t0); // Input texture (source image)
RWTexture2D<float4> OutputTexture : register(u0); // Output texture (downsampled image)

float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// Catmull-Rom Kernel
float catmullRom(float x)
{
    x = abs(x);

    if (x < 1.0)
    {
        return (1.5 * x - 2.5) * x * x + 1.0;
    }
    else if (x < 2.0)
    {
        return ((-0.5 * x + 2.5) * x - 4.0) * x + 2.0;
    }
    else
    {
        return 0.0;
    }
}

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Coordinates of the pixel in the target (downsampled) texture
    uint2 targetCoords = dispatchThreadID.xy;

    // Ensure thread is within the target texture bounds
    if (targetCoords.x >= _DstWidth || targetCoords.y >= _DstHeight)
        return;

    // Scaling factor
    float2 scale = float2(_SrcWidth, _SrcHeight) / float2(_DstWidth, _DstHeight);

    // Position in the source texture
    float2 sourcePos = float2(targetCoords) * scale;

    // Integer and fractional parts of the source position
    int2 sourceBase = int2(sourcePos); // Top-left integer pixel
    float2 fraction = frac(sourcePos); // Fractional offset for interpolation

    // Accumulators for color and weight
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

    // Loop over the 4x4 neighborhood of pixels for cubic interpolation
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            // Neighbor pixel position in the source texture
            int2 sampleCoords = sourceBase + int2(dx, dy);

            // Clamp to source texture bounds
            sampleCoords = clamp(sampleCoords, int2(0, 0), int2(_SrcWidth - 1, _SrcHeight - 1));

            // Fetch the sample color
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

            // Compute the Catmull-Rom weight for x and y directions
            float weightX = catmullRom(float(dx) - fraction.x);
            float weightY = catmullRom(float(dy) - fraction.y);

            // Accumulate the weighted color
            float weight = weightX * weightY;
            color += sampleColor * weight;
            totalWeight += weight;
        }
    }

    // Normalize the color by the total weight
    color /= totalWeight;

    // Write the result to the output texture
    OutputTexture[dispatchThreadID.xy] = color;
}