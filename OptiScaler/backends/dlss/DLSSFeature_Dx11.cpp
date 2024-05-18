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

	HRESULT result;
	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	Device = InDevice;
	DeviceContext = InContext;

	do
	{
		if (!_dlssInited)
		{
			NVSDK_NGX_FeatureCommonInfo fcInfo{};

			GetFeatureCommonInfo(&fcInfo);

			if (Config::Instance()->NVNGX_ProjectId != "" && _Init_with_ProjectID != nullptr)
			{
				spdlog::debug("DLSSFeatureDx11::Init _Init_with_ProjectID!");

				nvResult = _Init_with_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
					Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else if (_Init_Ext != nullptr)
			{
				spdlog::debug("DLSSFeatureDx11::Init _Init_Ext!");

				nvResult = _Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else
			{
				spdlog::error("DLSSFeatureDx11::Init _Init_with_ProjectID and  is null");
				break;
			}

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx11::Init _Init_with_ProjectID result: {0:X}", (unsigned int)nvResult);
				break;
			}

			_dlssInited = true;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (_AllocateParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureDx11::Init _AllocateParameters will be used");

			nvResult = _AllocateParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx11::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else if (_GetParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureDx11::Init _GetParameters will be used");

			nvResult = _GetParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx11::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else
		{
			spdlog::error("DLSSFeatureDx11::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureDx12::Evaluate Creating DLSS feature");

		if (_CreateFeature != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = _CreateFeature(InContext, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

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

	if (_EvaluateFeature != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = _EvaluateFeature(InDeviceContext, _p_dlssHandle, Parameters, NULL);

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

	_frameCount++;

	return true;
}

void DLSSFeatureDx11::Shutdown(ID3D11Device* InDevice)
{
	if (_dlssInited)
	{
		if (_Shutdown != nullptr)
			_Shutdown();
		else if (_Shutdown1 != nullptr)
			_Shutdown1(InDevice);
	}

	DLSSFeature::Shutdown();
}

DLSSFeatureDx11::DLSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (!ModuleLoaded())
	{
		spdlog::error("DLSSFeatureDx11::DLSSFeatureDx11 nvngx.dll not loaded!");
		return;
	}

	spdlog::info("DLSSFeatureDx11::DLSSFeatureDx11 binding methods!");

	if (_Shutdown == nullptr)
		_Shutdown = (PFN_NVSDK_NGX_D3D11_Shutdown)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_Shutdown");

	if (_Shutdown1 == nullptr)
		_Shutdown1 = (PFN_NVSDK_NGX_D3D11_Shutdown1)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_Shutdown1");

	if (_Init_Ext == nullptr)
		_Init_Ext = (PFN_NVSDK_NGX_D3D11_Init_Ext)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_Init_Ext");

	if (_Init_with_ProjectID == nullptr)
		_Init_with_ProjectID = (PFN_NVSDK_NGX_D3D11_Init_ProjectID)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_Init_ProjectID");

	if (_GetParameters == nullptr)
		_GetParameters = (PFN_NVSDK_NGX_D3D11_GetParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_GetParameters");

	if (_AllocateParameters == nullptr)
		_AllocateParameters = (PFN_NVSDK_NGX_D3D11_AllocateParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_AllocateParameters");

	if (_DestroyParameters == nullptr)
		_DestroyParameters = (PFN_NVSDK_NGX_D3D11_DestroyParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_DestroyParameters");

	if (_CreateFeature == nullptr)
		_CreateFeature = (PFN_NVSDK_NGX_D3D11_CreateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_CreateFeature");

	if (_ReleaseFeature == nullptr)
		_ReleaseFeature = (PFN_NVSDK_NGX_D3D11_ReleaseFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_ReleaseFeature");

	if (_EvaluateFeature == nullptr)
		_EvaluateFeature = (PFN_NVSDK_NGX_D3D11_EvaluateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D11_EvaluateFeature");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _Init_with_ProjectID ptr: {0:X}", (unsigned long)_Init_with_ProjectID);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _Init_with_ProjectID ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _Init_Ext ptr: {0:X}", (unsigned long)_Init_Ext);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _Init_Ext ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _Shutdown ptr: {0:X}", (unsigned long)_Shutdown);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _Shutdown ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _Shutdown1 ptr: {0:X}", (unsigned long)_Shutdown1);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _Shutdown1 ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _GetParameters ptr: {0:X}", (unsigned long)_GetParameters);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _GetParameters ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _AllocateParameters ptr: {0:X}", (unsigned long)_AllocateParameters);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _AllocateParameters ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _DestroyParameters ptr: {0:X}", (unsigned long)_DestroyParameters);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _DestroyParameters ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _CreateFeature ptr: {0:X}", (unsigned long)_CreateFeature);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _CreateFeature ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _ReleaseFeature ptr: {0:X}", (unsigned long)_ReleaseFeature);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _ReleaseFeature ptr: nullptr");

	if (_Init_with_ProjectID)
		spdlog::trace("DLSSFeatureDx11::DLSSFeatureDx11 _EvaluateFeature ptr: {0:X}", (unsigned long)_EvaluateFeature);
	else
		spdlog::warn("DLSSFeatureDx11::DLSSFeatureDx11 _EvaluateFeature ptr: nullptr");

	_moduleLoaded = (_Init_with_ProjectID != nullptr || _Init_Ext != nullptr) && (_Shutdown != nullptr || _Shutdown1 != nullptr) &&
		(_GetParameters != nullptr || _AllocateParameters != nullptr) && _DestroyParameters != nullptr && _CreateFeature != nullptr &&
		_ReleaseFeature != nullptr && _EvaluateFeature != nullptr;

	spdlog::info("DLSSFeatureDx11::DLSSFeatureDx11 binding complete!");
}

DLSSFeatureDx11::~DLSSFeatureDx11()
{
	if (Parameters != nullptr && _DestroyParameters != nullptr)
		_DestroyParameters(Parameters);

	if (_ReleaseFeature != nullptr)
		_ReleaseFeature(_p_dlssHandle);
}
