#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Dx11on12.h"

bool FSR2FeatureDx11on12::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12::Init!");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &_initFlags) == NVSDK_NGX_Result_Success)
		_initFlagsReady = true;

	_baseInit = false;

	return true;
}

bool FSR2FeatureDx11on12::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12::Evaluate");

	if (!_baseInit)
	{
		// to prevent creation dx12 device if we are going to recreate feature
		if (!Config::Instance()->DisplayResolution.has_value())
		{
			ID3D11Resource* paramVelocity = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

			if (paramVelocity != nullptr)
			{
				D3D11_TEXTURE2D_DESC desc;
				ID3D11Texture2D* pvTexture;
				paramVelocity->QueryInterface(IID_PPV_ARGS(&pvTexture));
				pvTexture->GetDesc(&desc);
				bool lowResMV = desc.Width < DisplayWidth();
				bool displaySizeEnabled = (InitFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0;

				if (displaySizeEnabled && lowResMV)
				{
					spdlog::warn("FSR2FeatureDx11on12::Evaluate MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, DisplayWidth(), displaySizeEnabled);
					Config::Instance()->DisplayResolution = false;
				}
			}
		}

		if (!Config::Instance()->AutoExposure.has_value())
		{
			ID3D11Resource* paramExpo = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExpo) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExpo);

			if (paramExpo == nullptr)
			{
				spdlog::warn("FSR2FeatureDx11on12::Evaluate ExposureTexture is not exist, enabling AutoExposure!!");
				Config::Instance()->AutoExposure = true;
			}
		}

		if (!Config::Instance()->DisableReactiveMask.has_value())
		{
			ID3D11Resource* paramRM = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramRM) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramRM);

			if (paramRM == nullptr)
			{
				spdlog::warn("FSR2FeatureDx11on12::Evaluate Bias mask not exist, enabling DisableReactiveMask!!");
				Config::Instance()->DisableReactiveMask = true;
			}
		}

		if (!BaseInit(Device, InDeviceContext, InParameters))
		{
			spdlog::debug("FSR2FeatureDx11on12::Evaluate BaseInit failed!");
			return false;
		}

		_baseInit = true;

		spdlog::debug("FSR2FeatureDx11on12::Evaluate calling InitFSR2");

		if (Dx12on11Device == nullptr)
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate Dx12on11Device is null!");
			return false;
		}

		if (!InitFSR2(InParameters))
		{
			spdlog::error("FSR2FeatureDx11on12::Evaluate InitFSR2 fail!");
			return false;
		}

		if (Imgui == nullptr || Imgui.get() == nullptr)
			Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);

		if (Config::Instance()->Dx11DelayedInit.value_or(false))
		{
			spdlog::trace("FSR2FeatureDx11on12::Evaluate sleeping after FSRContext creation for 1500ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		}

		OUT_DS = std::make_unique<DS_Dx12>("Output Downsample", Dx12on11Device);
	}

	if (!IsInited())
		return false;

	auto frame = _frameCount % 2;

	ID3D11DeviceContext4* dc;
	auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

	if (result != S_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	if (dc != Dx11DeviceContext)
	{
		spdlog::warn("FSR2FeatureDx11on12::Evaluate Dx11DeviceContext changed!");
		ReleaseSharedResources();
		Dx11DeviceContext = dc;
	}

	FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	bool useSS = Config::Instance()->SuperSamplingEnabled.value_or(false) &&
		!Config::Instance()->DisplayResolution.value_or(false) &&
		((float)DisplayWidth() / (float)params.renderSize.width) < Config::Instance()->SuperSamplingMultiplier.value_or(2.5f);

	spdlog::debug("FSR2FeatureDx11on12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = ffxGetCommandListDX12(Dx12CommandList);

	if (!ProcessDx11Textures(InParameters))
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate Can't process Dx11 textures!");

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}

	// AutoExposure or ReactiveMask is nullptr
	if (Config::Instance()->changeBackend)
	{
		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return true;
	}

	params.color = ffxGetResourceDX12(&_context, dx11Color.Dx12Resource, (wchar_t*)L"FSR2_Color", FFX_RESOURCE_STATE_COMPUTE_READ);
	params.motionVectors = ffxGetResourceDX12(&_context, dx11Mv.Dx12Resource, (wchar_t*)L"FSR2_Motion", FFX_RESOURCE_STATE_COMPUTE_READ);
	params.depth = ffxGetResourceDX12(&_context, dx11Depth.Dx12Resource, (wchar_t*)L"FSR2_Depth", FFX_RESOURCE_STATE_COMPUTE_READ);
	params.exposure = ffxGetResourceDX12(&_context, dx11Exp.Dx12Resource, (wchar_t*)L"FSR2_Exp", FFX_RESOURCE_STATE_COMPUTE_READ);
	params.reactive = ffxGetResourceDX12(&_context, dx11Tm.Dx12Resource, (wchar_t*)L"FSR2_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);

	if (useSS)
	{
		OUT_DS->Scale = (float)TargetWidth() / (float)DisplayWidth();

		if (OUT_DS->CreateBufferResource(Dx12on11Device, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			OUT_DS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			params.output = ffxGetResourceDX12(&_context, OUT_DS->Buffer(), (wchar_t*)L"FSR2_Out", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		else
			params.output = ffxGetResourceDX12(&_context, dx11Out.Dx12Resource, (wchar_t*)L"FSR2_Out", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
		params.output = ffxGetResourceDX12(&_context, dx11Out.Dx12Resource, (wchar_t*)L"FSR2_Out", FFX_RESOURCE_STATE_UNORDERED_ACCESS);

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
		spdlog::warn("FSR2FeatureDx11on12::Evaluate Can't get motion vector scales!");

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
		params.cameraFovAngleVertical = Config::Instance()->FsrVerticalFov.value() * 0.0174532925199433f;
	else if (Config::Instance()->FsrHorizontalFov.value_or(0.0f) > 0.0f)
		params.cameraFovAngleVertical = 2.0f * atan((tan(Config::Instance()->FsrHorizontalFov.value() * 0.0174532925199433f) * 0.5f) / (float)DisplayHeight() * (float)DisplayWidth());
	else
		params.cameraFovAngleVertical = 1.0471975511966f;

	if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
		params.frameTimeDelta = (float)GetDeltaTime();

	params.preExposure = 1.0f;

	spdlog::debug("FSR2FeatureDx11on12::Evaluate Dispatch!!");
	auto ffxresult = ffxFsr2ContextDispatch(&_context, &params);

	if (ffxresult != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(ffxresult));

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}

	if (useSS)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate downscaling output...");
		OUT_DS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OUT_DS->Dispatch(Dx12on11Device, Dx12CommandList, OUT_DS->Buffer(), dx11Out.Dx12Resource, 16, 16))
		{
			Config::Instance()->SuperSamplingEnabled = false;
			Config::Instance()->changeBackend = true;

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return true;
		}
	}

	// fsr done
	if (!CopyBackOutput())
	{
		spdlog::error("FSR2FeatureDx11on12::Evaluate Can't copy output texture back!");

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}

	// imgui
	if (_frameCount > 20)
	{
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
	}

	// Execute dx12 commands to process fsr
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	_frameCount++;

	if (_frameCount == 200 && !Config::Instance()->UseSafeSyncQueries.has_value())
		Config::Instance()->UseSafeSyncQueries = 1;

	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

FSR2FeatureDx11on12::~FSR2FeatureDx11on12()
{
	spdlog::debug("FSR2FeatureDx11on12::~FSR2FeatureDx11on12");
}

bool FSR2FeatureDx11on12::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx11on12::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx11on12::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeDX12();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = ffxFsr2GetInterfaceDX12(&_contextDesc.callbacks, Dx12on11Device, scratchBuffer, scratchBufferSize);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = ffxGetDeviceDX12(Dx12on11Device);
	_contextDesc.flags = 0;

	int featureFlags = 0;
	if (!_initFlagsReady)
	{
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);
		_initFlags = featureFlags;
		_initFlagsReady = true;
	}
	else
		featureFlags = _initFlags;

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
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		_contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		spdlog::info("FSR2FeatureDx11on12::InitFSR2 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->SuperSamplingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
	{
		float ssMulti = Config::Instance()->SuperSamplingMultiplier.value_or(2.5f);
		float ssMultilimit = 5.0f;

		if (ssMulti < 0.0f || ssMulti > ssMultilimit)
		{
			ssMulti = ssMultilimit;
			Config::Instance()->SuperSamplingMultiplier = ssMulti;
		}

		_targetWidth = RenderWidth() * ssMulti;
		_targetHeight = RenderHeight() * ssMulti;

		if (_targetWidth <= DisplayWidth() || _targetHeight <= DisplayHeight())
		{
			Config::Instance()->SuperSamplingEnabled = false;
			_targetWidth = DisplayWidth();
			_targetHeight = DisplayHeight();
		}
	}
	else
	{
		_targetWidth = DisplayWidth();
		_targetHeight = DisplayHeight();
	}

	_contextDesc.maxRenderSize.width = TargetWidth();
	_contextDesc.maxRenderSize.height = TargetHeight();
	_contextDesc.displaySize.width = TargetWidth();
	_contextDesc.displaySize.height = TargetHeight();

#if _DEBUG
	_contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
	_contextDesc.fpMessage = FfxLogCallback;
#endif

	spdlog::debug("FSR2FeatureDx11on12::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = ffxFsr2ContextCreate(&_context, &_contextDesc);

	if (ret != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx11on12::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString(ret));
		return false;
	}

	SetInit(true);

	return true;
}