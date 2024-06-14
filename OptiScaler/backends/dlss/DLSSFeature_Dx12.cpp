#include "DLSSFeature_Dx12.h"
#include <dxgi1_4.h>
#include "../../Config.h"
#include "../../pch.h"

bool DLSSFeatureDx12::Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		spdlog::error("DLSSFeatureDx12::Init nvngx.dll not loaded!");
		SetInit(false);
		return false;
	}

	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	Device = InDevice;

	do
	{
		if (!_dlssInited)
		{
			_dlssInited = NVNGXProxy::InitDx12(InDevice);

			if (!_dlssInited)
				return false;

			_moduleLoaded = (NVNGXProxy::D3D12_Init_ProjectID() != nullptr || NVNGXProxy::D3D12_Init_Ext() != nullptr) &&
				(NVNGXProxy::D3D12_Shutdown() != nullptr || NVNGXProxy::D3D12_Shutdown1() != nullptr) &&
				(NVNGXProxy::D3D12_GetParameters() != nullptr || NVNGXProxy::D3D12_AllocateParameters() != nullptr) &&
				NVNGXProxy::D3D12_DestroyParameters() != nullptr && NVNGXProxy::D3D12_CreateFeature() != nullptr &&
				NVNGXProxy::D3D12_ReleaseFeature() != nullptr && NVNGXProxy::D3D12_EvaluateFeature() != nullptr;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (NVNGXProxy::D3D12_AllocateParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureDx12::Init _AllocateParameters will be used");

			nvResult = NVNGXProxy::D3D12_AllocateParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else if (NVNGXProxy::D3D12_GetParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureDx12::Init _GetParameters will be used");

			nvResult = NVNGXProxy::D3D12_GetParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else
		{
			spdlog::error("DLSSFeatureDx12::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureDx12::Evaluate Creating DLSS feature");

		if (NVNGXProxy::D3D12_CreateFeature() != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = NVNGXProxy::D3D12_CreateFeature()(InCommandList, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Evaluate _CreateFeature result: {0:X}", (unsigned int)nvResult);
				break;
			}
			else
			{
				spdlog::info("DLSSFeatureDx12::Evaluate _CreateFeature result: NVSDK_NGX_Result_Success");
			}
		}
		else
		{
			spdlog::error("DLSSFeatureDx12::Evaluate _CreateFeature is nullptr");
			break;
		}

		ReadVersion();

		initResult = true;

	} while (false);

	bool rcasEnabled = (Version().major > 2 || (Version().major == 2 && Version().minor >= 5 && Version().patch >= 1));

	if (initResult)
	{
		if (Config::Instance()->RcasEnabled.value_or(rcasEnabled))
			RCAS = std::make_unique<RCAS_Dx12>("RCAS", InDevice);

		if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
			Imgui = std::make_unique<Imgui_Dx12>(Util::GetProcessWindow(), InDevice);

		OutputScaler = std::make_unique<BS_Dx12>("Output Downsample", InDevice, (TargetWidth() < DisplayWidth()));
	}

	SetInit(initResult);

	return initResult;
}

bool DLSSFeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureDx12::Evaluate nvngx.dll or _nvngx.dll is not loaded!");
		return false;
	}

	if (!IsInited())
	{
		spdlog::error("DLSSFeatureDx12::Evaluate Not inited!");
		return false;
	}

	bool rcasEnabled = (Version().major > 2 || (Version().major == 2 && Version().minor >= 5 && Version().patch >= 1));

	if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) && (RCAS == nullptr || RCAS.get() == nullptr || !RCAS->IsInit()))
		Config::Instance()->RcasEnabled = false;

	if (!OutputScaler->IsInit())
		Config::Instance()->OutputScalingEnabled = false;

	NVSDK_NGX_Result nvResult;

	if (NVNGXProxy::D3D12_EvaluateFeature() != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		ID3D12Resource* paramOutput = nullptr;
		ID3D12Resource* paramMotion = nullptr;
		ID3D12Resource* setBuffer = nullptr;

		bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

		Parameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);
		Parameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMotion);

		// supersampling
		if (useSS)
		{
			if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				setBuffer = OutputScaler->Buffer();
			}
			else
				setBuffer = paramOutput;
		}
		else
			setBuffer = paramOutput;

		// RCAS sharpness & preperation
		auto sharpness = GetSharpness(InParameters);

		if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) &&
			(sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
			RCAS->IsInit() && RCAS->CreateBufferResource(Device, setBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			// Disable DLSS sharpness
			Parameters->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);

			RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			setBuffer = RCAS->Buffer();
		}

		// sharpness override
		else if (Config::Instance()->OverrideSharpness.value_or(false))
		{
			sharpness = Config::Instance()->Sharpness.value_or(0.3);
			Parameters->Set(NVSDK_NGX_Parameter_Sharpness, sharpness);
		}

		Parameters->Set(NVSDK_NGX_Parameter_Output, setBuffer);

		nvResult = NVNGXProxy::D3D12_EvaluateFeature()(InCommandList, _p_dlssHandle, Parameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			spdlog::error("DLSSFeatureDx12::Evaluate _EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}

		// Apply CAS
		if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) &&
			(sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
			RCAS->CanRender())
		{
			if (setBuffer != RCAS->Buffer())
				ResourceBarrier(InCommandList, setBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			RcasConstants rcasConstants{};

			rcasConstants.Sharpness = sharpness;
			rcasConstants.DisplayWidth = TargetWidth();
			rcasConstants.DisplayHeight = TargetHeight();
			InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
			InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
			rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
			rcasConstants.RenderHeight = RenderHeight();
			rcasConstants.RenderWidth = RenderWidth();

			if (useSS)
			{
				if (!RCAS->Dispatch(Device, InCommandList, setBuffer, paramMotion, rcasConstants, OutputScaler->Buffer()))
				{
					Config::Instance()->RcasEnabled = false;
					return true;
				}
			}
			else
			{
				if (!RCAS->Dispatch(Device, InCommandList, setBuffer, paramMotion, rcasConstants, paramOutput))
				{
					Config::Instance()->RcasEnabled = false;
					return true;
				}
			}
		}

		// Downsampling
		if (useSS)
		{
			spdlog::debug("DLSSFeatureDx12::Evaluate downscaling output...");
			OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			if (!OutputScaler->Dispatch(Device, InCommandList, OutputScaler->Buffer(), paramOutput))
			{
				Config::Instance()->OutputScalingEnabled = false;
				Config::Instance()->changeBackend = true;
				return true;
			}
		}

		// imgui
		if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30 && paramOutput)
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
					Imgui = std::make_unique<Imgui_Dx12>(Util::GetProcessWindow(), Device);
			}
		}
	}
	else
	{
		spdlog::error("DLSSFeatureDx12::Evaluate _EvaluateFeature is nullptr");
		return false;
	}

	_frameCount++;

	return true;
}

void DLSSFeatureDx12::Shutdown(ID3D12Device* InDevice)
{
	if (_dlssInited)
	{
		if (NVNGXProxy::D3D12_Shutdown() != nullptr)
			NVNGXProxy::D3D12_Shutdown()();
		else if (NVNGXProxy::D3D12_Shutdown1() != nullptr)
			NVNGXProxy::D3D12_Shutdown1()(InDevice);
	}

	DLSSFeature::Shutdown();
}

DLSSFeatureDx12::DLSSFeatureDx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		spdlog::info("DLSSFeatureDx12::DLSSFeatureDx12 nvngx.dll not loaded, now loading");
		NVNGXProxy::InitNVNGX();
	}

	spdlog::info("DLSSFeatureDx12::DLSSFeatureDx12 binding complete!");
}

DLSSFeatureDx12::~DLSSFeatureDx12()
{
	if (Parameters != nullptr && NVNGXProxy::D3D12_DestroyParameters() != nullptr)
		NVNGXProxy::D3D12_DestroyParameters()(Parameters);

	if (NVNGXProxy::D3D12_ReleaseFeature() != nullptr && _p_dlssHandle != nullptr)
		NVNGXProxy::D3D12_ReleaseFeature()(_p_dlssHandle);

	if (RCAS != nullptr && RCAS.get() != nullptr)
		RCAS.reset();
}

float DLSSFeatureDx12::GetSharpness(const NVSDK_NGX_Parameter* InParameters)
{
	if (Config::Instance()->OverrideSharpness.value_or(false))
		return Config::Instance()->Sharpness.value_or(0.3f);

	float sharpness = 0.0f;
	InParameters->Get(NVSDK_NGX_Parameter_Sharpness, &sharpness);

	return sharpness;
}
