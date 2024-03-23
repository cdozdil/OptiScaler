#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Dx11.h"

#define ASSIGN_DESC(dest, src) dest.Width = src.Width; dest.Height = src.Height; dest.Format = src.Format; dest.BindFlags = src.BindFlags; 

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);	

bool FSR2FeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11::Init");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (InitFSR2(InParameters)) 
	{
		Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
		return true;
	}

	return false;
}

bool FSR2FeatureDx11::CopyTexture(ID3D11Resource* InResource, D3D11_TEXTURE2D_RESOURCE_C* OutTextureRes, UINT bindFlags, bool InCopy)
{
	ID3D11Texture2D* originalTexture = nullptr;
	D3D11_TEXTURE2D_DESC desc{};

	auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));

	if (result != S_OK)
		return false;

	originalTexture->GetDesc(&desc);

	if (desc.BindFlags == bindFlags)
	{
		ASSIGN_DESC(OutTextureRes->Desc, desc);
		OutTextureRes->Texture = originalTexture;
		OutTextureRes->usingOriginal = true;
		return true;
	}

	if (OutTextureRes->usingOriginal || OutTextureRes->Texture == nullptr ||
		desc.Width != OutTextureRes->Desc.Width || desc.Height != OutTextureRes->Desc.Height ||
		desc.Format != OutTextureRes->Desc.Format || desc.BindFlags != OutTextureRes->Desc.BindFlags)
	{
		if (OutTextureRes->Texture != nullptr)
		{
			if (!OutTextureRes->usingOriginal)
				OutTextureRes->Texture->Release();
			else
				OutTextureRes->Texture = nullptr;
		}

		OutTextureRes->usingOriginal = false;
		ASSIGN_DESC(OutTextureRes->Desc, desc);

		desc.BindFlags = bindFlags;

		result = Device->CreateTexture2D(&desc, nullptr, &OutTextureRes->Texture);

		if (result != S_OK)
		{
			spdlog::error("FSR2FeatureDx11::CopyTexture CreateTexture2D error: {0:x}", result);
			return false;
		}
	}

	if (InCopy)
		DeviceContext->CopyResource(OutTextureRes->Texture, InResource);

	return true;
}

void FSR2FeatureDx11::ReleaseResources()
{
	spdlog::debug("FSR2FeatureDx11::ReleaseResources!");

	if (!bufferColor.usingOriginal)
	{
		SAFE_RELEASE(bufferColor.Texture);
	}
}

bool FSR2FeatureDx11::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx11::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeDX11();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = ffxFsr2GetInterfaceDX11(&_contextDesc.callbacks, Device, scratchBuffer, scratchBufferSize);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = ffxGetDeviceDX11(Device);
	_contextDesc.maxRenderSize.width = DisplayWidth();
	_contextDesc.maxRenderSize.height = DisplayHeight();
	_contextDesc.displaySize.width = DisplayWidth();
	_contextDesc.displaySize.height = DisplayHeight();

	_contextDesc.flags = 0;

	int featureFlags;
	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

	_initFlags = featureFlags;

	bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
	bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
	bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
	bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
	bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
	bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

	if (Config::Instance()->DepthInverted.value_or(DepthInverted))
	{
		Config::Instance()->DepthInverted = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INVERTED;
		SetDepthInverted(true);
		spdlog::info("FSR2FeatureDx11::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureDx11::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureDx11::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureDx11::InitFSR2 xessParams.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureDx11::InitFSR2 xessParams.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureDx11::InitFSR2 xessParams.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureDx11::InitFSR2 xessParams.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2FeatureDx11::InitFSR2 xessParams.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

#if _DEBUG
	_contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
	_contextDesc.fpMessage = FfxLogCallback;
#endif

	spdlog::debug("FSR2FeatureDx11::InitFSR2 ffxFsr2ContextCreate!");

	errorCode = ffxFsr2ContextCreate(&_context, &_contextDesc);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString(errorCode));
		return false;
	}

	SetInit(true);

	return true;
}

FSR2FeatureDx11::FSR2FeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
}

bool FSR2FeatureDx11::Evaluate(ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11::Evaluate");

	if (!IsInited())
		return false;

	FfxFsr2DispatchDescription params{};

	params.commandList = InContext;

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureDx11::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	ID3D11Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureDx11::Evaluate Color exist..");

		if (!CopyTexture(paramColor, &bufferColor, 40, true))
		{
			spdlog::debug("FSR2FeatureDx11::Evaluate Can't copy Color!");
			return false;
		}

		if (bufferColor.Texture != nullptr)
			params.color = ffxGetResourceDX11(&_context, bufferColor.Texture, (wchar_t*)L"FSR2_Color");
		else
			params.color = ffxGetResourceDX11(&_context, paramColor, (wchar_t*)L"FSR2_Color");
	}
	else
	{
		spdlog::error("FSR2FeatureDx11::Evaluate Color not exist!!");
		return false;
	}

	ID3D11Resource* paramVelocity;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR2FeatureDx11::Evaluate MotionVectors exist..");
		params.motionVectors = ffxGetResourceDX11(&_context, paramVelocity, (wchar_t*)L"FSR2_MotionVectors");
	}
	else
	{
		spdlog::error("FSR2FeatureDx11::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D11Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR2FeatureDx11::Evaluate Output exist..");
		params.output = ffxGetResourceDX11(&_context, paramOutput, (wchar_t*)L"FSR2_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR2FeatureDx11::Evaluate Output not exist!!");
		return false;
	}

	ID3D11Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureDx11::Evaluate Depth exist..");
		params.depth = ffxGetResourceDX11(&_context, paramDepth, (wchar_t*)L"FSR2_Depth");
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR2FeatureDx11::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR2FeatureDx11::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	ID3D11Resource* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

		if (paramExp)
			spdlog::debug("FSR2FeatureDx11::Evaluate ExposureTexture exist..");
		else
			spdlog::debug("FSR2FeatureDx11::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");

		params.exposure = ffxGetResourceDX11(&_context, paramExp, (wchar_t*)L"FSR2_Exposure");
	}
	else
		spdlog::debug("FSR2FeatureDx11::Evaluate AutoExposure enabled!");

	ID3D11Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
			spdlog::debug("FSR2FeatureDx11::Evaluate Bias mask exist..");
		else
			spdlog::debug("FSR2FeatureDx11::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		params.reactive = ffxGetResourceDX11(&_context, paramMask, (wchar_t*)L"FSR2_Reactive");
	}

	_hasColor = params.color.resource != nullptr;
	_hasDepth = params.depth.resource != nullptr;
	_hasMV = params.motionVectors.resource != nullptr;
	_hasExposure = params.exposure.resource != nullptr;
	_hasTM = params.reactive.resource != nullptr;
	_hasOutput = params.output.resource != nullptr;

	float MVScaleX = 1.0f;
	float MVScaleY = 1.0f;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		params.motionVectorScale.x = MVScaleX;
		params.motionVectorScale.y = MVScaleY;
	}
	else
	{
		spdlog::warn("FSR2FeatureDx11::Evaluate Can't get motion vector scales!");

		params.motionVectorScale.x = MVScaleX;
		params.motionVectorScale.y = MVScaleY;
	}

	if (Config::Instance()->OverrideSharpness.value_or(false))
	{
		params.enableSharpening = Config::Instance()->Sharpness.value_or(0.3) > 0.0f;
		params.sharpness = Config::Instance()->Sharpness.value_or(0.3);
	}
	else
	{
		float shapness = 0.0f;
		if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &shapness) == NVSDK_NGX_Result_Success)
		{
			_sharpness = shapness;

			params.enableSharpening = shapness > 0.0f;

			if (params.enableSharpening)
			{
				if (shapness > 1.0f)
					params.sharpness = 1.0f;
				else
					params.sharpness = shapness;
			}
		}
	}

	if (IsDepthInverted())
	{
		params.cameraFar = 0.0f;
		params.cameraNear = 1.0f;
	}
	else
	{
		params.cameraFar = 1.0f;
		params.cameraNear = 0.0f;
	}

	if (Config::Instance()->FsrVerticalFov.has_value())
		params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.01745329251f;
	else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
		params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.01745329251f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
	else
		params.cameraFovAngleVertical = 1.047198f;

	if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
		params.frameTimeDelta = (float)GetDeltaTime();

	params.preExposure = 1.0f;

	spdlog::debug("FSR2FeatureDx11::Evaluate Dispatch!!");
	auto result = ffxFsr2ContextDispatch(&_context, &params);

	if (result != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(result));
		return false;
	}

	// imgui
	if (Imgui)
	{
		if (Imgui->IsHandleDifferent())
			Imgui.reset();
		else
			Imgui->Render(InContext, paramOutput);
	}
	else
		Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);

	return true;
}

FSR2FeatureDx11::~FSR2FeatureDx11()
{
	spdlog::debug("FSR2FeatureDx11::~FSR2FeatureDx11");
}

