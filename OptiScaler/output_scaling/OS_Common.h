#pragma once
#include "../pch.h"
#include <d3dcompiler.h>

struct alignas(256) Constants
{
    int32_t srcWidth;
    int32_t srcHeight;
    int32_t destWidth;
    int32_t destHeight;
};

inline static std::string downsampleCode = R"(
cbuffer Params : register(b0) {
    int _SrcWidth;    // Source texture width
    int _SrcHeight;   // Source texture height
    int _DstWidth;    // Destination texture width
    int _DstHeight;   // Destination texture height
}

Texture2D<float4> InputTexture : register(t0);  // Source texture
RWTexture2D<float4> OutputTexture : register(u0);  // Downsampled target texture

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

[numthreads(32, 32, 1)]
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
)";

inline static std::string downsampleCodeO = R"(
cbuffer Params : register(b0) {
    int _SrcWidth;    // Source texture width
    int _SrcHeight;   // Source texture height
    int _DstWidth;    // Destination texture width
    int _DstHeight;   // Destination texture height
}

Texture2D<float4> InputTexture : register(t0);  // Source texture
RWTexture2D<float4> OutputTexture : register(u0);  // Downsampled target texture

// Luminance calculation for HDR images
float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722)); // Linear HDR luminance weights
}

// MAGC Kernel
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

[numthreads(32, 32, 1)]
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

    // Cache for texels and their luminance
    float4 texelCache[16]; // 4x4 neighborhood (4 rows × 4 columns)
    float luminanceCache[16]; // Corresponding luminance values

    // Accumulators for luminance computation
    float avgLuminance = 0.0;
    float totalLuminanceWeight = 0.0;

    // Index for caching
    int index = 0;

    // First pass: Sample texels and cache luminance
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            // Compute sample coordinates
            int2 sampleCoords = sourceBase + int2(dx, dy);
            sampleCoords = clamp(sampleCoords, int2(0, 0), int2(_SrcWidth - 1, _SrcHeight - 1));

            // Fetch sample color
            float4 sampleColor = InputTexture.Load(int3(sampleCoords, 0));

            // Compute and cache luminance
            float sampleLuminance = luminance(sampleColor.rgb);
            texelCache[index] = sampleColor;
            luminanceCache[index] = sampleLuminance;

            // Compute luminance weights (MAGC kernel-based)
            float weightX = magcKernel(float(dx) - fraction.x);
            float weightY = magcKernel(float(dy) - fraction.y);
            float luminanceWeight = weightX * weightY;

            // Accumulate weighted luminance
            avgLuminance += sampleLuminance * luminanceWeight;
            totalLuminanceWeight += luminanceWeight;

            index++;
        }
    }

    // Normalize the average luminance
    avgLuminance /= totalLuminanceWeight;

    // Accumulators for color computation
    float4 color = 0.0;
    float totalWeight = 0.0;

    // Second pass: Weighted color accumulation with luminance-aware clamping
    index = 0;
    for (int dy = -1; dy <= 2; dy++)
    {
        for (int dx = -1; dx <= 2; dx++)
        {
            // Fetch cached texel and luminance
            float4 sampleColor = texelCache[index];
            float sampleLuminance = luminanceCache[index];

            // Clamp luminance to avoid outliers
            float maxDeviation = 0.5; // Allowable deviation from average
            if (abs(sampleLuminance - avgLuminance) > maxDeviation)
            {
				float luminanceScale = avgLuminance / max(sampleLuminance, 1e-5); // Avoid division by zero
				sampleColor.rgb *= luminanceScale; // Scale brightness while preserving hue/saturation
            }

            // Compute weights
            float weightX = magcKernel(float(dx) - fraction.x);
            float weightY = magcKernel(float(dy) - fraction.y);
            float weight = weightX * weightY;

            // Accumulate weighted color
            color += sampleColor * weight;
            totalWeight += weight;

            index++;
        }
    }

    // Normalize accumulated color by total weight
    color /= totalWeight;

    // Write the final downsampled color
    OutputTexture[dispatchThreadID.xy] = color;
}
)";

inline static std::string upsampleCode = R"(
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):  James Stanard
//

Texture2D<float3> Source : register(t0);
RWTexture2D<float3> Dest : register(u0);

cbuffer Params : register(b0)
{
	int _SrcWidth;
	int _SrcHeight;
	int _DstWidth;
	int _DstHeight;
};

#define TILE_DIM_X 32
#define TILE_DIM_Y 32

#define GROUP_COUNT (TILE_DIM_X * TILE_DIM_Y)

#define SAMPLES_X (TILE_DIM_X + 3)
#define SAMPLES_Y (TILE_DIM_Y + 3)

#define TOTAL_SAMPLES (SAMPLES_X * SAMPLES_Y)

// De-interleaved to avoid LDS bank conflicts
groupshared float g_R[TOTAL_SAMPLES];
groupshared float g_G[TOTAL_SAMPLES];
groupshared float g_B[TOTAL_SAMPLES];

float W1(float x, float A)
{
	return x * x * ((A + 2) * x - (A + 3)) + 1.0;
}

float W2(float x, float A)
{
	return A * (x * (x * (x - 5) + 8) - 4);
}

float4 ComputeWeights(float d1, float A)
{
	return float4(W2(1.0 + d1, A), W1(d1, A), W1(1.0 - d1, A), W2(2.0 - d1, A));
}

float4 GetBicubicFilterWeights(float offset, float A)
{
	//return ComputeWeights(offset, A);

	// Precompute weights for 16 discrete offsets
	static const float4 FilterWeights[16] =
	{
		ComputeWeights( 0.5 / 16.0, -0.5),
		ComputeWeights( 1.5 / 16.0, -0.5),
		ComputeWeights( 2.5 / 16.0, -0.5),
		ComputeWeights( 3.5 / 16.0, -0.5),
		ComputeWeights( 4.5 / 16.0, -0.5),
		ComputeWeights( 5.5 / 16.0, -0.5),
		ComputeWeights( 6.5 / 16.0, -0.5),
		ComputeWeights( 7.5 / 16.0, -0.5),
		ComputeWeights( 8.5 / 16.0, -0.5),
		ComputeWeights( 9.5 / 16.0, -0.5),
		ComputeWeights(10.5 / 16.0, -0.5),
		ComputeWeights(11.5 / 16.0, -0.5),
		ComputeWeights(12.5 / 16.0, -0.5),
		ComputeWeights(13.5 / 16.0, -0.5),
		ComputeWeights(14.5 / 16.0, -0.5),
		ComputeWeights(15.5 / 16.0, -0.5)
	};

	return FilterWeights[(uint)(offset * 16.0)];
}

// Store pixel to LDS (local data store)
void StoreLDS(uint LdsIdx, float3 rgb)
{
	g_R[LdsIdx] = rgb.r;
	g_G[LdsIdx] = rgb.g;
	g_B[LdsIdx] = rgb.b;
}

// Load four pixel samples from LDS.  Stride determines horizontal or vertical groups.
float3x4 LoadSamples(uint idx, uint Stride)
{
	uint i0 = idx, i1 = idx+Stride, i2 = idx+2*Stride, i3=idx+3*Stride;
	return float3x4(
		g_R[i0], g_R[i1], g_R[i2], g_R[i3],
		g_G[i0], g_G[i1], g_G[i2], g_G[i3],
		g_B[i0], g_B[i1], g_B[i2], g_B[i3]);
}

[numthreads(TILE_DIM_X, TILE_DIM_Y, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
{
	float scaleX = (float)_SrcWidth / (float)_DstWidth;
	float scaleY = (float)_SrcHeight / (float)_DstHeight;
	const float2 kRcpScale = float2(scaleX, scaleY);
	const float kA = 0.3f;
	
	// Number of samples needed from the source buffer to generate the output tile dimensions.
	const uint2 SampleSpace = ceil(float2(TILE_DIM_X, TILE_DIM_Y) * kRcpScale + 3.0);
	
	// Pre-Load source pixels
	int2 UpperLeft = floor((Gid.xy * uint2(TILE_DIM_X, TILE_DIM_Y) + 0.5) * kRcpScale - 1.5);

	for (uint i = GI; i < TOTAL_SAMPLES; i += GROUP_COUNT)
		StoreLDS(i, Source[UpperLeft + int2(i % SAMPLES_X, i / SAMPLES_Y)]);

	GroupMemoryBarrierWithGroupSync();

	// The coordinate of the top-left sample from the 4x4 kernel (offset by -0.5
	// so that whole numbers land on a pixel center.)  This is in source texture space.
	float2 TopLeftSample = (DTid.xy + 0.5) * kRcpScale - 1.5;

	// Position of samples relative to pixels used to evaluate the Sinc function.
	float2 Phase = frac(TopLeftSample);

	// LDS tile coordinate for the top-left sample (for this thread)
	uint2 TileST = int2(floor(TopLeftSample)) - UpperLeft;

	// Convolution weights, one per sample (in each dimension)
	float4 xWeights = GetBicubicFilterWeights(Phase.x, kA);
	float4 yWeights = GetBicubicFilterWeights(Phase.y, kA);

	// Horizontally convolve the first N rows
	uint ReadIdx = TileST.x + GTid.y * SAMPLES_X;

	uint WriteIdx = GTid.x + GTid.y * SAMPLES_X;
	StoreLDS(WriteIdx, mul(LoadSamples(ReadIdx, 1), xWeights));

	// If the source tile plus border is larger than the destination tile, we
	// have to convolve a few more rows.
	if (GI + GROUP_COUNT < SampleSpace.y * TILE_DIM_X)
	{
		ReadIdx += TILE_DIM_Y * SAMPLES_X;
		WriteIdx += TILE_DIM_Y * SAMPLES_X;
		StoreLDS(WriteIdx, mul(LoadSamples(ReadIdx, 1), xWeights));
	}

	GroupMemoryBarrierWithGroupSync();

	// Convolve vertically N columns
	ReadIdx = GTid.x + TileST.y * SAMPLES_X;
	float3 Result = mul(LoadSamples(ReadIdx, SAMPLES_X), yWeights);

	// Transform to display settings
	// Result = RemoveDisplayProfile(Result, LDR_COLOR_FORMAT);
	// Dest[DTid.xy] = ApplyDisplayProfile(Result, DISPLAY_PLANE_FORMAT);
	Dest[DTid.xy] = Result;
}
)";

inline static ID3DBlob* OS_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        LOG_ERROR("error while compiling shader");

        if (errorBlob)
        {
            LOG_ERROR("error while compiling shader : {0}", (char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return nullptr;
    }

    if (errorBlob)
        errorBlob->Release();

    return shaderBlob;
}