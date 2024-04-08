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

	std::string downsampleCode = R"(
#define PI 3.141592

#define LANCZOS_N 3

Texture2D<float4> _SrcImage;
RWTexture2D<float4> _PixelValueWrite;

cbuffer params
{
    int _SrcWidth;
    int _SrcHeight;
    int _DstWidth;
    int _DstHeight;
};

bool IsValidPixelPosition(uint2 p, uint width, uint height)
{
    bool isValid = (p.x >= 0 && p.x < width  && p.y >= 0 && p.y < height);
    return isValid ? true : false;
}

float Sinc(float x)
{
    if(x == 0.0) return 1.0;
    return sin(PI * x) / (PI * x);
}

float Lanczos(float x, float n)
{
    if(abs(x) >= n) return 0.0;
    return Sinc(x) * Sinc(x/n);
}

[numthreads(16, 16, 1)]
void ResizeLanczos(uint3 id : SV_DispatchThreadID)
{
    if (!IsValidPixelPosition(id.xy, _DstWidth, _DstHeight))
        return;

    float2 range = float2(LANCZOS_N, LANCZOS_N);
    float2 scaleFactor = float2((float)_DstWidth/_SrcWidth, (float)_DstHeight/_SrcHeight);

    if(scaleFactor.x < 1.0)
        range.x *= 1.0/scaleFactor.x;
    else 
        range.x *= scaleFactor.x;

    if(scaleFactor.y < 1.0)
        range.y *= 1.0/scaleFactor.y;
    else
        range.y *= scaleFactor.y;

    float2 srcCenter = float2(_SrcWidth/2.0 - 0.5, _SrcHeight/2.0 - 0.5);
    float2 dstCenter = float2(_DstWidth/2.0 - 0.5, _DstHeight/2.0 - 0.5);

    float x = id.x - dstCenter.x;
    float y = id.y - dstCenter.y;
    float2 uv = float2(x/_DstWidth, y/_DstHeight);

    float2 srcPixelPos = float2(uv.x*_SrcWidth + srcCenter.x, uv.y*_SrcHeight + srcCenter.y);

    int startX = (int)clamp(srcPixelPos.x - range.x, 0, _SrcWidth - 1);
    int startY = (int)clamp(srcPixelPos.y - range.y, 0, _SrcHeight - 1);
    int endX   = (int)clamp(srcPixelPos.x + range.x, 0, _SrcWidth - 1);
    int endY   = (int)clamp(srcPixelPos.y + range.y, 0, _SrcHeight - 1);

    float scaleCorrectionX = LANCZOS_N / range.x;
    float scaleCorrectionY = LANCZOS_N / range.y;
    float totalWeight = 0.0;
    float4 color = float4(0, 0, 0, 0);
    for(int iy = startY; iy <= endY; iy++)
    {
        for(int ix = startX; ix <= endX; ix++)
        {
            float dx = (ix - srcPixelPos.x) * scaleCorrectionX;
            float dy = (iy - srcPixelPos.y) * scaleCorrectionY;

            float weight = Lanczos(dx, LANCZOS_N) * Lanczos(dy, LANCZOS_N);
            totalWeight += weight;
            color += _SrcImage[int2(ix, iy)] * weight;
        }
    }

    if(totalWeight > 0.0)
    {
        color = color / totalWeight;
    }

    _PixelValueWrite[id.xy] = color;
}
)";

	ID3D12Device* _device = nullptr;
	ID3D12Resource* _buffer = nullptr;
	ID3D12Resource* _constantBuffer = nullptr;
	D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;

public:
	bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState);
	void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
	bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* OutResource, uint32_t InNumThreadsX, uint32_t InNumThreadsY);
	float Scale = 1.0f;

	ID3D12Resource* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }

	DS_Dx12(std::string InName, ID3D12Device* InDevice);

	~DS_Dx12();
};