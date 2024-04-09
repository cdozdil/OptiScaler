#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Dx12_212.h"

bool FSR2FeatureDx12_212::Init(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx12_212::Init");

	if (IsInited())
		return true;

	Device = InDevice;

	if (InitFSR2(InParameters))
	{
		if (Imgui == nullptr || Imgui.get() == nullptr)
			Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), Device);

		OUT_DS = std::make_unique<DS_Dx12>("Output Downsample", InDevice);

		return true;
	}

	return false;
}

bool FSR2FeatureDx12_212::Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx12_212::Evaluate");

	if (!IsInited())
		return false;

	Fsr212::FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	spdlog::debug("FSR2FeatureDx12_212::Evaluate Jitter Offset: {0}x{1}", params.jitterOffset.x, params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	bool useSS = Config::Instance()->SuperSamplingEnabled.value_or(false) &&
		!Config::Instance()->DisplayResolution.value_or(false) &&
		((float)DisplayWidth() / (float)params.renderSize.width) < Config::Instance()->SuperSamplingMultiplier.value_or(3.0f);

	spdlog::debug("FSR2FeatureDx12_212::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = Fsr212::ffxGetCommandListDX12_212(InCommandList);

	ID3D12Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureDx12_212::Evaluate Color exist..");

		if (Config::Instance()->ColorResourceBarrier.has_value())
		{
			ResourceBarrier(InCommandList, paramColor,
				(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
		{
			Config::Instance()->ColorResourceBarrier = (int)D3D12_RESOURCE_STATE_RENDER_TARGET;

			ResourceBarrier(InCommandList, paramColor,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		params.color = Fsr212::ffxGetResourceDX12_212(&_context, paramColor, (wchar_t*)L"FSR2_Color", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12_212::Evaluate Color not exist!!");
		return false;
	}

	ID3D12Resource* paramVelocity;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR2FeatureDx12_212::Evaluate MotionVectors exist..");

		if (Config::Instance()->MVResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramVelocity,
				(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.motionVectors = Fsr212::ffxGetResourceDX12_212(&_context, paramVelocity, (wchar_t*)L"FSR2_MotionVectors", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12_212::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D12Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR2FeatureDx12_212::Evaluate Output exist..");

		if (Config::Instance()->OutputResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramOutput,
				(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (useSS)
		{
			OUT_DS->Scale = (float)TargetWidth() / (float)DisplayWidth();

			if (OUT_DS->CreateBufferResource(Device, paramOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				OUT_DS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				params.output = Fsr212::ffxGetResourceDX12_212(&_context, OUT_DS->Buffer(), (wchar_t*)L"FSR2_Output", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
			}
			else
				params.output = Fsr212::ffxGetResourceDX12_212(&_context, paramOutput, (wchar_t*)L"FSR2_Output", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
		}
		else
			params.output = Fsr212::ffxGetResourceDX12_212(&_context, paramOutput, (wchar_t*)L"FSR2_Output", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12_212::Evaluate Output not exist!!");
		return false;
	}

	ID3D12Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureDx12_212::Evaluate Depth exist..");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramDepth,
				(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.depth = Fsr212::ffxGetResourceDX12_212(&_context, paramDepth, (wchar_t*)L"FSR2_Depth", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR2FeatureDx12_212::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR2FeatureDx12_212::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	ID3D12Resource* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

		if (paramExp)
			spdlog::debug("FSR2FeatureDx12_212::Evaluate ExposureTexture exist..");
		else
		{
			spdlog::debug("FSR2FeatureDx12_212::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
			Config::Instance()->AutoExposure = true;
			Config::Instance()->changeBackend = true;
			return true;
		}

		if (Config::Instance()->ExposureResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramExp,
				(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.exposure = Fsr212::ffxGetResourceDX12_212(&_context, paramExp, (wchar_t*)L"FSR2_Exposure", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
		spdlog::debug("FSR2FeatureDx12_212::Evaluate AutoExposure enabled!");

	ID3D12Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
			spdlog::debug("FSR2FeatureDx12_212::Evaluate Bias mask exist..");
		else
			spdlog::debug("FSR2FeatureDx12_212::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		if (Config::Instance()->MaskResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramMask,
				(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.reactive = Fsr212::ffxGetResourceDX12_212(&_context, paramMask, (wchar_t*)L"FSR2_Reactive", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
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
		spdlog::warn("FSR2FeatureDx12_212::Evaluate Can't get motion vector scales!");

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

	spdlog::debug("FSR2FeatureDx12_212::Evaluate Sharpness: {0}", params.sharpness);

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

	spdlog::debug("FSR2FeatureDx12_212::Evaluate FsrVerticalFov: {0}", params.cameraFovAngleVertical);

	if (InParameters->Get(NVSDK_NGX_Parameter_FrameTimeDeltaInMsec, &params.frameTimeDelta) != NVSDK_NGX_Result_Success || params.frameTimeDelta < 1.0f)
		params.frameTimeDelta = (float)GetDeltaTime();

	spdlog::debug("FSR2FeatureDx12_212::Evaluate FrameTimeDeltaInMsec: {0}", params.frameTimeDelta);

	params.preExposure = 1.0f;

	spdlog::debug("FSR2FeatureDx12_212::Evaluate Dispatch!!");
	auto result = Fsr212::ffxFsr2ContextDispatch212(&_context, &params);

	if (result != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx12_212::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString212(result));
		return false;
	}

	if (useSS)
	{
		spdlog::debug("FSR2FeatureDx12_212::Evaluate downscaling output...");
		OUT_DS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OUT_DS->Dispatch(Device, InCommandList, OUT_DS->Buffer(), paramOutput, 16, 16))
		{
			Config::Instance()->SuperSamplingEnabled = false;
			Config::Instance()->changeBackend = true;
			return true;
		}
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
				Imgui->Render(InCommandList, paramOutput);
		}
		else
		{
			if (Imgui == nullptr || Imgui.get() == nullptr)
				Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), Device);
		}
	}

	// restore resource states
	if (paramColor && Config::Instance()->ColorResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramColor,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	if (paramVelocity && Config::Instance()->MVResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramVelocity,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramOutput,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramDepth,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	if (paramExp && Config::Instance()->ExposureResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramExp,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramMask,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

	_frameCount++;

	return true;
}

FSR2FeatureDx12_212::~FSR2FeatureDx12_212()
{
}

bool FSR2FeatureDx12_212::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx12_212::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2FeatureDx12_212::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = Fsr212::ffxFsr2GetScratchMemorySizeDX12_212();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = Fsr212::ffxFsr2GetInterfaceDX12_212(&_contextDesc.callbacks, Device, scratchBuffer, scratchBufferSize);

	if (errorCode != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx12_212::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString212(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = Fsr212::ffxGetDeviceDX12_212(Device);

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
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 xessParams.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 xessParams.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 xessParams.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 xessParams.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2FeatureDx12_212::InitFSR2 xessParams.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->SuperSamplingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false))
	{
		_targetWidth = RenderWidth() * Config::Instance()->SuperSamplingMultiplier.value_or(3.0f);
		_targetHeight = RenderHeight() * Config::Instance()->SuperSamplingMultiplier.value_or(3.0f);
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

	spdlog::debug("FSR2FeatureDx12_212::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = Fsr212::ffxFsr2ContextCreate212(&_context, &_contextDesc);

	if (ret != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureDx12_212::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString212(ret));
		return false;
	}

	SetInit(true);

	return true;
}