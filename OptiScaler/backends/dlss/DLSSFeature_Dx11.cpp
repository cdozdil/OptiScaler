#include "DLSSFeature_Dx11.h"
#include <dxgi.h>
#include "../../Config.h"
#include "../../pch.h"

bool DLSSFeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureDx11::Init nvngx.dll not loaded!");

		SetInit(false);
		return false;
	}

	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	Device = InDevice;
	DeviceContext = InContext;

	do
	{
		if (!_dlssInited)
		{
			_dlssInited = NVNGXProxy::InitDx11(InDevice);

			if (!_dlssInited)
				return false;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (NVNGXProxy::D3D11_AllocateParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureDx11::Init _AllocateParameters will be used");

			nvResult = NVNGXProxy::D3D11_AllocateParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx11::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else if (NVNGXProxy::D3D11_GetParameters() != nullptr)
		{
			spdlog::debug("DLSSFeatureDx11::Init _GetParameters will be used");

			nvResult = NVNGXProxy::D3D11_GetParameters()(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx11::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}

#ifdef DLSS_PARAM_DUMP
			DumpNvParams(Parameters);
#endif
		}
		else
		{
			spdlog::error("DLSSFeatureDx11::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureDx12::Evaluate Creating DLSS feature");

		if (NVNGXProxy::D3D11_CreateFeature() != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = NVNGXProxy::D3D11_CreateFeature()(InContext, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Evaluate _CreateFeature result: {0:X}", (unsigned int)nvResult);
				break;
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

	if (initResult)
	{
		if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
			Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), InDevice);
	}

	SetInit(initResult);

	return initResult;
}

bool DLSSFeatureDx11::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureDx11::Evaluate nvngx.dll or _nvngx.dll is not loaded!");
		return false;
	}

	NVSDK_NGX_Result nvResult;
	ID3D11Resource* paramOutput;

	if (NVNGXProxy::D3D11_EvaluateFeature() != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = NVNGXProxy::D3D11_EvaluateFeature()(InDeviceContext, _p_dlssHandle, Parameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			spdlog::error("DLSSFeatureDx12::Evaluate _EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}
		else
		{
			spdlog::trace("DLSSFeatureDx12::Evaluate _EvaluateFeature ok!");
		}
	}
	else
	{
		spdlog::error("DLSSFeatureDx11::Evaluate _EvaluateFeature is nullptr");
		return false;
	}

	// imgui
	if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30 && 
		Parameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) == NVSDK_NGX_Result_Success)
	{
		if (Imgui != nullptr && Imgui.get() != nullptr)
		{
			if (Imgui->IsHandleDifferent())
				Imgui.reset();
			else
				Imgui->Render(InDeviceContext, paramOutput);
		}
		else
		{
			if (Imgui == nullptr || Imgui.get() == nullptr)
				Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
		}
	}

	_frameCount++;

	return true;
}

void DLSSFeatureDx11::Shutdown(ID3D11Device* InDevice)
{
	if (_dlssInited)
	{
		if (NVNGXProxy::D3D11_Shutdown() != nullptr)
			NVNGXProxy::D3D11_Shutdown()();
		else if (NVNGXProxy::D3D11_Shutdown1() != nullptr)
			NVNGXProxy::D3D11_Shutdown1()(InDevice);
	}

	DLSSFeature::Shutdown();
}

DLSSFeatureDx11::DLSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		spdlog::info("DLSSFeatureDx11::DLSSFeatureDx11 nvngx.dll not loaded, now loading");
		NVNGXProxy::InitNVNGX();
	}

	_moduleLoaded = (NVNGXProxy::D3D11_Init_with_ProjectID() != nullptr || NVNGXProxy::D3D11_Init_Ext() != nullptr) &&
		(NVNGXProxy::D3D11_Shutdown() != nullptr || NVNGXProxy::D3D11_Shutdown1() != nullptr) &&
		(NVNGXProxy::D3D11_GetParameters() != nullptr || NVNGXProxy::D3D11_AllocateParameters() != nullptr) && NVNGXProxy::D3D11_DestroyParameters() != nullptr &&
		NVNGXProxy::D3D11_CreateFeature() != nullptr && NVNGXProxy::D3D11_ReleaseFeature() != nullptr && NVNGXProxy::D3D11_EvaluateFeature() != nullptr;

	spdlog::info("DLSSFeatureDx11::DLSSFeatureDx11 binding complete!");
}

DLSSFeatureDx11::~DLSSFeatureDx11()
{
	if (Parameters != nullptr && NVNGXProxy::D3D11_DestroyParameters() != nullptr)
		NVNGXProxy::D3D11_DestroyParameters()(Parameters);

	if (NVNGXProxy::D3D11_ReleaseFeature() != nullptr && _p_dlssHandle != nullptr)
		NVNGXProxy::D3D11_ReleaseFeature()(_p_dlssHandle);
}
