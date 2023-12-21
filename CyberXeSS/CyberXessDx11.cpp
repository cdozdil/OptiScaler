#include "pch.h"
#include "Config.h"
#include "CyberXess.h"
#include "Util.h"
#include "detours.h"

#ifdef _DEBUG
#pragma comment(lib, "detours.d.lib")
#else
#pragma comment(lib, "detours.r.lib")
#endif // DEBUG

//#define ALWAYS_COPY_OUTPUT

static volatile int listCount = 0;
decltype(&ID3D11Device::CreateTexture2D) ptrCreateTexture2D = nullptr;
decltype(&ID3D11Device3::CreateTexture2D1) ptrCreateTexture2D1 = nullptr;

typedef enum _texture_type_t
{
	COLOR,
	MOTIONV,
	DEPTH,
	TMASK,
	EXPOSURE,
	OUTPUT
} texture_type_t;

typedef struct D3D11_TEXTURE2D_DESC_C
{
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
	UINT BindFlags;
	void* pointer;
} 	D3D11_TEXTURE2D_DESC_C;

//static D3D11_TEXTURE2D_DESC_C colorDesc = {};
//static D3D11_TEXTURE2D_DESC_C mvDesc = {};
//static D3D11_TEXTURE2D_DESC_C depthDesc = {};
//static D3D11_TEXTURE2D_DESC_C tmDesc = {};
//static D3D11_TEXTURE2D_DESC_C expDesc = {};
//static D3D11_TEXTURE2D_DESC_C outDesc = {};

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; \
dest.Height = src.Height; \
dest.Format = src.Format; \
dest.BindFlags = src.BindFlags; \

static HANDLE colorHandle = NULL;
static HANDLE mvHandle = NULL;
static HANDLE depthHandle = NULL;
static HANDLE outHandle = NULL;
static HANDLE tmHandle = NULL;
static HANDLE expHandle = NULL;
static ID3D11Resource* colorShared = nullptr;
static ID3D11Resource* mvShared = nullptr;
static ID3D11Resource* depthShared = nullptr;
static ID3D11Resource* outShared = nullptr;
static ID3D11Resource* tmShared = nullptr;
static ID3D11Resource* expShared = nullptr;

static ID3D12Fence* d3d12Fence = nullptr;
static ID3D11Fence* d3d11Fence = nullptr;
static HANDLE fenceHandle = NULL;

xess_d3d12_execute_params_t params{};

static int inWidth = 0;
static int outWidth = 0;

std::atomic<bool> isDeviceBusy(false);
std::atomic<bool> xessActive(false);

// xess log callback
inline void LogCallback(const char* Message, xess_logging_level_t Level)
{
	std::string s = Message;
	LOG("XeSS Runtime (" + std::to_string(Level) + ") : " + s, LEVEL_DEBUG);
}

#pragma region D3D11 hooks

HRESULT WINAPI hk_ID3D11Device_CreateTexture2D(ID3D11Device* This, const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D)
{
	if (ptrCreateTexture2D == nullptr)
		return E_INVALIDARG;

	//if (xessActive.load() && pDesc->MipLevels == 1 && (pDesc->MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 &&
	//	(pDesc->Width == 1 || (pDesc->Width != pDesc->Height && pDesc->Width > 580)) &&
	//	(
	//		(pDesc->Width == 1 && pDesc->Height == 1) ||
	//		(
	//			(pDesc->BindFlags == colorDesc.BindFlags && pDesc->Format == colorDesc.Format) ||
	//			(pDesc->BindFlags == depthDesc.BindFlags && pDesc->Format == depthDesc.Format) ||
	//			(pDesc->BindFlags == outDesc.BindFlags && pDesc->Format == outDesc.Format) ||
	//			(pDesc->BindFlags == mvDesc.BindFlags && pDesc->Format == mvDesc.Format) ||
	//			(pDesc->BindFlags == tmDesc.BindFlags && pDesc->Format == tmDesc.Format) ||
	//			(pDesc->BindFlags == expDesc.BindFlags && pDesc->Format == expDesc.Format)
	//			)
	//		)
	//	)
	//{
	//	LOG("hk_ID3D11Device_CreateTexture2D marked D3D11_RESOURCE_MISC_SHARED", LEVEL_DEBUG);

	//	auto makeShared = false;

	//	if ((pDesc->Width == 1 && pDesc->Height == 1) || (pDesc->BindFlags == expDesc.BindFlags && pDesc->Format == expDesc.Format))
	//		makeShared = true;

	//	D3D11_TEXTURE2D_DESC desc = *pDesc;
	//	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
	//	auto hr = (This->*ptrCreateTexture2D)(&desc, pInitialData, ppTexture2D);
	//	return hr;
	//}

	return (This->*ptrCreateTexture2D)(pDesc, pInitialData, ppTexture2D);
}

HRESULT WINAPI hk_ID3D11Device_CreateTexture2D1(ID3D11Device3* This, const D3D11_TEXTURE2D_DESC1* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D1** ppTexture2D)
{
	if (ptrCreateTexture2D1 == nullptr)
		return E_INVALIDARG;

	//if (xessActive.load() && pDesc->MipLevels == 1 && (pDesc->MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 &&
	//	(pDesc->Width == 1 || (pDesc->Width != pDesc->Height && pDesc->Width > 580)) &&
	//	(
	//		(pDesc->Width == 1 && pDesc->Height == 1) ||
	//		(
	//			(pDesc->BindFlags == colorDesc.BindFlags && pDesc->Format == colorDesc.Format) ||
	//			(pDesc->BindFlags == depthDesc.BindFlags && pDesc->Format == depthDesc.Format) ||
	//			(pDesc->BindFlags == outDesc.BindFlags && pDesc->Format == outDesc.Format) ||
	//			(pDesc->BindFlags == mvDesc.BindFlags && pDesc->Format == mvDesc.Format) ||
	//			(pDesc->BindFlags == tmDesc.BindFlags && pDesc->Format == tmDesc.Format) ||
	//			(pDesc->BindFlags == expDesc.BindFlags && pDesc->Format == expDesc.Format)
	//			)
	//		)
	//	)
	//{
	//	LOG("hk_ID3D11Device_CreateTexture2D1 marked D3D11_RESOURCE_MISC_SHARED", LEVEL_DEBUG);
	//	D3D11_TEXTURE2D_DESC1 desc = *pDesc;
	//	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
	//	auto hr = (This->*ptrCreateTexture2D1)(&desc, pInitialData, ppTexture2D);
	//	return hr;
	//}

	return (This->*ptrCreateTexture2D1)(pDesc, pInitialData, ppTexture2D);
}

void AttachToD3D11()
{
	return;

	if (ptrCreateTexture2D == nullptr)
	{
		LOG("Attach to CreateTexture2D", LEVEL_INFO);
		*(uintptr_t*)&ptrCreateTexture2D = Detours::X64::DetourClassVTable(*(uintptr_t*)CyberXessContext::instance()->Dx11Device, &hk_ID3D11Device_CreateTexture2D, 5);
	}

	if (ptrCreateTexture2D1 == nullptr)
	{
		LOG("Attach to CreateTexture2D1", LEVEL_INFO);
		*(uintptr_t*)&ptrCreateTexture2D1 = Detours::X64::DetourClassVTable(*(uintptr_t*)CyberXessContext::instance()->Dx11Device, &hk_ID3D11Device_CreateTexture2D1, 5);
	}
}

#pragma endregion

#pragma region Texture copy&share methods 

void ReleaseSharedResources()
{
	LOG("ReleaseSharedResources!", LEVEL_INFO);

	if (colorShared != nullptr)
	{
		colorShared->Release();
		colorShared = nullptr;
	}

	if (mvShared != nullptr)
	{
		mvShared->Release();
		mvShared = nullptr;
	}

	if (depthShared != nullptr)
	{
		depthShared->Release();
		depthShared = nullptr;
	}

	if (outShared != nullptr)
	{
		outShared->Release();
		outShared = nullptr;
	}

	if (tmShared != nullptr)
	{
		tmShared->Release();
		tmShared = nullptr;
	}

	if (expShared != nullptr)
	{
		expShared->Release();
		expShared = nullptr;
	}
}

static bool CreateFeature11(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* handle)
{
	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	isDeviceBusy.store(true);

	LOG("CreateFeature11 Start!", LEVEL_INFO);


#pragma region Read XeSS Version

	xess_version_t ver;
	xess_result_t ret = xessGetVersion(&ver);
	LOG("CreateFeature11 xessGetVersion result: " + ResultToString(ret), LEVEL_ERROR);

	if (ret == XESS_RESULT_SUCCESS)
	{
		char buf[128];
		sprintf_s(buf, "%u.%u.%u", ver.major, ver.minor, ver.patch);

		std::string m_VersionStr = buf;

		LOG("CreateFeature11 XeSS Version - " + m_VersionStr, LEVEL_WARNING);
	}

#pragma endregion

#pragma region Check for Dx12Device Device

	if (CyberXessContext::instance()->Dx12Device == nullptr)
	{
		if (InCmdList == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature InCmdList is null!!!", LEVEL_ERROR);
			isDeviceBusy.store(false);
			return false;
		}

		LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device is null trying to get from InCmdList!", LEVEL_WARNING);
		InCmdList->GetDevice(IID_PPV_ARGS(&CyberXessContext::instance()->Dx12Device));

		if (CyberXessContext::instance()->Dx12Device == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device can't receive from InCmdList!", LEVEL_ERROR);
			isDeviceBusy.store(false);
			return false;
		}
	}
	else
		LOG("NVSDK_NGX_D3D11_CreateFeature CyberXessContext::instance()->Dx12Device is OK!", LEVEL_DEBUG);

#pragma endregion

	auto inParams = CyberXessContext::instance()->CreateFeatureParams;
	auto deviceContext = CyberXessContext::instance()->Contexts[handle->Id].get();

	if (deviceContext == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature deviceContext is null!", LEVEL_ERROR);
		isDeviceBusy.store(false);
		return false;
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature deviceContext ok, xessD3D12CreateContext start", LEVEL_DEBUG);

	if (deviceContext->XessContext != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature Destrying old XeSSContext", LEVEL_WARNING);
		ret = xessDestroyContext(deviceContext->XessContext);
		LOG("NVSDK_NGX_D3D11_CreateFeature xessDestroyContext result -> " + ResultToString(ret), LEVEL_WARNING);
	}

	ret = xessD3D12CreateContext(CyberXessContext::instance()->Dx12Device, &deviceContext->XessContext);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12CreateContext result -> " + ResultToString(ret), LEVEL_INFO);

	ret = xessSetLoggingCallback(deviceContext->XessContext, XESS_LOGGING_LEVEL_DEBUG, LogCallback);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessSetLoggingCallback : " + ResultToString(ret), LEVEL_DEBUG);

	ret = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);
	LOG("NVSDK_NGX_D3D11_CreateFeature xessSetVelocityScale : " + ResultToString(ret), LEVEL_DEBUG);

#pragma region Create Parameters for XeSS

	xess_d3d12_init_params_t initParams{};

	LOG("NVSDK_NGX_D3D11_CreateFeature Params Init!", LEVEL_DEBUG);
	initParams.outputResolution.x = inParams->OutWidth;
	LOG("NVSDK_NGX_D3D11_CreateFeature initParams.outputResolution.x : " + std::to_string(initParams.outputResolution.x), LEVEL_DEBUG);
	initParams.outputResolution.y = inParams->OutHeight;
	LOG("NVSDK_NGX_D3D11_CreateFeature initParams.outputResolution.y : " + std::to_string(initParams.outputResolution.y), LEVEL_DEBUG);

	switch (inParams->PerfQualityValue)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
		initParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		initParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		initParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED;
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		initParams.qualitySetting = XESS_QUALITY_SETTING_QUALITY;
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		initParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;
		break;
	default:
		initParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing fsr ultra quality mode
		break;
	}

	initParams.initFlags = XESS_INIT_FLAG_NONE;

	if (CyberXessContext::instance()->MyConfig->DepthInverted.value_or(inParams->DepthInverted))
	{
		initParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
		CyberXessContext::instance()->MyConfig->DepthInverted = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (DepthInverted) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->AutoExposure.value_or(inParams->AutoExposure))
	{
		initParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		CyberXessContext::instance()->MyConfig->AutoExposure = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	else
	{
		initParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (!AutoExposure) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->HDR.value_or(!inParams->Hdr))
	{
		initParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		CyberXessContext::instance()->MyConfig->HDR = false;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (HDR) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->JitterCancellation.value_or(inParams->JitterMotion))
	{
		initParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		CyberXessContext::instance()->MyConfig->JitterCancellation = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (JitterCancellation) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}
	if (CyberXessContext::instance()->MyConfig->DisplayResolution.value_or(!inParams->LowRes))
	{
		initParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		CyberXessContext::instance()->MyConfig->DisplayResolution = true;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (LowRes) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	if (!CyberXessContext::instance()->MyConfig->DisableReactiveMask.value_or(true))
	{
		initParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		LOG("NVSDK_NGX_D3D11_CreateFeature initParams.initFlags (DisableReactiveMask) " + std::to_string(initParams.initFlags), LEVEL_INFO);
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature Params done!", LEVEL_DEBUG);

#pragma endregion

#pragma region Build Pipelines

	if (CyberXessContext::instance()->MyConfig->BuildPipelines.value_or(true))
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12BuildPipelines start!", LEVEL_DEBUG);

		ret = xessD3D12BuildPipelines(deviceContext->XessContext, NULL, false, initParams.initFlags);

		if (ret != XESS_RESULT_SUCCESS)
		{
			LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12BuildPipelines error : -> " + ResultToString(ret), LEVEL_ERROR);
			isDeviceBusy.store(false);
			return false;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature skipping xessD3D12BuildPipelines!", LEVEL_DEBUG);
	}

#pragma endregion

#pragma region Select Network Model

	auto model = static_cast<xess_network_model_t>(CyberXessContext::instance()->MyConfig->NetworkModel.value_or(0));

	LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel trying to set value to " + std::to_string(model), LEVEL_DEBUG);

	ret = xessSelectNetworkModel(deviceContext->XessContext, model);

	if (ret == XESS_RESULT_SUCCESS)
		LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel set to " + std::to_string(model), LEVEL_DEBUG);
	else
		LOG("NVSDK_NGX_D3D11_CreateFeature xessSelectNetworkModel(" + std::to_string(model) + ") error : " + ResultToString(ret), LEVEL_ERROR);

#pragma endregion


	LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12Init start!", LEVEL_DEBUG);

	ret = xessD3D12Init(deviceContext->XessContext, &initParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature xessD3D12Init error: " + ResultToString(ret), LEVEL_ERROR);
		CyberXessContext::instance()->init = false;
		isDeviceBusy.store(false);
		return false;
	}

	LOG("NVSDK_NGX_D3D11_CreateFeature End!", LEVEL_DEBUG);

	CyberXessContext::instance()->init = true;
	isDeviceBusy.store(false);
	return true;
}

FeatureContext* CreateContext11(NVSDK_NGX_Handle** OutHandle)
{
	auto deviceContext = CyberXessContext::instance()->CreateContext();
	*OutHandle = &deviceContext->Handle;
	return deviceContext;
}

HANDLE CopyTexture(ID3D11Texture2D* originalTexture, ID3D11Texture2D** pSharedTexture, bool copy = true)
{
	HANDLE handle;
	ID3D11Texture2D* sharedTexture = *pSharedTexture;
	D3D11_TEXTURE2D_DESC desc;
	HRESULT result;

	LOG("CopyTexture slow path :/", LEVEL_WARNING);

	originalTexture->GetDesc(&desc);

	// ok then lets kill the perf, we need shared version of this texture
	// so first create a 2d texture with same properties and shared flag
	//desc.CPUAccessFlags = 0;
	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

	// if it's output texture add UNORDERED ACCESS
	if (!copy)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	// Create shared texture
	if (sharedTexture != nullptr)
	{
		D3D11_TEXTURE2D_DESC sDesc;
		sharedTexture->GetDesc(&sDesc);

		if (sDesc.BindFlags != desc.BindFlags || sDesc.Format != desc.Format ||
			sDesc.Width != desc.Width || sDesc.Height != desc.Height)
		{
			sharedTexture->Release();
			result = CyberXessContext::instance()->Dx11Device->CreateTexture2D(&desc, nullptr, &sharedTexture);
			LOG("CopyTexture CreateTexture2D result: " + int_to_hex(result), LEVEL_DEBUG);
		}
		else
		{
			LOG("CopyTexture desc matches, skipping CreateTexture2D", LEVEL_DEBUG);
			result = S_OK;
		}
	}
	else
	{
		result = CyberXessContext::instance()->Dx11Device->CreateTexture2D(&desc, nullptr, &sharedTexture);
		LOG("CopyTexture CreateTexture2D result: " + int_to_hex(result), LEVEL_DEBUG);
	}

	if (result != S_OK)
		return NULL;

	// if it's not output texture copy it to use it with xess
	if (copy)
	{
		CyberXessContext::instance()->Dx11DeviceContext->CopyResource(sharedTexture, originalTexture);
		LOG("CopyTexture CreateTexture2D CopyResource!", LEVEL_DEBUG);
	}

	// Query resource
	IDXGIResource1* resource;
	result = sharedTexture->QueryInterface(IID_PPV_ARGS(&resource));
	LOG("CopyTexture QueryInterface(resource) result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	// Get shared handle
	result = resource->GetSharedHandle(&handle);
	LOG("CopyTexture CreateSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	resource->Release();

	*pSharedTexture = sharedTexture;

	return handle;
}

HANDLE CopyTexture1(ID3D11Texture2D1* originalTexture, ID3D11Texture2D1** pSharedTexture, bool copy = true)
{
	HANDLE handle;
	ID3D11Texture2D1* sharedTexture = *pSharedTexture;
	D3D11_TEXTURE2D_DESC1 desc;
	HRESULT result;

	LOG("CopyTexture1 slow path :/", LEVEL_WARNING);

	originalTexture->GetDesc1(&desc);

	// ok then lets kill the perf, we need shared version of this texture
	// so first create a 2d texture with same properties and shared flag
	//desc.CPUAccessFlags = 0;
	desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

	// if it's output texture add UNORDERED ACCESS
	if (!copy)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	// Create shared texture
	if (sharedTexture != nullptr)
	{
		D3D11_TEXTURE2D_DESC sDesc;
		sharedTexture->GetDesc(&sDesc);

		if (sDesc.BindFlags != desc.BindFlags || sDesc.Format != desc.Format ||
			sDesc.Width != desc.Width || sDesc.Height != desc.Height)
		{
			sharedTexture->Release();
			result = CyberXessContext::instance()->Dx11Device->CreateTexture2D1(&desc, nullptr, &sharedTexture);
			LOG("CopyTexture1 CreateTexture2D result: " + int_to_hex(result), LEVEL_DEBUG);
		}
		else
		{
			LOG("CopyTexture1 desc matches, skipping CreateTexture2D", LEVEL_DEBUG);
			result = S_OK;
		}
	}
	else
	{
		result = CyberXessContext::instance()->Dx11Device->CreateTexture2D1(&desc, nullptr, &sharedTexture);
		LOG("CopyTexture1 CreateTexture2D result: " + int_to_hex(result), LEVEL_DEBUG);
	}

	if (result != S_OK)
		return NULL;

	// if it's not output texture copy it to use it with xess
	if (copy)
	{
		CyberXessContext::instance()->Dx11DeviceContext->CopyResource(sharedTexture, originalTexture);
		LOG("CopyTexture1 CreateTexture2D CopyResource!", LEVEL_DEBUG);
	}

	// Query resource
	IDXGIResource1* resource;
	result = sharedTexture->QueryInterface(IID_PPV_ARGS(&resource));
	LOG("CopyTexture1 QueryInterface(resource) result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	// Get shared handle
	result = resource->GetSharedHandle(&handle);
	LOG("CopyTexture1 CreateSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	resource->Release();

	*pSharedTexture = sharedTexture;

	return handle;
}

HANDLE ShareTexture(ID3D11Texture2D* originalTexture, ID3D11Texture2D** pSharedTexture)
{
	HANDLE handle;

	// Query resource
	IDXGIResource1* resource;

	auto result = originalTexture->QueryInterface(IID_PPV_ARGS(&resource));
	LOG("ShareTexture QueryInterface(resource) result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	// Get shared handle
	result = resource->GetSharedHandle(&handle);
	LOG("ShareTexture CreateSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	resource->Release();

	// to release at end of frame
	*pSharedTexture = originalTexture;

	return handle;
}

HANDLE ShareTexture1(ID3D11Texture2D1* originalTexture, ID3D11Texture2D1** pSharedTexture)
{
	HANDLE handle;

	// Query resource
	IDXGIResource1* resource;

	auto result = originalTexture->QueryInterface(IID_PPV_ARGS(&resource));
	LOG("ShareTexture1 QueryInterface(resource) result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	// Get shared handle
	result = resource->GetSharedHandle(&handle);
	LOG("ShareTexture1 CreateSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
		return NULL;

	resource->Release();

	// to release at end of frame
	*pSharedTexture = originalTexture;

	return handle;
}

HANDLE CopyTextureFrom11To12(ID3D11Resource* d3d11texture, ID3D11Resource** pSharedTexture, texture_type_t texType)
{
	ID3D11Texture2D* originalTexture = nullptr;
	ID3D11Texture2D1* originalTexture1 = nullptr;
	ID3D11Texture2D* sharedTexture = nullptr;
	ID3D11Texture2D1* sharedTexture1 = nullptr;
	D3D11_TEXTURE2D_DESC desc;
	D3D11_TEXTURE2D_DESC1 desc1;
	HANDLE handle;
	HRESULT result;
	bool copy = true;

	// Get texture
	result = d3d11texture->QueryInterface(IID_PPV_ARGS(&originalTexture1));
	LOG("CopyTextureFrom11To12 QueryInterface(texture2d) result: " + int_to_hex(result), LEVEL_DEBUG);

	if (result != S_OK)
	{
		result = d3d11texture->QueryInterface(IID_PPV_ARGS(&originalTexture));

		if (result != S_OK)
			return NULL;

		if (*pSharedTexture != nullptr)
		{
			result = (*pSharedTexture)->QueryInterface(IID_PPV_ARGS(&sharedTexture));

			if (result != S_OK)
				return NULL;
		}

		originalTexture->GetDesc(&desc);

		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
			LOG("texture is shared!", LEVEL_DEBUG);

		handle = CopyTexture(originalTexture, &sharedTexture, texType != OUTPUT);
		originalTexture = reinterpret_cast<ID3D11Texture2D*>(sharedTexture);

//
//#ifdef ALWAYS_COPY_OUTPUT
//		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 || texType == OUTPUT)
//#else
//		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
//#endif // ALWAYS_COPY_OUTPUT
//		{
//			switch (texType)
//			{
//			case COLOR:
//				ASSIGN_DESC(colorDesc, desc)
//					colorDesc.pointer = d3d11texture;
//					break;
//
//			case MOTIONV:
//				ASSIGN_DESC(mvDesc, desc)
//					mvDesc.pointer = d3d11texture;
//					break;
//
//			case DEPTH:
//				ASSIGN_DESC(depthDesc, desc)
//					depthDesc.pointer = d3d11texture;
//					break;
//
//			case TMASK:
//				ASSIGN_DESC(tmDesc, desc)
//					tmDesc.pointer = d3d11texture;
//					break;
//
//			case EXPOSURE:
//				ASSIGN_DESC(expDesc, desc)
//					expDesc.pointer = d3d11texture;
//					break;
//
//			case OUTPUT:
//				ASSIGN_DESC(outDesc, desc)
//					outDesc.pointer = d3d11texture;
//					copy = false;
//				break;
//
//			}
//
//			handle = CopyTexture(originalTexture, &sharedTexture, copy);
//		}
//		else
//			handle = ShareTexture(originalTexture, &sharedTexture);

		if (handle != NULL)
			*pSharedTexture = sharedTexture;

		return handle;
	}
	else
	{
		if (*pSharedTexture != nullptr)
		{
			result = (*pSharedTexture)->QueryInterface(IID_PPV_ARGS(&sharedTexture1));

			if (result != S_OK)
				return NULL;
		}

		originalTexture1->GetDesc1(&desc1);
		if ((desc1.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
			LOG("texture1 is shared!", LEVEL_DEBUG);

		handle = CopyTexture1(originalTexture1, &sharedTexture1, texType != OUTPUT);
		originalTexture1 = reinterpret_cast<ID3D11Texture2D1*>(sharedTexture1);


		// Get desc of original texture and create shared desc
//
//#ifdef ALWAYS_COPY_OUTPUT
//		if ((desc1.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0 || texType == OUTPUT)
//#else
//		if ((desc1.MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0)
//#endif // ALWAYS_COPY_OUTPUT
//		{
//			switch (texType)
//			{
//			case COLOR:
//				ASSIGN_DESC(colorDesc, desc1)
//					colorDesc.pointer = d3d11texture;
//				break;
//
//			case MOTIONV:
//				ASSIGN_DESC(mvDesc, desc1)
//					mvDesc.pointer = d3d11texture;
//				break;
//
//			case DEPTH:
//				ASSIGN_DESC(depthDesc, desc1)
//					depthDesc.pointer = d3d11texture;
//				break;
//
//			case TMASK:
//				ASSIGN_DESC(tmDesc, desc1)
//					tmDesc.pointer = d3d11texture;
//				break;
//
//			case EXPOSURE:
//				ASSIGN_DESC(expDesc, desc1)
//					expDesc.pointer = d3d11texture;
//				break;
//
//			case OUTPUT:
//				ASSIGN_DESC(outDesc, desc1)
//					outDesc.pointer = d3d11texture;
//				copy = false;
//				break;
//
//			}
//
//			handle = CopyTexture1(originalTexture1, &sharedTexture1, copy);
//		}
//		else
//			handle = ShareTexture1(originalTexture1, &sharedTexture1);

		if (handle != NULL)
			*pSharedTexture = sharedTexture1;

		return handle;
	}
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Init

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_Ext(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion, unsigned long long unknown0)
{
	LOG("NVSDK_NGX_D3D11_Init_Ext AppId:" + std::to_string(InApplicationId), LEVEL_INFO);
	LOG("NVSDK_NGX_D3D11_Init_Ext SDK:" + std::to_string(InSDKVersion), LEVEL_INFO);

	ReleaseSharedResources();
	CyberXessContext::instance()->init = false;
	CyberXessContext::instance()->Shutdown(true, true);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init AppId:" + std::to_string(InApplicationId), LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_API NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	LOG("NVSDK_NGX_D3D11_Init_ProjectID Init!", LEVEL_DEBUG);
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_Init_with_ProjectID(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D11Device* InDevice, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo, NVSDK_NGX_Version InSDKVersion)
{
	std::string pId = InProjectId;
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID : " + pId, LEVEL_DEBUG);
	LOG("NVSDK_NGX_D3D11_Init_with_ProjectID SDK:" + std::to_string(InSDKVersion), LEVEL_DEBUG);

	return NVSDK_NGX_D3D11_Init_Ext(0x1337, InApplicationDataPath, InDevice, InFeatureInfo, InSDKVersion, 0);
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11_Shutdown

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown(void)
{
	LOG("NVSDK_NGX_D3D11_Shutdown", LEVEL_INFO);

	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	// Close D3D12 device
	if (CyberXessContext::instance()->Dx12Device != nullptr && CyberXessContext::instance()->Dx12CommandQueue != nullptr &&
		CyberXessContext::instance()->Dx12CommandList[0] != nullptr && CyberXessContext::instance()->Dx12CommandList[1] != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 resources", LEVEL_DEBUG);
		ID3D12Fence* d3d12Fence;
		CyberXessContext::instance()->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
		CyberXessContext::instance()->Dx12CommandQueue->Signal(d3d12Fence, 999);
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 fence created and signalled", LEVEL_DEBUG);

		CyberXessContext::instance()->Dx12CommandList[0]->Close();
		CyberXessContext::instance()->Dx12CommandList[1]->Close();
		ID3D12CommandList* ppCommandLists[] = { CyberXessContext::instance()->Dx12CommandList[0], CyberXessContext::instance()->Dx12CommandList[1] };
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 command list executing", LEVEL_DEBUG);
		CyberXessContext::instance()->Dx12CommandQueue->ExecuteCommandLists(2, ppCommandLists);

		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 waiting signal", LEVEL_DEBUG);
		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		d3d12Fence->SetEventOnCompletion(999, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
		d3d12Fence->Release();
		LOG("NVSDK_NGX_D3D11_Shutdown: releasing d3d12 release done", LEVEL_DEBUG);
	}

	ReleaseSharedResources();
	CyberXessContext::instance()->Shutdown(true, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

	// close all xess contexts
	for (auto const& [key, val] : CyberXessContext::instance()->Contexts) {
		NVSDK_NGX_D3D11_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();
	xessActive.store(false);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_D3D11_Shutdown1(ID3D11Device* InDevice)
{
	LOG("NVSDK_NGX_D3D11_Shutdown1", LEVEL_INFO);

	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	// Close D3D12 device
	if (CyberXessContext::instance()->Dx12Device != nullptr && CyberXessContext::instance()->Dx12CommandQueue != nullptr &&
		CyberXessContext::instance()->Dx12CommandList[0] != nullptr && CyberXessContext::instance()->Dx12CommandList[1] != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_Shutdown1: releasing d3d12 resources", LEVEL_DEBUG);
		ID3D12Fence* d3d12Fence;
		CyberXessContext::instance()->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence));
		CyberXessContext::instance()->Dx12CommandQueue->Signal(d3d12Fence, 999);
		LOG("NVSDK_NGX_D3D11_Shutdown1: releasing d3d12 fence created and signalled", LEVEL_DEBUG);

		CyberXessContext::instance()->Dx12CommandList[0]->Close();
		CyberXessContext::instance()->Dx12CommandList[1]->Close();
		ID3D12CommandList* ppCommandLists[] = { CyberXessContext::instance()->Dx12CommandList[0], CyberXessContext::instance()->Dx12CommandList[1] };
		LOG("NVSDK_NGX_D3D11_Shutdown1: releasing d3d12 command list executing", LEVEL_DEBUG);
		CyberXessContext::instance()->Dx12CommandQueue->ExecuteCommandLists(2, ppCommandLists);

		LOG("NVSDK_NGX_D3D11_Shutdown1: releasing d3d12 waiting signal", LEVEL_DEBUG);
		auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		d3d12Fence->SetEventOnCompletion(999, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
		d3d12Fence->Release();
		LOG("NVSDK_NGX_D3D11_Shutdown1: releasing d3d12 release done", LEVEL_DEBUG);
	}

	ReleaseSharedResources();
	CyberXessContext::instance()->Shutdown(true, true);
	CyberXessContext::instance()->NvParameterInstance->Params.clear();

	// close all xess contexts
	for (auto const& [key, val] : CyberXessContext::instance()->Contexts) {
		NVSDK_NGX_D3D11_ReleaseFeature(&val->Handle);
	}

	CyberXessContext::instance()->Contexts.clear();
	xessActive.store(false);

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Parameters

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetParameters", LEVEL_DEBUG);

	*OutParameters = CyberXessContext::instance()->NvParameterInstance->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation 
NVSDK_NGX_Result NVSDK_NGX_D3D11_GetCapabilityParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_GetCapabilityParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_AllocateParameters(NVSDK_NGX_Parameter** OutParameters)
{
	LOG("NVSDK_NGX_D3D11_AllocateParameters", LEVEL_DEBUG);

	*OutParameters = NvParameter::instance()->AllocateParameters();
	return NVSDK_NGX_Result_Success;
}

//currently it's kind of hack still needs a proper implementation
NVSDK_NGX_Result NVSDK_NGX_D3D11_DestroyParameters(NVSDK_NGX_Parameter* InParameters)
{
	LOG("NVSDK_NGX_D3D11_DestroyParameters", LEVEL_DEBUG);

	NvParameter::instance()->DeleteParameters((NvParameter*)InParameters);
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetScratchBufferSize(NVSDK_NGX_Feature InFeatureId, const NVSDK_NGX_Parameter* InParameters, size_t* OutSizeInBytes)
{
	LOG("NVSDK_NGX_D3D11_GetScratchBufferSize -> 52428800", LEVEL_WARNING);

	*OutSizeInBytes = 52428800;
	return NVSDK_NGX_Result_Success;
}

#pragma endregion

#pragma region NVSDK_NGX_D3D11 Feature

NVSDK_NGX_Result NVSDK_NGX_D3D11_CreateFeature(ID3D11DeviceContext* InDevCtx, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle)
{
	LOG("NVSDK_NGX_D3D11_CreateFeature", LEVEL_DEBUG);

	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	isDeviceBusy.store(true);

	HRESULT result;

	ID3D11Device* device;
	InDevCtx->GetDevice(&device);

	result = device->QueryInterface(IID_PPV_ARGS(&CyberXessContext::instance()->Dx11Device));

	if (result != S_OK)
	{
		LOG("NVSDK_NGX_D3D11_CreateFeature QueryInterface ID3D11Device5 result: " + int_to_hex(result), LEVEL_DEBUG);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_FeatureNotFound;
	}

	ReleaseSharedResources();
	AttachToD3D11();

	// create d3d12 device
	auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
	result = CyberXessContext::instance()->CreateDx12Device(fl);

	if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
	{
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_NotInitialized;
	}

	auto cfResult = NVSDK_NGX_D3D12_CreateFeature(nullptr, InFeatureID, InParameters, OutHandle);
	LOG("NVSDK_NGX_D3D11_CreateFeature Handle: " + std::to_string((*OutHandle)->Id) + ", result: " + int_to_hex(cfResult), LEVEL_DEBUG);

	isDeviceBusy.store(false);
	return cfResult;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_ReleaseFeature(NVSDK_NGX_Handle* InHandle)
{
	if (InHandle == nullptr)
		return NVSDK_NGX_Result_Success;

	LOG("NVSDK_NGX_D3D11_ReleaseFeature Handle: " + std::to_string(InHandle->Id), LEVEL_DEBUG);

	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	isDeviceBusy.store(true);

	if (auto deviceContext = CyberXessContext::instance()->Contexts[InHandle->Id].get(); deviceContext->XessContext != nullptr)
	{
		auto result = xessDestroyContext(deviceContext->XessContext);
		deviceContext->XessContext = nullptr;
		LOG("NVSDK_NGX_D3D11_ReleaseFeature: xessDestroyContext result: " + ResultToString(result), LEVEL_DEBUG);
	}

	CyberXessContext::instance()->DeleteContext(InHandle);

	if (CyberXessContext::instance()->Contexts.empty())
	{
		ReleaseSharedResources();
		CyberXessContext::instance()->Shutdown(true, false);

	}

	isDeviceBusy.store(false);

	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_GetFeatureRequirements(IDXGIAdapter* Adapter, const NVSDK_NGX_FeatureDiscoveryInfo* FeatureDiscoveryInfo, NVSDK_NGX_FeatureRequirement* OutSupported)
{
	LOG("NVSDK_NGX_D3D11_GetFeatureRequirements", LEVEL_DEBUG);

	*OutSupported = NVSDK_NGX_FeatureRequirement();
	OutSupported->FeatureSupported = NVSDK_NGX_FeatureSupportResult_Supported;
	OutSupported->MinHWArchitecture = 0;
	//Some windows 10 os version
	strcpy_s(OutSupported->MinOSVersion, "10.0.19045.2728");
	return NVSDK_NGX_Result_Success;
}

NVSDK_NGX_Result NVSDK_NGX_D3D11_EvaluateFeature(ID3D11DeviceContext* InDevCtx, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback)
{
	LOG("NVSDK_NGX_D3D11_EvaluateFeature Handle: " + std::to_string(InFeatureHandle->Id), LEVEL_DEBUG);

	if (!xessActive.load())
	{
		ReleaseSharedResources();
		xessActive.store(true);
	}

	auto instance = CyberXessContext::instance();
	auto deviceContext = instance->Contexts[InFeatureHandle->Id].get();

	if (deviceContext == nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature trying to use released handle, returning NVSDK_NGX_Result_Success", LEVEL_DEBUG);
		return NVSDK_NGX_Result_Success;
	}

	while (isDeviceBusy.load())
	{
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	HRESULT result;

	isDeviceBusy.store(true);

	if (instance->Dx11Device == nullptr)
	{
		InDevCtx->GetDevice((ID3D11Device**)&instance->Dx11Device);

		// No D3D12 device!
		if (instance->Dx12Device == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device device!", LEVEL_DEBUG);
			ReleaseSharedResources();

			auto fl = CyberXessContext::instance()->Dx11Device->GetFeatureLevel();
			result = CyberXessContext::instance()->CreateDx12Device(fl);

			if (result != S_OK || CyberXessContext::instance()->Dx12Device == nullptr)
			{
				isDeviceBusy.store(false);
				return NVSDK_NGX_Result_FAIL_NotInitialized;
			}

			LOG("NVSDK_NGX_D3D11_EvaluateFeature no Dx12Device created!", LEVEL_DEBUG);

		}
	}

	if (instance->Dx11DeviceContext == nullptr)
	{
		result = InDevCtx->QueryInterface(IID_PPV_ARGS(&instance->Dx11DeviceContext));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature no ID3D11DeviceContext4 interface!", LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_FeatureNotSupported;
		}
	}

	auto listIndex = listCount % 2;

	// Command allocator & command list
	if (instance->Dx12CommandAllocator[listIndex] == nullptr)
	{
		//	CreateCommandAllocator 
		result = instance->Dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&instance->Dx12CommandAllocator[listIndex]));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandAllocator result: " + int_to_hex(result), LEVEL_DEBUG);

		if (result != S_OK)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}

		// CreateCommandList
		result = instance->Dx12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, instance->Dx12CommandAllocator[listIndex], nullptr, IID_PPV_ARGS(&instance->Dx12CommandList[listIndex]));
		LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateCommandList result: " + int_to_hex(result), LEVEL_DEBUG);

		if (result != S_OK)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
		}
	}

	// get params from dlss
	const auto inParams = static_cast<const NvParameter*>(InParameters);

	// init check
	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature init is false, calling CreateFeature!", LEVEL_WARNING);
		instance->init = CreateFeature11(nullptr, InFeatureHandle);
	}

	if (!instance->init)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature init still is null CreateFeature failed!", LEVEL_ERROR);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_UnableToInitializeFeature;
	}

	AttachToD3D11();

	// Fence for syncing
	if (d3d12Fence == nullptr)
	{
		result = instance->Dx12Device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&d3d12Fence));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence d3d12fence result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (fenceHandle == NULL)
	{
		result = instance->Dx12Device->CreateSharedHandle(d3d12Fence, NULL, GENERIC_ALL, nullptr, &fenceHandle);

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence fenceHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (d3d11Fence == nullptr)
	{
		result = instance->Dx11Device->OpenSharedFence(fenceHandle, IID_PPV_ARGS(&d3d11Fence));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature CreateFence d3d11fence result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	// creatimg params for XeSS
	xess_result_t xessResult;

	params.jitterOffsetX = inParams->JitterOffsetX;
	params.jitterOffsetY = inParams->JitterOffsetY;

	params.exposureScale = inParams->ExposureScale;
	params.resetHistory = inParams->ResetRender;

	params.inputWidth = inParams->Width;
	params.inputHeight = inParams->Height;
	LOG("NVSDK_NGX_D3D11_EvaluateFeature inp width: " + std::to_string(inParams->Width) + " height: " + std::to_string(inParams->Height), LEVEL_DEBUG);

#pragma region Texture copies

	if (listIndex > 0)
	{
		auto oldIndex = (listIndex + 1) % 2;

		if (params.pColorTexture != nullptr && params.pColorTexture != NULL)
		{
			//CloseHandle(colorHandle);
			colorHandle = NULL;
		}

		if (params.pVelocityTexture != nullptr && params.pVelocityTexture != NULL)
		{
			//CloseHandle(mvHandle);
			mvHandle = NULL;
		}

		if (params.pExposureScaleTexture != nullptr && params.pExposureScaleTexture != NULL)
		{
			//CloseHandle(expHandle);
			expHandle = NULL;
		}

		if (params.pDepthTexture != nullptr && params.pDepthTexture != NULL)
		{
			//CloseHandle(depthHandle);
			depthHandle = NULL;
		}

		if (params.pResponsivePixelMaskTexture != nullptr && params.pResponsivePixelMaskTexture != NULL)
		{
			//CloseHandle(tmHandle);
			tmHandle = NULL;
		}

		if (params.pOutputTexture != nullptr && params.pOutputTexture != NULL)
		{
			//CloseHandle(outHandle);
			outHandle = NULL;
		}
	}

	if (params.pColorTexture != nullptr && params.pColorTexture != NULL)
		params.pColorTexture->Release();

	if (params.pVelocityTexture != nullptr && params.pVelocityTexture != NULL)
		params.pVelocityTexture->Release();

	if (params.pExposureScaleTexture != nullptr && params.pExposureScaleTexture != NULL)
		params.pExposureScaleTexture->Release();

	if (params.pDepthTexture != nullptr && params.pDepthTexture != NULL)
		params.pDepthTexture->Release();

	if (params.pResponsivePixelMaskTexture != nullptr && params.pResponsivePixelMaskTexture != NULL)
		params.pResponsivePixelMaskTexture->Release();

	if (params.pOutputTexture != nullptr && params.pOutputTexture != NULL)
		params.pOutputTexture->Release();

	ID3D11Query* query1;
	D3D11_QUERY_DESC pQueryDesc;
	pQueryDesc.Query = D3D11_QUERY_EVENT;
	pQueryDesc.MiscFlags = 0;
	instance->Dx11Device->CreateQuery(&pQueryDesc, &query1);

	// Associate the query with the copy operation
	instance->Dx11DeviceContext->Begin(query1);

	if (inParams->Color != nullptr)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Color exist..", LEVEL_DEBUG);
		colorHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Color, &colorShared, COLOR);

		if (colorHandle == NULL)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Color not exist!!", LEVEL_ERROR);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->MotionVectors)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors exist..", LEVEL_DEBUG);
		mvHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->MotionVectors, &mvShared, MOTIONV);

		if (mvHandle == NULL)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors not exist!!", LEVEL_ERROR);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Output)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Output exist..", LEVEL_DEBUG);
		outHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Output, &outShared, OUTPUT);

		if (outHandle == NULL)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Output not exist!!", LEVEL_ERROR);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;
	}

	if (inParams->Depth)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth exist..", LEVEL_DEBUG);
		depthHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->Depth, &depthShared, DEPTH);

		if (depthHandle == NULL)
		{
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth not exist!!", LEVEL_ERROR);
		params.pDepthTexture = nullptr;
	}

	if (!instance->MyConfig->AutoExposure.value_or(false))
	{
		if (inParams->ExposureTexture == nullptr)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!", LEVEL_WARNING);
			params.pExposureScaleTexture = nullptr;
		}
		else
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature ExposureTexture exist..", LEVEL_DEBUG);
			expHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->ExposureTexture, &expShared, EXPOSURE);

			if (expHandle == NULL)
			{
				isDeviceBusy.store(false);
				return NVSDK_NGX_Result_FAIL_InvalidParameter;
			}
		}
	}
	else
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature AutoExposure enabled!", LEVEL_WARNING);
		params.pExposureScaleTexture = nullptr;
	}

	if (!instance->MyConfig->DisableReactiveMask.value_or(true))
	{
		if (inParams->TransparencyMask != nullptr)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask exist..", LEVEL_INFO);

			tmHandle = CopyTextureFrom11To12((ID3D11Resource*)inParams->TransparencyMask, &tmShared, TMASK);

			if (tmHandle == NULL)
			{
				isDeviceBusy.store(false);
				return NVSDK_NGX_Result_FAIL_InvalidParameter;
			}
		}
		else
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask not exist and its enabled in config, it may cause problems!!", LEVEL_WARNING);
			params.pResponsivePixelMaskTexture = nullptr;
		}
	}
	else
	{
		params.pResponsivePixelMaskTexture = nullptr;
	}

	// Execute dx11 commands 
	instance->Dx11DeviceContext->End(query1);

	// Wait for the query to be ready
	while (instance->Dx11DeviceContext->GetData(query1, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
		std::this_thread::yield(); // Yield to reduce CPU usage
	}

	// Release the query
	query1->Release();

	// Signal for copy & share complete
	instance->Dx11DeviceContext->Signal(d3d11Fence, 10);

	if (instance->Dx12CommandQueue == nullptr)
		LOG("NVSDK_NGX_D3D11_EvaluateFeature where is Dx12CommandQueue!!", LEVEL_WARNING);

	if (inParams->Color)
	{
		result = instance->Dx12Device->OpenSharedHandle(colorHandle, IID_PPV_ARGS(&params.pColorTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Color OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (inParams->MotionVectors)
	{
		result = instance->Dx12Device->OpenSharedHandle(mvHandle, IID_PPV_ARGS(&params.pVelocityTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature MotionVectors OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (inParams->Output)
	{
		result = instance->Dx12Device->OpenSharedHandle(outHandle, IID_PPV_ARGS(&params.pOutputTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Output OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (inParams->Depth && !instance->MyConfig->DisplayResolution.value_or(false))
	{
		result = instance->Dx12Device->OpenSharedHandle(depthHandle, IID_PPV_ARGS(&params.pDepthTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature Depth OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (!instance->MyConfig->AutoExposure.value_or(false) && inParams->ExposureTexture != nullptr)
	{
		result = instance->Dx12Device->OpenSharedHandle(expHandle, IID_PPV_ARGS(&params.pExposureScaleTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature ExposureTexture OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

	if (!instance->MyConfig->DisableReactiveMask.value_or(true) && inParams->TransparencyMask != nullptr)
	{
		result = instance->Dx12Device->OpenSharedHandle(tmHandle, IID_PPV_ARGS(&params.pResponsivePixelMaskTexture));

		if (result != S_OK)
		{
			LOG("NVSDK_NGX_D3D11_EvaluateFeature TransparencyMask OpenSharedHandle result: " + int_to_hex(result), LEVEL_DEBUG);
			isDeviceBusy.store(false);
			return NVSDK_NGX_Result_FAIL_InvalidParameter;
		}
	}

#pragma endregion

	LOG("NVSDK_NGX_D3D11_EvaluateFeature mvscale x: " + std::to_string(inParams->MVScaleX) + " y: " + std::to_string(inParams->MVScaleY), LEVEL_DEBUG);
	xessResult = xessSetVelocityScale(deviceContext->XessContext, inParams->MVScaleX, inParams->MVScaleY);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature xessSetVelocityScale : " + ResultToString(xessResult), LEVEL_ERROR);
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_Fail;
	}

	// wait for copy
	instance->Dx12CommandQueue->Wait(d3d12Fence, 10);

	// Execute xess
	LOG("NVSDK_NGX_D3D11_EvaluateFeature Executing!!", LEVEL_INFO);
	xessResult = xessD3D12Execute(deviceContext->XessContext, instance->Dx12CommandList[listIndex], &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature xessD3D12Execute result: " + ResultToString(xessResult), LEVEL_INFO);
		ReleaseSharedResources();
		isDeviceBusy.store(false);
		return NVSDK_NGX_Result_FAIL_InvalidParameter;

	}

	// Execute dx12 commands to process xess
	instance->Dx12CommandList[listIndex]->Close();
	ID3D12CommandList* ppCommandLists[] = { instance->Dx12CommandList[listIndex] };
	instance->Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	// xess done
	instance->Dx12CommandQueue->Signal(d3d12Fence, 20);

	//// wait for end of copy
	auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	d3d11Fence->SetEventOnCompletion(20, fenceEvent12);
	WaitForSingleObject(fenceEvent12, INFINITE);
	CloseHandle(fenceEvent12);

	//copy output back
	if (outShared != inParams->Output)
	{
		LOG("NVSDK_NGX_D3D11_EvaluateFeature Copy back!", LEVEL_DEBUG);
		ID3D11Query* query;
		D3D11_QUERY_DESC pQueryDesc;
		pQueryDesc.Query = D3D11_QUERY_EVENT;
		pQueryDesc.MiscFlags = 0;
		instance->Dx11Device->CreateQuery(&pQueryDesc, &query);

		// Associate the query with the copy operation
		instance->Dx11DeviceContext->Begin(query);
		instance->Dx11DeviceContext->CopyResource((ID3D11Resource*)inParams->Output, outShared);
		instance->Dx11DeviceContext->End(query);

		// Wait for the query to be ready
		while (instance->Dx11DeviceContext->GetData(query, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE) {
			std::this_thread::yield(); // Yield to reduce CPU usage
		}

		// Release the query
		query->Release();

		//instance->Dx11DeviceContext->CopyResource((ID3D11Resource*)inParams->Output, outShared);
		//instance->Dx11DeviceContext->Flush();
		//instance->Dx11DeviceContext->Signal(d3d11Fence, 30);

		////// wait for end of copy
		//auto fenceEvent11 = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		//d3d12Fence->SetEventOnCompletion(30, fenceEvent11);
		//WaitForSingleObject(fenceEvent11, INFINITE);
		//CloseHandle(fenceEvent11);
	}

	LOG("NVSDK_NGX_D3D11_EvaluateFeature Frame Ready: " + std::to_string(listCount), LEVEL_DEBUG);

	instance->Dx12CommandAllocator[listIndex]->Reset();
	instance->Dx12CommandList[listIndex]->Reset(instance->Dx12CommandAllocator[listIndex], nullptr);

	listCount++;

	isDeviceBusy.store(false);

	return NVSDK_NGX_Result_Success;
}

#pragma endregion

