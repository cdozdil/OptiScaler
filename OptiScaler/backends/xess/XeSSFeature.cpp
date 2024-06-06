#pragma once
#include "../../pch.h"
#include "../../Config.h"
#include "../../Util.h"

#include "../../detours/detours.h"

#include "XeSSFeature.h"
#include "../../d3dx/d3dx12.h"

inline static void XeSSLogCallback(const char* Message, xess_logging_level_t Level)
{
	spdlog::log((spdlog::level::level_enum)((int)Level + 1), "FeatureContext::LogCallback XeSS Runtime ({0})", Message);
}

bool XeSSFeature::InitXeSS(ID3D12Device* device, const IFeatureCreateParams InParams)
{
	spdlog::debug("XeSSFeature::InitXeSS!");

	if (!_moduleLoaded)
	{
		spdlog::error("XeSSFeature::InitXeSS libxess.dll not loaded!");
		return false;
	}

	if (IsInited())
		return true;

	if (device == nullptr)
	{
		spdlog::error("XeSSFeature::InitXeSS D3D12Device is null!");
		return false;
	}

	auto ret = GetVersion()(&_xessVersion);

	if (ret == XESS_RESULT_SUCCESS)
	{
		spdlog::info("XeSSFeature::InitXeSS XeSS Version: {0}.{1}.{2}", _xessVersion.major, _xessVersion.minor, _xessVersion.patch);
		_version = std::format("{0}.{1}.{2}", _xessVersion.major, _xessVersion.minor, _xessVersion.patch);
	}
	else
		spdlog::warn("XeSSFeature::InitXeSS xessGetVersion error: {0}", ResultToString(ret));

	ret = D3D12CreateContext()(device, &_xessContext);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeature::InitXeSS xessD3D12CreateContext error: {0}", ResultToString(ret));
		return false;
	}

	ret = IsOptimalDriver()(_xessContext);
	spdlog::debug("XeSSFeature::InitXeSS xessIsOptimalDriver : {0}", ResultToString(ret));

	ret = SetLoggingCallback()(_xessContext, XESS_LOGGING_LEVEL_DEBUG, XeSSLogCallback);
	spdlog::debug("XeSSFeature::InitXeSS xessSetLoggingCallback : {0}", ResultToString(ret));

	xess_d3d12_init_params_t xessParams{};

	xessParams.initFlags = XESS_INIT_FLAG_NONE;

	if (Config::Instance()->DepthInverted.value_or(InParams.DepthInverted()))
	{
		Config::Instance()->DepthInverted = true;
		xessParams.initFlags |= XESS_INIT_FLAG_INVERTED_DEPTH;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (DepthInverted) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->AutoExposure.value_or(InParams.AutoExposure()))
	{
		Config::Instance()->AutoExposure = true;
		xessParams.initFlags |= XESS_INIT_FLAG_ENABLE_AUTOEXPOSURE;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (AutoExposure) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->AutoExposure = false;
		xessParams.initFlags |= XESS_INIT_FLAG_EXPOSURE_SCALE_TEXTURE;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!AutoExposure) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->HDR.value_or(InParams.Hdr()))
	{
		Config::Instance()->HDR = false;
		xessParams.initFlags |= XESS_INIT_FLAG_LDR_INPUT_COLOR;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!HDR) {0:b}", xessParams.initFlags);
	}
	else
	{
		Config::Instance()->HDR = true;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (HDR) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->JitterCancellation.value_or(InParams.JitterCancellation()))
	{
		Config::Instance()->JitterCancellation = true;
		xessParams.initFlags |= XESS_INIT_FLAG_JITTERED_MV;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (JitterCancellation) {0:b}", xessParams.initFlags);
	}

	if (Config::Instance()->DisplayResolution.value_or(InParams.DisplayResolutionMV()))
	{
		xessParams.initFlags |= XESS_INIT_FLAG_HIGH_RES_MV;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (!LowResMV) {0:b}", xessParams.initFlags);
	}
	else
	{
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (LowResMV) {0:b}", xessParams.initFlags);
	}

	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		Config::Instance()->DisableReactiveMask = false;
		xessParams.initFlags |= XESS_INIT_FLAG_RESPONSIVE_PIXEL_MASK;
		spdlog::info("XeSSFeature::InitXeSS xessParams.initFlags (ReactiveMaskActive) {0:b}", xessParams.initFlags);
	}

	switch (InParams.QualityPreset())
	{
	case UltraPerformance:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_PERFORMANCE;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;

		break;

	case Performance:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_PERFORMANCE;

		break;

	case Balanced:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_QUALITY;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED;

		break;

	case Quality:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_QUALITY;

		break;

	case UltraQuality:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY_PLUS;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;

		break;

	case NativeAA:
		if (_xessVersion.major >= 1 && _xessVersion.minor >= 3)
			xessParams.qualitySetting = XESS_QUALITY_SETTING_AA;
		else
			xessParams.qualitySetting = XESS_QUALITY_SETTING_ULTRA_QUALITY;

		break;

	default:
		xessParams.qualitySetting = XESS_QUALITY_SETTING_BALANCED; //Set out-of-range value for non-existing XESS_QUALITY_SETTING_BALANCED mode
		break;
	}

	xessParams.outputResolution.x = TargetWidth();
	xessParams.outputResolution.y = TargetHeight();

	// create heaps to prevent create heap errors of xess
	if (Config::Instance()->CreateHeaps.value_or(true))
	{
		HRESULT hr;
		xess_properties_t xessProps{};
		ret = GetProperties()(_xessContext, &xessParams.outputResolution, &xessProps);

		if (ret == XESS_RESULT_SUCCESS)
		{
			CD3DX12_HEAP_DESC bufferHeapDesc(xessProps.tempBufferHeapSize, D3D12_HEAP_TYPE_DEFAULT);
			hr = device->CreateHeap(&bufferHeapDesc, IID_PPV_ARGS(&_localBufferHeap));

			if (SUCCEEDED(hr))
			{
				D3D12_HEAP_DESC textureHeapDesc{ xessProps.tempTextureHeapSize,
					{D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0},
					0, D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES };

				hr = device->CreateHeap(&textureHeapDesc, IID_PPV_ARGS(&_localTextureHeap));

				if (SUCCEEDED(hr))
				{
					Config::Instance()->CreateHeaps = true;

					spdlog::info("XeSSFeature::InitXeSS using _localBufferHeap & _localTextureHeap!");

					xessParams.bufferHeapOffset = 0;
					xessParams.textureHeapOffset = 0;
					xessParams.pTempBufferHeap = _localBufferHeap;
					xessParams.pTempTextureHeap = _localTextureHeap;
				}
				else
				{
					spdlog::error("XeSSFeature::InitXeSS CreateHeap textureHeapDesc failed {0:x}!", hr);
				}
			}
			else
			{
				spdlog::error("XeSSFeature::InitXeSS CreateHeap bufferHeapDesc failed {0:x}!", hr);
			}

		}
		else
		{
			spdlog::error("XeSSFeature::InitXeSS xessGetProperties failed {0}!", ResultToString(ret));
		}
	}

	// try to build pipelines with local pipeline object
	if (Config::Instance()->BuildPipelines.value_or(true))
	{
		spdlog::debug("XeSSFeature::InitXeSS xessD3D12BuildPipelines!");

		ID3D12Device1* device1;
		if (FAILED(device->QueryInterface(IID_PPV_ARGS(&device1))))
		{
			spdlog::error("XeSSFeature::InitXeSS QueryInterface device1 failed!");
			ret = D3D12BuildPipelines()(_xessContext, NULL, false, xessParams.initFlags);
		}
		else
		{
			HRESULT hr = device1->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(&_localPipeline));

			if (FAILED(hr) || !_localPipeline)
			{
				spdlog::error("XeSSFeature::InitXeSS CreatePipelineLibrary failed {0:x}!", hr);
				ret = D3D12BuildPipelines()(_xessContext, NULL, false, xessParams.initFlags);
			}
			else
			{
				ret = D3D12BuildPipelines()(_xessContext, _localPipeline, false, xessParams.initFlags);

				if (ret != XESS_RESULT_SUCCESS)
				{
					spdlog::error("XeSSFeature::InitXeSS xessD3D12BuildPipelines error with _localPipeline: {0}", ResultToString(ret));
					ret = D3D12BuildPipelines()(_xessContext, NULL, false, xessParams.initFlags);
				}
				else
				{
					spdlog::info("XeSSFeature::InitXeSS using _localPipelines!");
					xessParams.pPipelineLibrary = _localPipeline;
				}
			}
		}

		if (device1 != nullptr)
			device1->Release();

		if (ret != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeature::InitXeSS xessD3D12BuildPipelines error: {0}", ResultToString(ret));
			return false;
		}
	}

	spdlog::debug("XeSSFeature::InitXeSS xessD3D12Init!");

	if (Config::Instance()->NetworkModel.has_value() && Config::Instance()->NetworkModel.value() >= 0 && Config::Instance()->NetworkModel.value() <= 5)
	{
		ret = SelectNetworkModel()(_xessContext, (xess_network_model_t)Config::Instance()->NetworkModel.value());
		spdlog::error("XeSSFeature::InitXeSS xessSelectNetworkModel result: {0}", ResultToString(ret));
	}

	ret = D3D12Init()(_xessContext, &xessParams);

	if (ret != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeature::InitXeSS xessD3D12Init error: {0}", ResultToString(ret));
		return false;
	}

	SetInit(true);

	return true;
}

XeSSFeature::XeSSFeature(unsigned int handleId, const IFeatureCreateParams InParams) : IFeature(handleId, InParams)
{
	PRN_xessGetVersion ptrMemoryGetVersion = (PRN_xessGetVersion)DetourFindFunction("libxess.dll", "xessGetVersion");
	PRN_xessGetVersion ptrDllGetVersion = nullptr;

	xess_version_t memoryVersion{ 0,0,0,0 };
	xess_version_t dllVersion{ 0,0,0,0 };

	// if there is libxess already loaded 
	if (ptrMemoryGetVersion)
	{
		// get it's version to compare with dll
		ptrMemoryGetVersion(&memoryVersion);

		spdlog::info("XeSSFeature::XeSSFeature DetourFindFunction loaded libxess.dll v{0}.{1}.{2}.", memoryVersion.major, memoryVersion.minor, memoryVersion.patch);

		_xessD3D12CreateContext = (PFN_xessD3D12CreateContext)DetourFindFunction("libxess.dll", "xessD3D12CreateContext");
		_xessD3D12BuildPipelines = (PFN_xessD3D12BuildPipelines)DetourFindFunction("libxess.dll", "xessD3D12BuildPipelines");
		_xessD3D12Init = (PRN_xessD3D12Init)DetourFindFunction("libxess.dll", "xessD3D12Init");
		_xessGetVersion = (PRN_xessGetVersion)DetourFindFunction("libxess.dll", "xessGetVersion");
		_xessD3D12Execute = (PFN_xessD3D12Execute)DetourFindFunction("libxess.dll", "xessD3D12Execute");
		_xessSelectNetworkModel = (PFN_xessSelectNetworkModel)DetourFindFunction("libxess.dll", "xessSelectNetworkModel");
		_xessStartDump = (PFN_xessStartDump)DetourFindFunction("libxess.dll", "xessStartDump");
		_xessIsOptimalDriver = (PFN_xessIsOptimalDriver)DetourFindFunction("libxess.dll", "xessIsOptimalDriver");
		_xessSetLoggingCallback = (PFN_xessSetLoggingCallback)DetourFindFunction("libxess.dll", "xessSetLoggingCallback");
		_xessGetProperties = (PFN_xessGetProperties)DetourFindFunction("libxess.dll", "xessGetProperties");
		_xessDestroyContext = (PFN_xessDestroyContext)DetourFindFunction("libxess.dll", "xessDestroyContext");
		_xessSetVelocityScale = (PFN_xessSetVelocityScale)DetourFindFunction("libxess.dll", "xessSetVelocityScale");

		_moduleLoaded = _xessD3D12CreateContext && _xessD3D12BuildPipelines && _xessD3D12Init && _xessD3D12Execute && _xessSelectNetworkModel && _xessStartDump &&
			_xessGetVersion && _xessIsOptimalDriver && _xessSetLoggingCallback && _xessGetProperties && _xessDestroyContext && _xessSetVelocityScale;
	}

	if (Config::Instance()->XeSSLibrary.has_value())
	{
		std::filesystem::path cfgPath(Config::Instance()->XeSSLibrary.value().c_str());
		auto loadPath = cfgPath / L"opti.libxess.dll";
		_libxess = LoadLibraryW(loadPath.c_str());

		if (_libxess == nullptr)
		{
			loadPath = cfgPath / L"libxess.dll";
			_libxess = LoadLibraryW(loadPath.c_str());
		}
	}

	if (_libxess == nullptr)
	{
		auto path = Util::DllPath().parent_path() / L"opti.libxess.dll";
		_libxess = LoadLibraryW(path.c_str());

		if (_libxess == nullptr)
		{
			path = Util::DllPath().parent_path() / L"libxess.dll";
			_libxess = LoadLibraryW(path.c_str());
		}
	}

	if (_libxess)
	{
		ptrDllGetVersion = (PRN_xessGetVersion)GetProcAddress(_libxess, "xessGetVersion");

		// query dll version
		if (ptrDllGetVersion)
		{
			ptrDllGetVersion(&dllVersion);
			spdlog::info("XeSSFeature::XeSSFeature Loaded libxess.dll v{0}.{1}.{2} file.", dllVersion.major, dllVersion.minor, dllVersion.patch);
		}

		// check versions and ptr if they are same we are loaded same file prevent freelibrary call
		if (dllVersion.major == memoryVersion.major && dllVersion.minor == memoryVersion.minor &&
			dllVersion.patch == memoryVersion.patch && dllVersion.reserved == memoryVersion.reserved &&
			ptrDllGetVersion == ptrMemoryGetVersion)
		{
			spdlog::info("XeSSFeature::XeSSFeature Both libxess.dll versions are same!");
			_libxess = nullptr;
		}
		else
		{
			spdlog::info("XeSSFeature::XeSSFeature Using loaded libxess.dll library!");

			// we would like to prioritize file pointed at ini, use methods from loaded dll
			_xessD3D12CreateContext = (PFN_xessD3D12CreateContext)GetProcAddress(_libxess, "xessD3D12CreateContext");
			_xessD3D12BuildPipelines = (PFN_xessD3D12BuildPipelines)GetProcAddress(_libxess, "xessD3D12BuildPipelines");
			_xessD3D12Init = (PRN_xessD3D12Init)GetProcAddress(_libxess, "xessD3D12Init");
			_xessD3D12Execute = (PFN_xessD3D12Execute)GetProcAddress(_libxess, "xessD3D12Execute");
			_xessSelectNetworkModel = (PFN_xessSelectNetworkModel)GetProcAddress(_libxess, "xessSelectNetworkModel");
			_xessStartDump = (PFN_xessStartDump)GetProcAddress(_libxess, "xessStartDump");
			_xessGetVersion = (PRN_xessGetVersion)GetProcAddress(_libxess, "xessGetVersion");
			_xessIsOptimalDriver = (PFN_xessIsOptimalDriver)GetProcAddress(_libxess, "xessIsOptimalDriver");
			_xessSetLoggingCallback = (PFN_xessSetLoggingCallback)GetProcAddress(_libxess, "xessSetLoggingCallback");
			_xessGetProperties = (PFN_xessGetProperties)GetProcAddress(_libxess, "xessGetProperties");
			_xessDestroyContext = (PFN_xessDestroyContext)GetProcAddress(_libxess, "xessDestroyContext");
			_xessSetVelocityScale = (PFN_xessSetVelocityScale)GetProcAddress(_libxess, "xessSetVelocityScale");

			_moduleLoaded = true;
		}
	}
}

XeSSFeature::~XeSSFeature()
{
	if (_xessContext)
	{
		DestroyContext()(_xessContext);
		_xessContext = nullptr;
	}

	if (_localPipeline != nullptr)
	{
		_localPipeline->Release();
		_localPipeline = nullptr;
	}

	if (_localBufferHeap != nullptr)
	{
		_localBufferHeap->Release();
		_localBufferHeap = nullptr;
	}

	if (_localTextureHeap != nullptr)
	{
		_localTextureHeap->Release();
		_localTextureHeap = nullptr;
	}

	if (_moduleLoaded && _libxess != nullptr)
		FreeLibrary(_libxess);
}

