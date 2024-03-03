#pragma once
#include "IFeature_Dx11.h"
#include "XeSSFeature.h"
#include <string>
#include <d3d11_4.h>

using D3D11_TEXTURE2D_DESC_C = struct D3D11_TEXTURE2D_DESC_C
{
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
	UINT BindFlags;
	void* pointer = nullptr;
	HANDLE handle = NULL;
};

using D3D11_TEXTURE2D_RESOURCE_C = struct D3D11_TEXTURE2D_RESOURCE_C
{
	D3D11_TEXTURE2D_DESC_C Desc = {};
	HANDLE SharedHandle = NULL;
	ID3D11Texture2D* SharedTexture = nullptr;
};

class XeSSFeatureDx11 : public XeSSFeature, public IFeature_Dx11
{
private:
	// D3D11
	ID3D11Device5* Dx11Device = nullptr;
	ID3D11DeviceContext4* Dx11DeviceContext = nullptr;

	// D3D11with12
	ID3D12Device* Dx12on11Device = nullptr;
	ID3D12CommandQueue* Dx12CommandQueue = nullptr;
	ID3D12CommandAllocator* Dx12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList* Dx12CommandList = nullptr;

	D3D11_TEXTURE2D_RESOURCE_C dx11Color = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Mv = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Depth = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Tm = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Exp = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Out = {};

	bool CopyTextureFrom11To12(ID3D11Resource* InResource, ID3D11Texture2D** OutSharedResource, D3D11_TEXTURE2D_DESC_C* OutSharedDesc, bool InCopy);
	void ReleaseSharedResources();
	void GetHardwareAdapter(IDXGIFactory1* InFactory, IDXGIAdapter** InAdapter, D3D_FEATURE_LEVEL InFeatureLevel, bool InRequestHighPerformanceAdapter);
	HRESULT CreateDx12Device(D3D_FEATURE_LEVEL InFeatureLevel);

protected:

public:
	XeSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : XeSSFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	// Inherited via XeSSFeature
	void ReInit(const NVSDK_NGX_Parameter* InParameters) override;

	// Inherited via IFeature_Dx11
	bool Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters) override;

	~XeSSFeatureDx11();
};