#pragma once
#include "../../pch.h"

#include <d3d11.h>

class Bias_Dx11
{
private:
	struct alignas(256) InternalConstants
	{
		float Bias;
	};

	std::string _name = "";
	bool _init = false;
	int _counter = 0;

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
	bool Dispatch(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, ID3D11Texture2D* InResource, float InBias, ID3D11Texture2D* OutResource);

	ID3D11Texture2D* Buffer() { return _buffer; }
	bool IsInit() const { return _init; }
	bool CanRender() const { return _init && _buffer != nullptr; }

	Bias_Dx11(std::string InName, ID3D11Device* InDevice);

	~Bias_Dx11();
};