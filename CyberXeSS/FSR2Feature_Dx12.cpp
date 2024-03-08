#include "pch.h"
#include "FSR2Feature_Dx12.h"
#include "Config.h"

bool FSR2FeatureDx12::Init(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx12::Init");

	if (IsInited())
		return true;

	Device = InDevice;

	return InitFSR2(InParameters);
}

bool FSR2FeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureDx12::Evaluate");

	if (!IsInited())
		return false;

	FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureDx12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = ffxGetCommandListDX12(InCommandList);

	ID3D12Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureDx12::Evaluate Color exist..");

		if (Config::Instance()->ColorResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramColor,
				(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.color = ffxGetResourceDX12(&_context, paramColor, (wchar_t*)L"FSR3Upscale_Color", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12::Evaluate Color not exist!!");
		return false;
	}

	ID3D12Resource* paramVelocity;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR2FeatureDx12::Evaluate MotionVectors exist..");

		if (Config::Instance()->MVResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramVelocity,
				(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.motionVectors = ffxGetResourceDX12(&_context, paramVelocity, (wchar_t*)L"FSR3Upscale_MotionVectors", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D12Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR2FeatureDx12::Evaluate Output exist..");

		if (Config::Instance()->OutputResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramOutput,
				(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		params.output = ffxGetResourceDX12(&_context, paramOutput, (wchar_t*)L"FSR3Upscale_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR2FeatureDx12::Evaluate Output not exist!!");
		return false;
	}

	ID3D12Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureDx12::Evaluate Depth exist..");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramDepth,
				(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.depth = ffxGetResourceDX12(&_context, paramDepth, (wchar_t*)L"FSR3Upscale_Depth", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR2FeatureDx12::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR2FeatureDx12::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	ID3D12Resource* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

		if (paramExp)
			spdlog::debug("FSR2FeatureDx12::Evaluate ExposureTexture exist..");
		else
			spdlog::debug("FSR2FeatureDx12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");

		if (Config::Instance()->ExposureResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramExp,
				(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.exposure = ffxGetResourceDX12(&_context, paramExp, (wchar_t*)L"FSR3Upscale_Exposure", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
		spdlog::debug("FSR2FeatureDx12::Evaluate AutoExposure enabled!");

	ID3D12Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
			spdlog::debug("FSR2FeatureDx12::Evaluate Bias mask exist..");
		else
			spdlog::debug("FSR2FeatureDx12::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		if (Config::Instance()->MaskResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramMask,
				(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.reactive = ffxGetResourceDX12(&_context, paramMask, (wchar_t*)L"FSR3Upscale_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);
	}

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		params.motionVectorScale.x = MVScaleX;
		params.motionVectorScale.y = MVScaleY;
	}
	else
		spdlog::warn("FSR2FeatureDx12::Evaluate Can't get motion vector scales!");


	float shapness;
	if (InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &shapness) == NVSDK_NGX_Result_Success)
	{
		params.enableSharpening = shapness != 0 && shapness != 1;

		if (params.enableSharpening)
		{
			if (shapness < 0)
				params.sharpness = (shapness + 1.0f) / 2.0f;
			else
				params.sharpness = shapness;
		}
	}

	if (IsDepthInverted())
	{
		params.cameraFar = 0.0f;
		params.cameraNear = FLT_MAX;
	}
	else
	{
		params.cameraNear = 0.0f;
		params.cameraFar = FLT_MAX;
	}

	params.cameraFovAngleVertical = 1.047198f;

	params.frameTimeDelta = (float)GetDeltaTime();
	params.preExposure = 1.0f;

	spdlog::debug("FSR2FeatureDx12::Evaluate Dispatch!!");
	auto result = ffxFsr2ContextDispatch(&_context, &params);

	if (result != FFX_OK)
	{
		spdlog::error("FSR2FeatureDx12::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(result));
		return false;
	}

	// restore resource states
	if (paramColor && Config::Instance()->ColorResourceBarrier.value_or(false))
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

	return true;
}

bool FSR2FeatureDx12::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2Feature::InitFSR2");

	if (IsInited())
		return true;

	if (Device == nullptr)
	{
		spdlog::error("FSR2Feature::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = ffxFsr2GetScratchMemorySizeDX12();
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = ffxFsr2GetInterfaceDX12(&_contextDesc.callbacks, Device, scratchBuffer, scratchBufferSize);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2Feature::InitFSR2 ffxGetInterfaceDX12 error: {0}", ResultToString(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = ffxGetDeviceDX12(Device);
	_contextDesc.maxRenderSize.width = RenderWidth();
	_contextDesc.maxRenderSize.height = RenderHeight();
	_contextDesc.displaySize.width = DisplayWidth();
	_contextDesc.displaySize.height = DisplayHeight();

	_contextDesc.flags = 0;

	int featureFlags;
	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

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
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = false;
		_contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2Feature::InitFSR2 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	_contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INFINITE;

#if _DEBUG
	_contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
	_contextDesc.fpMessage = FfxLogCallback;
#endif

	spdlog::debug("FSR2Feature::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = ffxFsr2ContextCreate(&_context, &_contextDesc);

	if (ret != FFX_OK)
	{
		spdlog::error("FSR2Feature::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString(ret));
		return false;
	}

	SetInit(true);

	return true;
}