#include "pch.h"
#include "FSR2Feature_Dx12.h"
#include "Config.h"

bool FSR2FeatureDx12::Init(ID3D12Device* device, const NVSDK_NGX_Parameter* InParameters)
{
	if (IsInited())
		return true;

	Device = device;

	return InitFSR2(device, InParameters);
}

bool FSR2FeatureDx12::Evaluate(ID3D12GraphicsCommandList* commandList, const NVSDK_NGX_Parameter* InParameters)
{
	if (!IsInited())
		return false;

	FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = reset == 1;

	FSR2Feature::SetRenderResolution(InParameters, &params);

	spdlog::debug("FSR3FeatureDx12::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = ffxGetCommandListDX12(commandList);

	ID3D12Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR3FeatureDx12::Evaluate Color exist..");

		if (Config::Instance()->ColorResourceBarrier.has_value())
			ResourceBarrier(commandList, paramColor,
				(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.color = ffxGetResourceDX12(paramColor, GetFfxResourceDescriptionDX12(paramColor), (wchar_t*)L"FSR3Upscale_Color", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR3FeatureDx12::Evaluate Color not exist!!");
		return false;
	}

	ID3D12Resource* paramVelocity;
	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR3FeatureDx12::Evaluate MotionVectors exist..");

		if (Config::Instance()->MVResourceBarrier.has_value())
			ResourceBarrier(commandList, paramVelocity,
				(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.motionVectors = ffxGetResourceDX12(paramVelocity, GetFfxResourceDescriptionDX12(paramVelocity), (wchar_t*)L"FSR3Upscale_MotionVectors", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR3FeatureDx12::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D12Resource* paramOutput;
	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR3FeatureDx12::Evaluate Output exist..");

		if (Config::Instance()->OutputResourceBarrier.has_value())
			ResourceBarrier(commandList, paramOutput,
				(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		params.output = ffxGetResourceDX12(paramOutput, GetFfxResourceDescriptionDX12(paramOutput), (wchar_t*)L"FSR3Upscale_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR3FeatureDx12::Evaluate Output not exist!!");
		return false;
	}

	ID3D12Resource* paramDepth;
	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR3FeatureDx12::Evaluate Depth exist..");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(commandList, paramDepth,
				(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.depth = ffxGetResourceDX12(paramDepth, GetFfxResourceDescriptionDX12(paramDepth), (wchar_t*)L"FSR3Upscale_Depth", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR3FeatureDx12::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR3FeatureDx12::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	ID3D12Resource* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExp);

		if (paramExp)
			spdlog::debug("FSR3FeatureDx12::Evaluate ExposureTexture exist..");
		else
			spdlog::debug("FSR3FeatureDx12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");

		if (Config::Instance()->ExposureResourceBarrier.has_value())
			ResourceBarrier(commandList, paramExp,
				(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.exposure = ffxGetResourceDX12(paramExp, GetFfxResourceDescriptionDX12(paramExp), (wchar_t*)L"FSR3Upscale_Exposure", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
		spdlog::debug("FSR3FeatureDx12::Evaluate AutoExposure enabled!");

	ID3D12Resource* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramMask);

		if (paramMask)
			spdlog::debug("FSR3FeatureDx12::Evaluate Bias mask exist..");
		else
			spdlog::debug("FSR3FeatureDx12::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		if (Config::Instance()->MaskResourceBarrier.has_value())
			ResourceBarrier(commandList, paramMask,
				(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(),
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.reactive = ffxGetResourceDX12(paramMask, GetFfxResourceDescriptionDX12(paramMask), (wchar_t*)L"FSR3Upscale_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);
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
		spdlog::warn("FSR3FeatureDx12::Evaluate Can't get motion vector scales!");


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

	spdlog::debug("FSR3FeatureDx12::Evaluate Dispatch!!");
	auto result = ffxFsr2ContextDispatch(&_context, &params);

	if (result != FFX_OK)
	{
		spdlog::error("FSR3FeatureDx12::Evaluate ffxFsr2ContextDispatch error: {0:x}", result);
		return false;
	}

	// restore resource states
	if (paramColor && Config::Instance()->ColorResourceBarrier.value_or(false))
		ResourceBarrier(commandList, paramColor,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	if (paramVelocity && Config::Instance()->MVResourceBarrier.has_value())
		ResourceBarrier(commandList, paramVelocity,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
		ResourceBarrier(commandList, paramOutput,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
		ResourceBarrier(commandList, paramDepth,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	if (paramExp && Config::Instance()->ExposureResourceBarrier.has_value())
		ResourceBarrier(commandList, paramExp,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
		ResourceBarrier(commandList, paramMask,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

	return true;
}

void FSR2FeatureDx12::ReInit(const NVSDK_NGX_Parameter* InParameters)
{
	SetInit(false);

	if (IsInited())
	{
		auto errorCode = ffxFsr2ContextDestroy(&_context);

		if (errorCode != FFX_OK)
			spdlog::error("FSR3Feature::~FSR3Feature ffxFsr2ContextDestroy error: {0:x}", errorCode);

		free(_contextDesc.backendInterface.scratchBuffer);
	}

	SetInit(Init(Device, InParameters));
}

void FSR2FeatureDx12::SetInit(bool value)
{
	FSR2Feature::SetInit(value);
}

bool FSR2FeatureDx12::IsInited()
{
	return FSR2Feature::IsInited();
}
