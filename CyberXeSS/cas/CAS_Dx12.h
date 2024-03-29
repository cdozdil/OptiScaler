#pragma once

#include "../pch.h"

#include "include/ffx_cas.h"
#include "include/backends/dx12/ffx_dx12.h"

#include <d3d12.h>
#include <string>

class CAS_Dx12
{
private:
	bool _init = false;
	uint32_t _width = 0;
	uint32_t _height = 0;

	FfxCas::FfxCasContext _context{};
	FfxCas::FfxCasContextDescription _contextDesc{};

	ID3D12Device* Device = nullptr;

	ID3D12Resource* _buffer = nullptr;
	D3D12_RESOURCE_STATES _bufferState = D3D12_RESOURCE_STATE_COMMON;


public:
	bool CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState);
	void SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState);
	bool Dispatch(ID3D12CommandList* InCommandList, float InSharpness, ID3D12Resource* InResource, ID3D12Resource* OutResource);
	
	ID3D12Resource* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }

	CAS_Dx12(ID3D12Device* InDevice, uint32_t InWidth, uint32_t InHeight, int colorSpace);

	~CAS_Dx12();
};