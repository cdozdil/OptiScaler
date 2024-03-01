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
	ID3D11Device5* Dx11Device = nullptr;
	ID3D11DeviceContext4* Dx11DeviceContext = nullptr;

	D3D11_TEXTURE2D_RESOURCE_C dx11Color = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Mv = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Depth = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Tm = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Exp = {};
	D3D11_TEXTURE2D_RESOURCE_C dx11Out = {};

	bool CopyTextureFrom11To12(ID3D11Resource* d3d11texture, ID3D11Texture2D** pSharedTexture, D3D11_TEXTURE2D_DESC_C* sharedDesc, bool copy);
	void ReleaseSharedResources();
	static void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter** ppAdapter, D3D_FEATURE_LEVEL featureLevel, bool requestHighPerformanceAdapter);
	static HRESULT CreateDx12Device(D3D_FEATURE_LEVEL featureLevel);

protected:

public:
	XeSSFeatureDx11(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters) : XeSSFeature(handleId, InParameters), IFeature_Dx11(handleId, InParameters)
	{
	}

	// Inherited via XeSSFeature
	void ReInit(const NVSDK_NGX_Parameter* InParameters) override;
	void SetInit(bool value) override;
	bool IsInited() override;

	// Inherited via IFeature_Dx11
	bool Init(ID3D11Device* device, ID3D11DeviceContext* context, const NVSDK_NGX_Parameter* initParams) override;
	bool Evaluate(ID3D11DeviceContext* deviceContext, const NVSDK_NGX_Parameter* initParams) override;

	~XeSSFeatureDx11();
};