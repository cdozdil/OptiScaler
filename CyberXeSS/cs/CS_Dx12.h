#pragma once

#include "../pch.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include "../d3dx/d3dx12.h"


class CS_Dx12
{
private:
	bool _init = false;
	ID3D12RootSignature* _rootSignature = nullptr;
	ID3D12PipelineState* _pipelineState = nullptr;
	

	ID3D12Resource* _buffer = nullptr;
	D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;


public:
	bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState);
	void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
	bool Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* OutResource, uint32_t InNumThreadsX, uint32_t InNumThreadsY);

	ID3D12Resource* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }

	CS_Dx12(ID3D12Device* InDevice, const char* InShaderCode, const char* InEntry, const char* InTarget);

	~CS_Dx12();
};