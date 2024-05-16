#pragma once

#include "../pch.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "../d3dx/d3dx12.h"

struct RcasConstants
{
	float Sharpness;

	// Motion Vector Stuff
	bool DynamicSharpenEnabled;
	bool DisplaySizeMV;
	bool Debug;

	float MotionSharpness;
	float MotionTextureScale;
	float MvScaleX;
	float MvScaleY;
	float Threshold;
	int DisplayWidth;
	int DisplayHeight;
};

class RCAS_Dx12
{
private:
	struct alignas(256) InternalConstants
	{
		float Sharpness;

		// Motion Vector Stuff
		int DynamicSharpenEnabled;
		int DisplaySizeMV;
		int Debug;

		float MotionSharpness;
		float MotionTextureScale;
		float MvScaleX;
		float MvScaleY;
		float Threshold;
		int DisplayWidth;
		int DisplayHeight;
	};

	std::string _name = "";
	bool _init = false;
	int _counter = 0;

	ID3D12RootSignature* _rootSignature = nullptr;
	ID3D12PipelineState* _pipelineState = nullptr;
	ID3D12DescriptorHeap* _srvHeap[4] = { nullptr, nullptr, nullptr, nullptr };
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuSrvHandle[2]{ { NULL }, { NULL } };
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuSrvHandle2[2]{ { NULL }, { NULL } };
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuUavHandle[2]{ { NULL }, { NULL } };
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuCbvHandle[2]{ { NULL }, { NULL } };
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuSrvHandle[2]{ { NULL }, { NULL } };
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuSrvHandle2[2]{ { NULL }, { NULL } };
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuUavHandle[2]{ { NULL }, { NULL } };
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuCbvHandle[2]{ { NULL }, { NULL } };

	ID3D12Device* _device = nullptr;
	ID3D12Resource* _buffer = nullptr;
	ID3D12Resource* _constantBuffer = nullptr;
	D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;

	uint32_t InNumThreadsX = 32;
	uint32_t InNumThreadsY = 32;
							 
	std::string rcasCode = R"(
cbuffer Params : register(b0)
{
	float Sharpness;

	// Motion Vector Stuff
	int DynamicSharpenEnabled;
	int DisplaySizeMV;
	int Debug;
	
	float MotionSharpness;
	float MotionTextureScale;
	float MvScaleX;
	float MvScaleY;
	float Threshold;
	int DisplayWidth;
	int DisplayHeight;
};

Texture2D<float3> Source : register(t0);
Texture2D<float2> Motion : register(t1);
RWTexture2D<float3> Dest : register(u0);

float getRCASLuma(float3 rgb)
{  
	return dot(rgb, float3(0.598, 1.174, 0.228));
}

[numthreads(32, 32, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
  float setSharpness = Sharpness;
  
  if(DynamicSharpenEnabled > 0 && MotionSharpness > Sharpness)
  {
	float2 mv;
	float mx;
	float my;
	float factor = MotionSharpness - Sharpness;
	float add = 0.0f;

	if(DisplaySizeMV > 0)
	  mv = Motion.Load(int3(DTid.x, DTid.y, 0)).rg;
	else
	  mv = Motion.Load(int3(DTid.x * MotionTextureScale, DTid.y * MotionTextureScale, 0)).rg;

	mx = abs(mv.r * MvScaleX);
	my = abs(mv.g * MvScaleY);

	if(mx > my)
		add = (mx / Threshold) * factor;
	else
		add = (my / Threshold) * factor;
	
	if(add > factor)
		add = factor;

	setSharpness += add;
  }

  float3 e = Source.Load(int3(DTid.x, DTid.y, 0)).rgb;

  float3 b = Source.Load(int3(DTid.x, DTid.y - 1, 0)).rgb;
  float3 d = Source.Load(int3(DTid.x - 1, DTid.y, 0)).rgb;
  float3 f = Source.Load(int3(DTid.x + 1, DTid.y, 0)).rgb;
  float3 h = Source.Load(int3(DTid.x, DTid.y + 1, 0)).rgb;

  // Get lumas times 2. Should use luma weights that are twice as large as normal.
  float bL = getRCASLuma(b);
  float dL = getRCASLuma(d);
  float eL = getRCASLuma(e);
  float fL = getRCASLuma(f);
  float hL = getRCASLuma(h);

  // Min and max of ring.
  float3 minRGB = min(min(b, d), min(f, h));
  float3 maxRGB = max(max(b, d), max(f, h));
  
  // Immediate constants for peak range.
  float2 peakC = float2(1.0, -4.0);

  // Limiters, these need to use high precision reciprocal operations.
  // Decided to use standard rcp for now in hopes of optimizing it
  float3 hitMin = minRGB * rcp(4.0 * maxRGB);
  float3 hitMax = (peakC.xxx - maxRGB) * rcp(4.0 * minRGB + peakC.yyy);
  float3 lobeRGB = max(-hitMin, hitMax);
  float lobe = max(-0.1875, min(max(lobeRGB.r, max(lobeRGB.g, lobeRGB.b)), 0.0)) * setSharpness;

  // Resolve, which needs medium precision rcp approximation to avoid visible tonality changes.
  float rcpL = rcp(4.0 * lobe + 1.0);
  float3 output = ((b + d + f + h) * lobe + e) * rcpL;

  if(Debug > 0 && DynamicSharpenEnabled > 0)
	output.r *= 1 + (10.0 * (setSharpness - Sharpness));

  Dest[DTid.xy] = output;
}
)";	

//	std::string rcasCode = R"(
//cbuffer Params : register(b0)
//{
//	float Sharpness;
//};
//
//Texture2D<float3> Source : register(t0);
//RWTexture2D<float3> Dest : register(u0);
//
//float getRCASLuma(float3 rgb)
//{  
//	return dot(rgb, float3(0.598, 1.174, 0.228));
//}
//
//[numthreads(32, 32, 1)]
//void CSMain(uint3 DTid : SV_DispatchThreadID)
//{
//  float3 e = Source.Load(int3(DTid.x, DTid.y, 0)).rgb;
//
//  float3 b = Source.Load(int3(DTid.x, DTid.y - 1, 0)).rgb;
//  float3 d = Source.Load(int3(DTid.x - 1, DTid.y, 0)).rgb;
//  float3 f = Source.Load(int3(DTid.x + 1, DTid.y, 0)).rgb;
//  float3 h = Source.Load(int3(DTid.x, DTid.y + 1, 0)).rgb;
//
//  // Get lumas times 2. Should use luma weights that are twice as large as normal.
//  float bL = getRCASLuma(b);
//  float dL = getRCASLuma(d);
//  float eL = getRCASLuma(e);
//  float fL = getRCASLuma(f);
//  float hL = getRCASLuma(h);
//
//  // Min and max of ring.
//  float3 minRGB = min(min(b, d), min(f, h));
//  float3 maxRGB = max(max(b, d), max(f, h));
//  
//  // Immediate constants for peak range.
//  float2 peakC = float2(1.0, -4.0);
//
//  // Limiters, these need to use high precision reciprocal operations.
//  // Decided to use standard rcp for now in hopes of optimizing it
//  float3 hitMin = minRGB * rcp(4.0 * maxRGB);
//  float3 hitMax = (peakC.xxx - maxRGB) * rcp(4.0 * minRGB + peakC.yyy);
//  float3 lobeRGB = max(-hitMin, hitMax);
//  float lobe = max(-0.1875, min(max(lobeRGB.r, max(lobeRGB.g, lobeRGB.b)), 0.0)) * Sharpness;
//
//  // Resolve, which needs medium precision rcp approximation to avoid visible tonality changes.
//  float rcpL = rcp(4.0 * lobe + 1.0);
//  float3 output = ((b + d + f + h) * lobe + e) * rcpL;
//
//  Dest[DTid.xy] = output;
//}
//)";	
	
public:
	bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState);
	void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
	bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* InMotionVectors, RcasConstants InConstants, ID3D12Resource* OutResource);

	float Sharpness = 0.3f;

	ID3D12Resource* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }

	RCAS_Dx12(std::string InName, ID3D12Device* InDevice);

	~RCAS_Dx12();
};