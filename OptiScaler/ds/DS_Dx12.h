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

class DS_Dx12
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

	uint32_t InNumThreadsX = 32;
	uint32_t InNumThreadsY = 32;
							 
	std::string downsampleCode = R"(
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

	ID3D12Device* _device = nullptr;
	ID3D12Resource* _buffer = nullptr;
	ID3D12Resource* _constantBuffer = nullptr;
	D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;

public:
	bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState);
	void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
	bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* OutResource);
	float Scale = 1.0f;

	ID3D12Resource* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }

	DS_Dx12(std::string InName, ID3D12Device* InDevice);

	~DS_Dx12();
};