#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Dx11on12_212.h"

bool FSR2FeatureDx11on12_212::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12_212::Init!");

	if (IsInited())
		return true;

	if (!BaseInit(InDevice, InContext, InParameters))
	{
		spdlog::debug("FSR2FeatureDx11on12_212::Init BaseInit failed!");
		return false;
	}

	spdlog::debug("FSR2FeatureDx11on12_212::Init calling InitFSR2");

	if (Dx12on11Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx11on12_212::Init Dx12on11Device is null!");
		return false;
	}

	if (!InitFSR2(InParameters))
	{
		spdlog::error("FSR2FeatureDx11on12_212::Init InitFSR2 fail!");
		return false;
	}

	if (Imgui == nullptr || Imgui.get() == nullptr)
		Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);

	return true;
}

bool FSR2FeatureDx11on12_212::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12_212::Evaluate");

	if (!IsInited())
		return false;

	ID3D11DeviceContext4* dc;
	auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	if (dc != Dx11DeviceContext)
	{
		spdlog::warn("FSR2FeatureDx11on12_212::Evaluate Dx11DeviceContext changed!");
		ReleaseSharedResources();
		Dx11DeviceContext = dc;
	}

	Fsr212::FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureDx11on12_212::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = Fsr212::ffxGetCommandListDX12_212(Dx12CommandList);

	if (!ProcessDx11Textures(InParameters))
	{
		spdlog::error("FSR2FeatureDx11on12_212::Evaluate Can't process Dx11 textures!");
		return false;
	}

	params.color = Fsr212::ffxGetResourceDX12_212(&_context, dx11Color.Dx12Resource, (wchar_t*)L"FSR2_Color", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	params.motionVectors = Fsr212::ffxGetResourceDX12_212(&_context, dx11Mv.Dx12Resource, (wchar_t*)L"FSR2_Motion", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	params.output = Fsr212::ffxGetResourceDX12_212(&_context, dx11Out.Dx12Resource, (wchar_t*)L"FSR2_Out", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	params.depth = Fsr212::ffxGetResourceDX12_212(&_context, dx11Depth.Dx12Resource, (wchar_t*)L"FSR2_Depth", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	params.exposure = Fsr212::ffxGetResourceDX12_212(&_context, dx11Exp.Dx12Resource, (wchar_t*)L"FSR2_Exp", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	params.reactive = Fsr212::ffxGetResourceDX12_212(&_context, dx11Tm.Dx12Resource, (wchar_t*)L"FSR2_Reactive", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);

	_hasColor = params.color.resource != nullptr;
	_hasDepth = params.depth.resource != nullptr;
	_hasMV = params.motionVectors.resource != nullptr;
	_hasExposure = params.exposure.resource != nullptr;
	_hasTM = params.reactive.resource != nullptr;
	_hasOutput = params.output.resource != nullptr;

#pragma endregion

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
		spdlog::warn("FSR2FeatureDx11on12_212::Evaluate Can't get motion vector scales!");

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

	spdlog::debug("FSR2FeatureDx11on12_212::Evaluate Dispatch!!");
	auto ffxresult = Fsr212::ffxFsr2ContextDispatch212(&_context, &params);

	if (ffxresult != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString212(ffxresult));
		return false;
	}

	// Execute dx12 commands to process fsr
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	// fsr done
	if (!CopyBackOutput())
	{
		spdlog::error("FSR2FeatureDx11on12_212::Evaluate Can't copy output texture back!");
		return false;
	}

	// imgui
	if (Imgui != nullptr && Imgui.get() != nullptr)
	{
		if (Imgui->IsHandleDifferent())
		{
			Imgui.reset();
		}
		else
			Imgui->Render(InDeviceContext, paramOutput);
	}
	else
	{
		if (Imgui == nullptr || Imgui.get() == nullptr)
			Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
	}

	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

FSR2FeatureDx11on12_212::~FSR2FeatureDx11on12_212()
{
	spdlog::debug("FSR2FeatureDx11on12_212::~FSR2FeatureDx11on12_212");
}

bool FSR2FeatureDx11on12_212::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12_212::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = Fsr212::ffxFsr2GetScratchMemorySizeDX12_212();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = Fsr212::ffxFsr2GetInterfaceDX12_212(&_contextDesc.callbacks, Dx12on11Device, scratchBuffer, scratchBufferSize);

	if (errorCode != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString212(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = Fsr212::ffxGetDeviceDX12_212(Dx12on11Device);
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
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DEPTH_INVERTED;
		SetDepthInverted(true);
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2FeatureDx11on12_212::InitFSR2 xessParams.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	spdlog::debug("FSR2FeatureDx11on12_212::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = Fsr212::ffxFsr2ContextCreate212(&_context, &_contextDesc);

	if (ret != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12_212::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString212(ret));
		return false;
	}

	SetInit(true);

	return true;
}