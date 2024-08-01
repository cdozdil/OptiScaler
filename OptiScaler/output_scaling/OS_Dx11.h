#pragma once

#include "../pch.h"
#include "OS_Common.h"

#include <d3d11.h>

class OS_Dx11
{
private:
	std::string _name = "";
	bool _init = false;
	int _counter = 0;
	bool _upsample = false;

	ID3D11Device* _device = nullptr;
	
	ID3D11ComputeShader* _computeShader = nullptr;
	ID3D11Buffer* _constantBuffer = nullptr;
	ID3D11Texture2D* _buffer = nullptr;
	ID3D11ShaderResourceView* _srvInput = nullptr;
	ID3D11UnorderedAccessView* _uavOutput = nullptr;

	ID3D11Texture2D* _currentInResource = nullptr;
	ID3D11Texture2D* _currentOutResource = nullptr;

	uint32_t InNumThreadsX = 32;
	uint32_t InNumThreadsY = 32;
						
	bool InitializeViews(ID3D11Texture2D* InResource, ID3D11Texture2D* OutResource);

public:
	bool CreateBufferResource(ID3D11Device* InDevice, ID3D11Resource* InSource);
	bool Dispatch(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, ID3D11Texture2D* InResource, ID3D11Texture2D* OutResource);

	ID3D11Texture2D* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }
	bool IsUpsampling() { return _upsample; }
	bool CanRender() const { return _init && _buffer != nullptr; }

	OS_Dx11(std::string InName, ID3D11Device* InDevice, bool InUpsample);

	~OS_Dx11();
};