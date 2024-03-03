#include "pch.h"
#include "FSR2Feature.h"
#include "Config.h"

inline void logCallback(FfxMsgType type, const wchar_t* message)
{
	std::wstring string(message);
	std::string str(string.begin(), string.end());

	if (type == FFX_MESSAGE_TYPE_ERROR)
		spdlog::error("FSR2Feature::LogCallback FSR Runtime: {0}", str);
	else if (type == FFX_MESSAGE_TYPE_WARNING)
		spdlog::warn("FSR2Feature::LogCallback FSR Runtime: {0}", str);
	else
		spdlog::debug("FSR2Feature::LogCallback FSR Runtime: {0}", str);
}

double FSR2Feature::GetDeltaTime()
{
	double currentTime = MillisecondsNow();
	double deltaTime = (currentTime - _lastFrameTime);
	_lastFrameTime = currentTime;
	return deltaTime;
}

bool FSR2Feature::IsDepthInverted() const
{
	return _depthInverted;
}

bool FSR2Feature::InitFSR2(ID3D12Device* InDevice, const NVSDK_NGX_Parameter* InParameters)
{
	if (IsInited())
		return true;

	if (InDevice == nullptr)
	{
		spdlog::error("FSR2Feature::InitFSR2 D3D12Device is null!");
		return false;
	}

	const size_t scratchBufferSize = ffxGetScratchMemorySizeDX12(FFX_FSR2_CONTEXT_COUNT);
	void* scratchBuffer = calloc(scratchBufferSize, 1);

	auto errorCode = ffxGetInterfaceDX12(&_contextDesc.backendInterface, InDevice, scratchBuffer, scratchBufferSize, FFX_FSR2_CONTEXT_COUNT);

	if (errorCode != FFX_OK)
	{
		spdlog::error("FSR2Feature::InitFSR2 ffxGetInterfaceDX12 error: {0:x}", errorCode);
		free(scratchBuffer);
		return false;
	}

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
		_depthInverted = true;
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
		spdlog::info("FSR2Feature::InitFSR2 xessParams.initFlags (HDR) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("FSR2Feature::InitFSR2 xessParams.initFlags (!HDR) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->JitterCancellation.value_or(JitterMotion))
	{
		Config::Instance()->JitterCancellation = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION;
		spdlog::info("FSR2Feature::InitFSR2 xessParams.initFlags (JitterCancellation) {0:b}", _contextDesc.flags);
	}

	if (Config::Instance()->DisplayResolution.value_or(!LowRes))
	{
		Config::Instance()->DisplayResolution = true;
		_contextDesc.flags |= FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS;
		spdlog::info("FSR2Feature::InitFSR2 xessParams.initFlags (!LowResMV) {0:b}", _contextDesc.flags);
	}
	else
	{
		Config::Instance()->DisplayResolution = false;
		spdlog::info("FSR2Feature::InitFSR2 xessParams.initFlags (LowResMV) {0:b}", _contextDesc.flags);
	}

	_contextDesc.flags |= FFX_FSR2_ENABLE_DEPTH_INFINITE;

#if _DEBUG
	_contextDesc.flags |= FFX_FSR2_ENABLE_DEBUG_CHECKING;
	_contextDesc.fpMessage = logCallback;
#endif

	spdlog::debug("FSR2Feature::InitFSR2 ffxFsr2ContextCreate!");

	auto ret = ffxFsr2ContextCreate(&_context, &_contextDesc);

	if (ret != FFX_OK)
	{
		spdlog::error("FSR2Feature::InitFSR2 ffxFsr2ContextCreate error: {0:x}", ret);
		return false;
	}

	SetInit(true);

	return true;
}

double FSR2Feature::MillisecondsNow()
{
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	double milliseconds = 0;

	if (s_use_qpc)
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		milliseconds = double(1000.0 * now.QuadPart) / s_frequency.QuadPart;
	}
	else
	{
		milliseconds = double(GetTickCount());
	}

	return milliseconds;
}

FSR2Feature::~FSR2Feature()
{
	spdlog::debug("FSR2Feature::~FSR2Feature");

	if (!IsInited())
		return;

	auto errorCode = ffxFsr2ContextDestroy(&_context);

	if (errorCode != FFX_OK)
		spdlog::error("FSR2Feature::~FSR2Feature ffxFsr2ContextDestroy error: {0:x}", errorCode);

	free(_contextDesc.backendInterface.scratchBuffer);

	SetInit(false);
}


