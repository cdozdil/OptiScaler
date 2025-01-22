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

#define TILE_DIM_X 16
#define TILE_DIM_Y 16

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
        ComputeWeights(0.5 / 16.0, -0.5),
		ComputeWeights(1.5 / 16.0, -0.5),
		ComputeWeights(2.5 / 16.0, -0.5),
		ComputeWeights(3.5 / 16.0, -0.5),
		ComputeWeights(4.5 / 16.0, -0.5),
		ComputeWeights(5.5 / 16.0, -0.5),
		ComputeWeights(6.5 / 16.0, -0.5),
		ComputeWeights(7.5 / 16.0, -0.5),
		ComputeWeights(8.5 / 16.0, -0.5),
		ComputeWeights(9.5 / 16.0, -0.5),
		ComputeWeights(10.5 / 16.0, -0.5),
		ComputeWeights(11.5 / 16.0, -0.5),
		ComputeWeights(12.5 / 16.0, -0.5),
		ComputeWeights(13.5 / 16.0, -0.5),
		ComputeWeights(14.5 / 16.0, -0.5),
		ComputeWeights(15.5 / 16.0, -0.5)
    };

    return FilterWeights[(uint) (offset * 16.0)];
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
    uint i0 = idx, i1 = idx + Stride, i2 = idx + 2 * Stride, i3 = idx + 3 * Stride;
    return float3x4(
		g_R[i0], g_R[i1], g_R[i2], g_R[i3],
		g_G[i0], g_G[i1], g_G[i2], g_G[i3],
		g_B[i0], g_B[i1], g_B[i2], g_B[i3]);
}

[numthreads(TILE_DIM_X, TILE_DIM_Y, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex)
{
    float scaleX = (float) _SrcWidth / (float) _DstWidth;
    float scaleY = (float) _SrcHeight / (float) _DstHeight;
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