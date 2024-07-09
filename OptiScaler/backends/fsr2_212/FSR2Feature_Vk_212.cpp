#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "FSR2Feature_Vk_212.h"

#include "nvsdk_ngx_vk.h"

bool FSR2FeatureVk212::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk212::InitFSR2");

	if (IsInited())
		return true;

	if (PhysicalDevice == nullptr)
	{
		spdlog::error("FSR2FeatureVk212::InitFSR2 PhysicalDevice is null!");
		return false;
	}

	auto scratchBufferSize = Fsr212::ffxFsr2GetScratchMemorySizeVK212(PhysicalDevice);
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = Fsr212::ffxFsr2GetInterfaceVK212(&_contextDesc.callbacks, scratchBuffer, scratchBufferSize, PhysicalDevice, vkGetDeviceProcAddr);

	if (errorCode != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureVk212::InitFSR2 ffxGetInterfaceVK error: {0}", ResultToString212(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.device = Fsr212::ffxGetDeviceVK212(Device);
	_contextDesc.maxRenderSize.width = DisplayWidth();
	_contextDesc.maxRenderSize.height = DisplayHeight();
	_contextDesc.displaySize.width = DisplayWidth();
	_contextDesc.displaySize.height = DisplayHeight();

	_contextDesc.flags = 0;

	unsigned int featureFlags;
	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

	bool Hdr = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_IsHDR;
	bool EnableSharpening = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DoSharpening;
	bool DepthInverted = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_DepthInverted;
	bool JitterMotion = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVJittered;
	bool LowRes = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes;
	bool AutoExposure = featureFlags & NVSDK_NGX_DLSS_Feature_Flags_AutoExposure;

	_initFlags = featureFlags;

	if (Config::Instance()->DepthInverted.value_or(DepthInverted))
	{
		Config::Instance()->DepthInverted = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DEPTH_INVERTED;
		SetDepthInverted(true);
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->DepthInverted = false;

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = false;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}
	else
		Config::Instance()->JitterCancellation = false;

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		_contextDesc.flags |= Fsr212::FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		spdlog::info("FSR2FeatureVk212::InitFSR2 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	spdlog::debug("FSR2FeatureVk212::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = Fsr212::ffxFsr2ContextCreate212(&_context, &_contextDesc);

	if (ret != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureVk212::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString212(ret));
		return false;
	}

	SetInit(true);

	return true;
	}

bool FSR2FeatureVk212::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk212::Init");

	if (IsInited())
		return true;

	Instance = InInstance;
	PhysicalDevice = InPD;
	Device = InDevice;
	GIPA = InGIPA;
	GDPA = InGDPA;

	return InitFSR2(InParameters);
}

bool FSR2FeatureVk212::Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk212::Evaluate");

	if (!IsInited())
		return false;

	Fsr212::FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureVk212::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = Fsr212::ffxGetCommandListVK212(InCmdBuffer);

	void* paramColor;
	InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureVk212::Evaluate Color exist..");

		params.color = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Image,
			((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.ImageView,
			((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Height,
			((NVSDK_NGX_Resource_VK*)paramColor)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Color", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureVk212::Evaluate Color not exist!!");
		return false;
	}

	void* paramVelocity;
	InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR2FeatureVk212::Evaluate MotionVectors exist..");

		params.motionVectors = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Image,
			((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.ImageView,
			((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Height,
			((NVSDK_NGX_Resource_VK*)paramVelocity)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_MotionVectors", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureVk212::Evaluate MotionVectors not exist!!");
		return false;
	}

	void* paramOutput;
	InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR2FeatureVk212::Evaluate Output exist..");

		params.output = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Image,
			((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.ImageView,
			((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Height,
			((NVSDK_NGX_Resource_VK*)paramOutput)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Output", Fsr212::FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR2FeatureVk212::Evaluate Output not exist!!");
		return false;
	}

	void* paramDepth;
	InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureVk212::Evaluate Depth exist..");

		params.depth = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Image,
			((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.ImageView,
			((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Height,
			((NVSDK_NGX_Resource_VK*)paramDepth)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Depth", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR2FeatureVk212::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR2FeatureVk212::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	void* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp);

		if (paramExp)
		{
			spdlog::debug("FSR2FeatureVk212::Evaluate ExposureTexture exist..");

			params.exposure = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Image,
				((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.ImageView,
				((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Height,
				((NVSDK_NGX_Resource_VK*)paramExp)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Exposure", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
		}
		else
		{
			spdlog::debug("FSR2FeatureVk212::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
			Config::Instance()->AutoExposure = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}
	else
		spdlog::debug("FSR2FeatureVk212::Evaluate AutoExposure enabled!");

	void* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask);

		if (paramMask)
		{
			spdlog::debug("FSR2FeatureVk212::Evaluate Bias mask exist..");

			params.reactive = Fsr212::ffxGetTextureResourceVK212(&_context, ((NVSDK_NGX_Resource_VK*)paramMask)->Resource.ImageViewInfo.Image,
				((NVSDK_NGX_Resource_VK*)paramMask)->Resource.ImageViewInfo.ImageView,
				((NVSDK_NGX_Resource_VK*)paramMask)->Resource.ImageViewInfo.Width, ((NVSDK_NGX_Resource_VK*)paramMask)->Resource.ImageViewInfo.Height,
				((NVSDK_NGX_Resource_VK*)paramMask)->Resource.ImageViewInfo.Format, (wchar_t*)L"FSR2_Reactive", Fsr212::FFX_RESOURCE_STATE_COMPUTE_READ);
		}
		else
		{
			spdlog::debug("FSR2FeatureVk212::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");
			Config::Instance()->DisableReactiveMask = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
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
		spdlog::warn("FSR2FeatureVk212::Evaluate Can't get motion vector scales!");

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
	InParameters->Get(NVSDK_NGX_Parameter_DLSS_Pre_Exposure, &params.preExposure);


	spdlog::debug("FSR2FeatureVk212::Evaluate Dispatch!!");
	auto result = Fsr212::ffxFsr2ContextDispatch212(&_context, &params);

	if (result != Fsr212::FFX_OK)
	{
		spdlog::error("FSR2FeatureVk212::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString212(result));
		return false;
	}

	_frameCount++;

	// restore resource states
	//if (paramColor && Config::Instance()->ColorResourceBarrier.value_or(false))
	//	ResourceBarrier(InCommandList, paramColor,
	//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	//if (paramVelocity && Config::Instance()->MVResourceBarrier.has_value())
	//	ResourceBarrier(InCommandList, paramVelocity,
	//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	//if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
	//	ResourceBarrier(InCommandList, paramOutput,
	//		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	//if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
	//	ResourceBarrier(InCommandList, paramDepth,
	//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	//if (paramExp && Config::Instance()->ExposureResourceBarrier.has_value())
	//	ResourceBarrier(InCommandList, paramExp,
	//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	//if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
	//	ResourceBarrier(InCommandList, paramMask,
	//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	//		(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

	return true;
}
