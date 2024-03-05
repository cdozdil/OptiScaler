#include "pch.h"
#include "FSR2Feature_Vk.h"
#include "Config.h"

#include <vulkan/vulkan.hpp>
#include "nvsdk_ngx_vk.h"

inline FfxSurfaceFormat GetFfxSurfaceFormat(VkFormat format)
{
	switch (format)
	{
	case (VK_FORMAT_R32G32B32A32_SINT):
		return FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
	case (VK_FORMAT_R32G32B32A32_SFLOAT):
		return FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT;
	case (VK_FORMAT_R16G16B16A16_SFLOAT):
		return FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
	case (VK_FORMAT_R32G32_SFLOAT):
		return FFX_SURFACE_FORMAT_R32G32_FLOAT;
	case (VK_FORMAT_R8_UINT):
		return FFX_SURFACE_FORMAT_R8_UINT;
	case (VK_FORMAT_R32_UINT):
		return FFX_SURFACE_FORMAT_R32_UINT;
	case (VK_FORMAT_R8G8B8A8_UINT):
	case (VK_FORMAT_R8G8B8A8_SINT):
		return FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
	case (VK_FORMAT_R8G8B8A8_UNORM):
		return FFX_SURFACE_FORMAT_R8G8B8A8_UNORM;
	case (VK_FORMAT_R8G8B8A8_SRGB):
		return FFX_SURFACE_FORMAT_R8G8B8A8_SRGB;
	case (VK_FORMAT_B10G11R11_UFLOAT_PACK32):
		return FFX_SURFACE_FORMAT_R11G11B10_FLOAT;
	case (VK_FORMAT_R16G16_SFLOAT):
		return FFX_SURFACE_FORMAT_R16G16_FLOAT;
	case (VK_FORMAT_R16G16_UINT):
		return FFX_SURFACE_FORMAT_R16G16_UINT;
	//case (VK_FORMAT_R16G16_SINT):
	//	return FFX_SURFACE_FORMAT_R16G16_SINT;
	case (VK_FORMAT_R16_SFLOAT):
		return FFX_SURFACE_FORMAT_R16_FLOAT;
	case (VK_FORMAT_R16_UINT):
		return FFX_SURFACE_FORMAT_R16_UINT;
	case (VK_FORMAT_R16_UNORM):
		return FFX_SURFACE_FORMAT_R16_UNORM;
	case (VK_FORMAT_R16_SNORM):
		return FFX_SURFACE_FORMAT_R16_SNORM;
	case (VK_FORMAT_R8_UNORM):
		return FFX_SURFACE_FORMAT_R8_UNORM;
	case (VK_FORMAT_R8G8_UNORM):
		return FFX_SURFACE_FORMAT_R8G8_UNORM;
	case (VK_FORMAT_R32_SFLOAT):
	case (VK_FORMAT_D32_SFLOAT):
		return FFX_SURFACE_FORMAT_R32_FLOAT;
	case (VK_FORMAT_R32G32B32A32_UINT):
		return FFX_SURFACE_FORMAT_R32G32B32A32_UINT;
	//case (VK_FORMAT_A2R10G10B10_UNORM_PACK32):
	//	return FFX_SURFACE_FORMAT_R10G10B10A2_UNORM;

	default:
		FFX_ASSERT_MESSAGE(false, "ValidationRemap: Unsupported format requested. Please implement.");
		return FFX_SURFACE_FORMAT_UNKNOWN;
	}
}

inline FfxResourceDescription GetFfxResourceDescriptionVk(const void* pResource, FfxResourceUsage usage)
{
	FfxResourceDescription resourceDescription = { };

	// This is valid
	if (!pResource)
		return resourceDescription;

	auto vkResource = (NVSDK_NGX_Resource_VK*)pResource;

	// Set flags properly for resource registration
	resourceDescription.flags = FFX_RESOURCE_FLAGS_NONE;
	resourceDescription.usage = usage;
	resourceDescription.width = vkResource->Resource.ImageViewInfo.Width;
	resourceDescription.height = vkResource->Resource.ImageViewInfo.Height;
	resourceDescription.depth = 1;
	resourceDescription.mipCount = 1;
	resourceDescription.format = GetFfxSurfaceFormat(vkResource->Resource.ImageViewInfo.Format);
	resourceDescription.type = FFX_RESOURCE_TYPE_TEXTURE2D;

	return resourceDescription;
}

bool FSR2FeatureVk::InitFSR2(const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk::InitFSR2");

	if (IsInited())
		return true;

	if (PhysicalDevice == nullptr)
	{
		spdlog::error("FSR2FeatureVk::InitFSR2 PhysicalDevice is null!");
		return false;
	}

	_scratchBufferSize = ffxGetScratchMemorySizeVK(PhysicalDevice, FFX_FSR2_CONTEXT_COUNT);
	void* scratchBuffer = calloc(_scratchBufferSize, 1);

	_deviceContext.vkDevice = Device;
	_deviceContext.vkDeviceProcAddr = vkGetDeviceProcAddr;
	_deviceContext.vkPhysicalDevice = PhysicalDevice;

	_ffxDevice = ffxGetDeviceVK(&_deviceContext);

	auto errorCode = ffxGetInterfaceVK(&_contextDesc.backendInterface, _ffxDevice, scratchBuffer, _scratchBufferSize, FFX_FSR2_CONTEXT_COUNT);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2FeatureVk::InitFSR2 ffxGetInterfaceVK error: {0}", ResultToString(errorCode));
		free(scratchBuffer);
		return false;
	}

	_contextDesc.maxRenderSize.width = static_cast<uint32_t>(RenderWidth());
	_contextDesc.maxRenderSize.height = static_cast<uint32_t>(RenderHeight());
	_contextDesc.displaySize.width = static_cast<uint32_t>(DisplayWidth());
	_contextDesc.displaySize.height = static_cast<uint32_t>(DisplayHeight());

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
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (DepthInverted) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->AutoExposure.value_or(AutoExposure))
	{
		Config::Instance()->AutoExposure = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_AUTO_EXPOSURE;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (AutoExposure) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (!AutoExposure) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->HDR.value_or(Hdr))
	{
		Config::Instance()->HDR = false;
		_contextDesc.flags |= FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2FeatureVk::InitFSR2 contextDesc.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	_contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INFINITE;

#if _DEBUG
	_contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
	_contextDesc.fpMessage = FfxLogCallback;
#endif

	spdlog::debug("FSR2FeatureVk::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = ffxFsr2ContextCreate(&_context, &_contextDesc);

	if (ret != FFX_OK)
	{
		spdlog::error("FSR2FeatureVk::InitFSR2 ffxFsr2ContextCreate error: {0}", ResultToString(ret));
		return false;
	}

	SetInit(true);

	return true;
}

bool FSR2FeatureVk::Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk::Init");

	if (IsInited())
		return true;

	Instance = InInstance;
	PhysicalDevice = InPD;
	Device = InDevice;
	GIPA = InGIPA;
	GDPA = InGDPA;

	return InitFSR2(InParameters);
}

bool FSR2FeatureVk::Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("FSR2FeatureVk::Evaluate");

	if (!IsInited())
		return false;

	FfxFsr2DispatchDescription params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffset.x);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffset.y);

	unsigned int reset;
	InParameters->Get(NVSDK_NGX_Parameter_Reset, &reset);
	params.reset = (reset == 1);

	GetRenderResolution(InParameters, &params.renderSize.width, &params.renderSize.height);

	spdlog::debug("FSR2FeatureVk::Evaluate Input Resolution: {0}x{1}", params.renderSize.width, params.renderSize.height);

	params.commandList = ffxGetCommandListVK(InCmdBuffer);

	void* paramColor;
	InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor);

	if (paramColor)
	{
		spdlog::debug("FSR2FeatureVk::Evaluate Color exist..");

		//if (Config::Instance()->MVResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramVelocity,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.color = ffxGetResourceVK(paramColor, GetFfxResourceDescriptionVk(paramColor, FFX_RESOURCE_USAGE_READ_ONLY), (wchar_t*)L"FSR3Upscale_Color", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureVk::Evaluate Color not exist!!");
		return false;
	}

	void* paramVelocity;
	InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity);

	if (paramVelocity)
	{
		spdlog::debug("FSR2FeatureVk::Evaluate MotionVectors exist..");

		//if (Config::Instance()->MVResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramVelocity,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.color = ffxGetResourceVK(paramVelocity, GetFfxResourceDescriptionVk(paramVelocity, FFX_RESOURCE_USAGE_READ_ONLY), (wchar_t*)L"FSR3Upscale_MotionVectors", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		spdlog::error("FSR2FeatureVk::Evaluate MotionVectors not exist!!");
		return false;
	}

	void* paramOutput;
	InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);

	if (paramOutput)
	{
		spdlog::debug("FSR3FeatureDx12::Evaluate Output exist..");

		//if (Config::Instance()->OutputResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramOutput,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		params.output = ffxGetResourceVK(paramOutput, GetFfxResourceDescriptionVk(paramOutput, FFX_RESOURCE_USAGE_UAV), (wchar_t*)L"FSR3Upscale_Output", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	else
	{
		spdlog::error("FSR2FeatureVk::Evaluate Output not exist!!");
		return false;
	}

	void* paramDepth;
	InParameters->Get(NVSDK_NGX_Parameter_Depth, &paramDepth);

	if (paramDepth)
	{
		spdlog::debug("FSR2FeatureVk::Evaluate Depth exist..");

		//if (Config::Instance()->DepthResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramDepth,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.depth = ffxGetResourceVK(paramDepth, GetFfxResourceDescriptionVk(paramDepth, FFX_RESOURCE_USAGE_READ_ONLY), (wchar_t*)L"FSR3Upscale_Depth", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("FSR2FeatureVk::Evaluate Depth not exist!!");
		else
			spdlog::info("FSR2FeatureVk::Evaluate Using high res motion vectors, depth is not needed!!");
	}

	void* paramExp = nullptr;
	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExp);

		if (paramExp)
			spdlog::debug("FSR2FeatureVk::Evaluate ExposureTexture exist..");
		else
			spdlog::debug("FSR2FeatureVk::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");

		//if (Config::Instance()->ExposureResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramExp,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.exposure = ffxGetResourceVK(paramExp, GetFfxResourceDescriptionVk(paramExp, FFX_RESOURCE_USAGE_READ_ONLY), (wchar_t*)L"FSR3Upscale_Exposure", FFX_RESOURCE_STATE_COMPUTE_READ);
	}
	else
		spdlog::debug("FSR2FeatureVk::Evaluate AutoExposure enabled!");

	void* paramMask = nullptr;
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramMask);

		if (paramMask)
			spdlog::debug("FSR2FeatureVk::Evaluate Bias mask exist..");
		else
			spdlog::debug("FSR2FeatureVk::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");

		//if (Config::Instance()->MaskResourceBarrier.has_value())
		//	ResourceBarrier(InCommandList, paramMask,
		//		(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(),
		//		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.reactive = ffxGetResourceVK(paramMask, GetFfxResourceDescriptionVk(paramMask, FFX_RESOURCE_USAGE_READ_ONLY), (wchar_t*)L"FSR3Upscale_Reactive", FFX_RESOURCE_STATE_COMPUTE_READ);
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
		spdlog::warn("FSR2FeatureVk::Evaluate Can't get motion vector scales!");


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

	spdlog::debug("FSR2FeatureVk::Evaluate Dispatch!!");
	auto result = ffxFsr2ContextDispatch(&_context, &params);

	if (result != FFX_OK)
	{
		spdlog::error("FSR2FeatureVk::Evaluate ffxFsr2ContextDispatch error: {0}", ResultToString(result));
		return false;
	}

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
