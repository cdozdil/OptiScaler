#pragma once

#include "../pch.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "../d3dx/d3dx12.h"

struct alignas(256) Constants
{
    int32_t srcWidth;
    int32_t srcHeight;
    int32_t destWidth;
    int32_t destHeight;
};

class BS_Dx12
{
private:
    std::string _name = "";
    bool _init = false;
    ID3D12RootSignature* _rootSignature = nullptr;
    ID3D12PipelineState* _pipelineState = nullptr;
    ID3D12DescriptorHeap* _srvHeap[3] = { nullptr, nullptr, nullptr };
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuSrvHandle[2]{ { NULL }, { NULL } };
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuUavHandle[2]{ { NULL }, { NULL } };
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuCbvHandle[2]{ { NULL }, { NULL } };
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuSrvHandle[2]{ { NULL }, { NULL } };
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuUavHandle[2]{ { NULL }, { NULL } };
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuCbvHandle[2]{ { NULL }, { NULL } };
    int _counter = 0;
    bool _upsample = false;

    uint32_t InNumThreadsX = 32;
    uint32_t InNumThreadsY = 32;

    inline static std::string downsampleCode = R"(
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

    inline static std::string fsrCode = R"(
// Based on: https://github.com/40163650/FSRForReShade/blob/main/FSR.h
// Based on: https://www.shadertoy.com/view/stXSWB

#define PI 3.1415923693
#define SINE(x) sin(.5 * PI * (x - .5)) + 1.
#define COSINE(x) 1. - cos(.5 * PI * x)

cbuffer Params : register(b0)
{
    int _SrcWidth;
    int _SrcHeight;
    int _DstWidth;
    int _DstHeight;
};

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

// Helper function to perform linear interpolation
float3 LinearInterpolate(float3 a, float3 b, float t)
{
    return a * (1.0 - t) + b * t;
}

// Helper function to load texture with bilinear filtering
float3 LoadBilinear(Texture2D<float4> texture, float2 uv, float2 texSize)
{
    int2 texelCoord = int2(uv * texSize - 0.5);
    float2 fracOffset = frac(uv * texSize - 0.5);

    float3 c00 = texture.Load(int3(texelCoord, 0)).rgb;
    float3 c10 = texture.Load(int3(texelCoord + int2(1, 0), 0)).rgb;
    float3 c01 = texture.Load(int3(texelCoord + int2(0, 1), 0)).rgb;
    float3 c11 = texture.Load(int3(texelCoord + int2(1, 1), 0)).rgb;

    float3 c0 = LinearInterpolate(c00, c10, fracOffset.x);
    float3 c1 = LinearInterpolate(c01, c11, fracOffset.x);

    return LinearInterpolate(c0, c1, fracOffset.y);
}

/**** EASU ****/
void FsrEasuCon(out float4 con0, out float4 con1, out float4 con2, out float4 con3,
    float2 inputViewportInPixels, // This the rendered image resolution being upscaled
    float2 inputSizeInPixels, // This is the resolution of the resource containing the input image (useful for dynamic resolution)
    float2 outputSizeInPixels) // This is the display resolution which the input image gets upscaled to
{
    // Output integer position to a pixel position in viewport.
    con0 = float4(inputViewportInPixels.x / outputSizeInPixels.x, inputViewportInPixels.y / outputSizeInPixels.y,
        mad(.5, inputViewportInPixels.x / outputSizeInPixels.x, -.5), mad(.5, inputViewportInPixels.y / outputSizeInPixels.y, -.5));
    
    // Viewport pixel position to normalized image space.
    // This is used to get upper-left of 'F' tap.
    con1 = float4(1, 1, 1, -1) / inputSizeInPixels.xyxy;
    
    // Centers of gather4, first offset from upper-left of 'F'.
    //      +---+---+
    //      |   |   |
    //      +--(0)--+
    //      | b | c |
    //  +---F---+---+---+
    //  | e | f | g | h |
    //  +--(1)--+--(2)--+
    //  | i | j | k | l |
    //  +---+---+---+---+
    //      | n | o |
    //      +--(3)--+
    //      |   |   |
    //      +---+---+
    // These are from (0) instead of 'F'.
    con2 = float4(-1, 2, 1, 2) / inputSizeInPixels.xyxy;
    con3 = float4(0, 4, 0, 0) / inputSizeInPixels.xyxy;
}

// Filtering for a given tap for the scalar.
void FsrEasuTapF(
    inout float3 aC, // Accumulated color, with negative lobe.
    inout float aW, // Accumulated weight.
    float2 off, // Pixel offset from resolve position to tap.
    float2 dir, // Gradient direction.
    float2 len, // Length.
    float lob, // Negative lobe strength.
    float clp, // Clipping point.
    float3 c)
{
    // Tap color.
    // Rotate offset by direction.
    float2 v = float2(dot(off, dir), dot(off, float2(-dir.y, dir.x)));
    
    // Anisotropy.
    v *= len;
    
    // Compute distance^2.
    float d2 = min(dot(v, v), clp);
    
    // Limit to the window as at corner, 2 taps can easily be outside.
    // Approximation of lancos2 without sin() or rcp(), or sqrt() to get x.
    //  (25/16 * (2/5 * x^2 - 1)^2 - (25/16 - 1)) * (1/4 * x^2 - 1)^2
    //  |_______________________________________|   |_______________|
    //                   base                             window
    // The general form of the 'base' is,
    //  (a*(b*x^2-1)^2-(a-1))
    // Where 'a=1/(2*b-b^2)' and 'b' moves around the negative lobe.
    float wB = mad(.4, d2, 1.);
    float wA = mad(lob, d2, -1.);
    wB *= wB;
    wA *= wA;
    wB = mad(1.5625, wB, .5625);
    float w = wB * wA;
    
    // Do weighted average.
    aC += c * w;
    aW += w;
}

// Accumulate direction and length.
void FsrEasuSetF(inout float2 dir, inout float len, float w, float lA, float lB, float lC, float lD, float lE)
{
    // Direction is the '+' diff.
    //    a
    //  b c d
    //    e
    // Then takes magnitude from abs average of both sides of 'c'.
    // Length converts gradient reversal to 0, smoothly to non-reversal at 1, shaped, then adding horz and vert terms.
    float lenX = max(abs(lD - lC), abs(lC - lB));
    float dirX = lD - lB;
    dir.x += dirX * w;
    lenX = clamp(abs(dirX) / lenX, 0., 1.);
    lenX *= lenX;
    len += lenX * w;
    
    // Repeat for the y axis.
    float lenY = max(abs(lE - lC), abs(lC - lA));
    float dirY = lE - lA;
    dir.y += dirY * w;
    lenY = clamp(abs(dirY) / lenY, 0., 1.);
    lenY *= lenY;
    len += lenY * w;
}

void FsrEasuF(out float3 pix,float2 ip, // Integer pixel position in output.
    // Constants generated by FsrEasuCon(), xy = output to input scale, zw = first pixel offset correction
    float4 con0, float4 con1, float4 con2, float4 con3)
{
    // Get position of 'f'.
    float2 pp = mad(ip, con0.xy, con0.zw); // Corresponding input pixel/subpixel
    float2 fp = floor(pp); // fp = source nearest pixel
    pp -= fp; // pp = source subpixel

    // 12-tap kernel.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    // Gather 4 ordering.
    //  a b
    //  r g
    float2 p0 = mad(fp, con1.xy, con1.zw);

    // These are from p0 to avoid pulling two constants on pre-Navi hardware.
    float2 p1 = p0 + con2.xy;
    float2 p2 = p0 + con2.zw;
    float2 p3 = p0 + con3.xy;

    // TextureGather is not available on WebGL2
    float4 off = float4(-.5, .5, -.5, .5) * con1.xxyy;
    
    // textureGather to texture offsets
    // x=west y=east z=north w=south
    float2 texSize = float2(_SrcWidth, _SrcHeight);
    
    float3 bC = LoadBilinear(InputTexture, p0 + off.xw, texSize);
    float3 cC = LoadBilinear(InputTexture, p0 + off.yw, texSize);
    float3 iC = LoadBilinear(InputTexture, p1 + off.xw, texSize);
    float3 jC = LoadBilinear(InputTexture, p1 + off.yw, texSize);
    float3 fC = LoadBilinear(InputTexture, p1 + off.yz, texSize);
    float3 eC = LoadBilinear(InputTexture, p1 + off.xz, texSize);
    float3 kC = LoadBilinear(InputTexture, p2 + off.xw, texSize);
    float3 lC = LoadBilinear(InputTexture, p2 + off.yw, texSize);
    float3 hC = LoadBilinear(InputTexture, p2 + off.yz, texSize);
    float3 gC = LoadBilinear(InputTexture, p2 + off.xz, texSize);
    float3 oC = LoadBilinear(InputTexture, p3 + off.yz, texSize);
    float3 nC = LoadBilinear(InputTexture, p3 + off.xz, texSize);

    float bL = mad(0.5, bC.r + bC.b, bC.g);
    float cL = mad(0.5, cC.r + cC.b, cC.g);
    float iL = mad(0.5, iC.r + iC.b, iC.g);
    float jL = mad(0.5, jC.r + jC.b, jC.g);
    float fL = mad(0.5, fC.r + fC.b, fC.g);
    float eL = mad(0.5, eC.r + eC.b, eC.g);
    float kL = mad(0.5, kC.r + kC.b, kC.g);
    float lL = mad(0.5, lC.r + lC.b, lC.g);
    float hL = mad(0.5, hC.r + hC.b, hC.g);
    float gL = mad(0.5, gC.r + gC.b, gC.g);
    float oL = mad(0.5, oC.r + oC.b, oC.g);
    float nL = mad(0.5, nC.r + nC.b, nC.g);

    // Simplest multi-channel approximate luma possible (luma times 2, in 2 FMA/MAD).
    // Accumulate for bilinear interpolation.
    float2 dir = float2(0.0, 0.0);
    float len = 0.;

    FsrEasuSetF(dir, len, (1. - pp.x) * (1. - pp.y), bL, eL, fL, gL, jL);
    FsrEasuSetF(dir, len, pp.x * (1. - pp.y), cL, fL, gL, hL, kL);
    FsrEasuSetF(dir, len, (1. - pp.x) * pp.y, fL, iL, jL, kL, nL);
    FsrEasuSetF(dir, len, pp.x * pp.y, gL, jL, kL, lL, oL);

    // Normalize with approximation, and cleanup close to zero.
    float2 dir2 = dir * dir;
    float dirR = dir2.x + dir2.y;
    bool zro = dirR < (1.0 / 32768.0);
    dirR = rsqrt(dirR);
    dirR = zro ? 1.0 : dirR;
    dir.x = zro ? 1.0 : dir.x;
    dir *= float2(dirR, dirR);
    
    // Transform from {0 to 2} to {0 to 1} range, and shape with square.
    len = len * 0.5;
    len *= len;
    
    // Stretch kernel {1.0 vert|horz, to sqrt(2.0) on diagonal}.
    float stretch = dot(dir, dir) / (max(abs(dir.x), abs(dir.y)));
    
    // Anisotropic length after rotation,
    //  x := 1.0 lerp to 'stretch' on edges
    //  y := 1.0 lerp to 2x on edges
    float2 len2 = float2(mad(stretch - 1.0, len, 1), mad(-.5, len, 1.));
    
    // Based on the amount of 'edge',
    // the window shifts from +/-{sqrt(2.0) to slightly beyond 2.0}.
    float lob = mad(-.29, len, .5);
    
    // Set distance^2 clipping point to the end of the adjustable window.
    float clp = 1. / lob;

    // Accumulation mixed with min/max of 4 nearest.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    float3 min4 = min(min(fC, gC), min(jC, kC));
    float3 max4 = max(max(fC, gC), max(jC, kC));
    
    // Accumulation.
    float3 aC = float3(0, 0, 0);
    float aW = 0.;
    FsrEasuTapF(aC, aW, float2(0, -1) - pp, dir, len2, lob, clp, bC);
    FsrEasuTapF(aC, aW, float2(1, -1) - pp, dir, len2, lob, clp, cC);
    FsrEasuTapF(aC, aW, float2(-1, 1) - pp, dir, len2, lob, clp, iC);
    FsrEasuTapF(aC, aW, float2(0, 1) - pp, dir, len2, lob, clp, jC);
    FsrEasuTapF(aC, aW, float2(0, 0) - pp, dir, len2, lob, clp, fC);
    FsrEasuTapF(aC, aW, float2(-1, 0) - pp, dir, len2, lob, clp, eC);
    FsrEasuTapF(aC, aW, float2(1, 1) - pp, dir, len2, lob, clp, kC);
    FsrEasuTapF(aC, aW, float2(2, 1) - pp, dir, len2, lob, clp, lC);
    FsrEasuTapF(aC, aW, float2(2, 0) - pp, dir, len2, lob, clp, hC);
    FsrEasuTapF(aC, aW, float2(1, 0) - pp, dir, len2, lob, clp, gC);
    FsrEasuTapF(aC, aW, float2(1, 2) - pp, dir, len2, lob, clp, oC);
    FsrEasuTapF(aC, aW, float2(0, 2) - pp, dir, len2, lob, clp, nC);
    
    // Normalize and dering.
    pix = min(max4, max(min4, aC / aW));
}

[numthreads(32, 32, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    float3 c;
    float4 con0, con1, con2, con3;

    float2 fragCoord = DTid.xy;

    // "rendersize" refers to size of source image before upscaling.
    float2 renderSize = float2(_SrcWidth, _SrcHeight);
    float2 displaySize = float2(_DstWidth, _DstHeight);
    
    FsrEasuCon(con0, con1, con2, con3, renderSize, renderSize, displaySize);
    FsrEasuF(c, fragCoord, con0, con1, con2, con3);
    
    OutputTexture[DTid.xy] = float4(c, 1);
}
)";
    ID3D12Device* _device = nullptr;
    ID3D12Resource* _buffer = nullptr;
    ID3D12Resource* _constantBuffer = nullptr;
    D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;

public:
    bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, uint32_t InWidth, uint32_t InHeight, D3D12_RESOURCE_STATES InState);
    void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
    bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* OutResource);

    ID3D12Resource* Buffer() { return _buffer; }
    bool IsUpsampling() { return _upsample; }
    bool IsInit() const { return _init; }

    BS_Dx12(std::string InName, ID3D12Device* InDevice, bool InUpsample);

    ~BS_Dx12();
};