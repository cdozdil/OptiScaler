#include "DLSSFeature_Dx12.h"
#include <dxgi1_4.h>
#include "../../Config.h"
#include "../../pch.h"

bool DLSSFeatureDx12::Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		spdlog::error("DLSSFeatureDx12::Init nvngx.dll not loaded!");

		SetInit(false);
		return false;
	}

	HRESULT result;
	NVSDK_NGX_Result nvResult;
	bool initResult = false;

	do
	{
		if (!_dlssInited)
		{
			NVSDK_NGX_FeatureCommonInfo fcInfo{};

			GetFeatureCommonInfo(&fcInfo);

			if (Config::Instance()->NVNGX_ProjectId != "" && _Init_with_ProjectID != nullptr)
			{
				spdlog::debug("DLSSFeatureDx12::Init _Init_with_ProjectID!");

				nvResult = _Init_with_ProjectID(Config::Instance()->NVNGX_ProjectId.c_str(), Config::Instance()->NVNGX_Engine, Config::Instance()->NVNGX_EngineVersion.c_str(),
					Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else if (_Init_Ext != nullptr)
			{
				spdlog::debug("DLSSFeatureDx12::Init _Init_Ext!");

				nvResult = _Init_Ext(Config::Instance()->NVNGX_ApplicationId, Config::Instance()->NVNGX_ApplicationDataPath.c_str(), InDevice, Config::Instance()->NVNGX_Version, &fcInfo);
			}
			else
			{
				spdlog::error("DLSSFeatureDx12::Init _Init_with_ProjectID and  is null");
				break;
			}

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Init _Init_with_ProjectID result: {0:X}", (unsigned int)nvResult);
				break;
			}

			_dlssInited = true;
		}

		if (_AllocateParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureDx12::Init _AllocateParameters will be used");

			nvResult = _AllocateParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Init _AllocateParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else if (_GetParameters != nullptr)
		{
			spdlog::debug("DLSSFeatureDx12::Init _GetParameters will be used");

			nvResult = _GetParameters(&Parameters);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				spdlog::error("DLSSFeatureDx12::Init _GetParameters result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else
		{
			spdlog::error("DLSSFeatureDx12::Init _AllocateParameters and _GetParameters are both nullptr!");
			break;
		}

		spdlog::info("DLSSFeatureDx12::Evaluate Creating DLSS feature");

		if (_CreateFeature != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = _CreateFeature(InCommandList, NVSDK_NGX_Feature_SuperSampling, Parameters, &_p_dlssHandle);

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

		initResult = true;

	} while (false);

	if (initResult)
	{
		if (Imgui == nullptr || Imgui.get() == nullptr)
			Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), InDevice);

		OUT_DS = std::make_unique<DS_Dx12>("Output Downsample", InDevice);
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

	NVSDK_NGX_Result nvResult;
	ID3D12Resource* paramOutput;

	if (_EvaluateFeature != nullptr)
	{
		ProcessEvaluateParams(InParameters);

		nvResult = _EvaluateFeature(InCommandList, _p_dlssHandle, Parameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			spdlog::error("DLSSFeatureDx12::Evaluate _EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}
	}
	else
	{
		spdlog::error("DLSSFeatureDx12::Evaluate _EvaluateFeature is nullptr");
		return false;
	}

	// imgui
	if (_frameCount > 20 && Parameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) == NVSDK_NGX_Result_Success)
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

	_frameCount++;

	return true;
}

void DLSSFeatureDx12::Shutdown(ID3D12Device* InDevice)
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

DLSSFeatureDx12::DLSSFeatureDx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx12(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (!_moduleLoaded)
		return;

	if (_Init_with_ProjectID == nullptr)
		_Init_with_ProjectID = (PFN_NVSDK_NGX_D3D12_Init_ProjectID)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_Init_ProjectID");

	if (_Init_Ext == nullptr)
		_Init_Ext = (PFN_NVSDK_NGX_D3D12_Init_Ext)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_Init_Ext");

	if (_Shutdown == nullptr)
		_Shutdown = (PFN_NVSDK_NGX_D3D12_Shutdown)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_Shutdown");

	if (_Shutdown1 == nullptr)
		_Shutdown1 = (PFN_NVSDK_NGX_D3D12_Shutdown1)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_Shutdown1");

	if (_GetParameters == nullptr)
		_GetParameters = (PFN_NVSDK_NGX_D3D12_GetParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_GetParameters");

	if (_AllocateParameters == nullptr)
		_AllocateParameters = (PFN_NVSDK_NGX_D3D12_AllocateParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_AllocateParameters");

	if (_DestroyParameters == nullptr)
		_DestroyParameters = (PFN_NVSDK_NGX_D3D12_DestroyParameters)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_DestroyParameters");

	if (_CreateFeature == nullptr)
		_CreateFeature = (PFN_NVSDK_NGX_D3D12_CreateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_CreateFeature");

	if (_ReleaseFeature == nullptr)
		_ReleaseFeature = (PFN_NVSDK_NGX_D3D12_ReleaseFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_ReleaseFeature");

	if (_EvaluateFeature == nullptr)
		_EvaluateFeature = (PFN_NVSDK_NGX_D3D12_EvaluateFeature)GetProcAddress(NVNGX(), "NVSDK_NGX_D3D12_EvaluateFeature");

	_moduleLoaded = (_Init_with_ProjectID != nullptr || _Init_Ext != nullptr) && (_Shutdown != nullptr || _Shutdown1 != nullptr) &&
		(_GetParameters != nullptr || _AllocateParameters != nullptr) && _DestroyParameters != nullptr && _CreateFeature != nullptr &&
		_ReleaseFeature != nullptr && _EvaluateFeature != nullptr;

	spdlog::info("DLSSFeatureDx12::DLSSFeatureDx12 binding complete!");
}

DLSSFeatureDx12::~DLSSFeatureDx12()
{
	if (Parameters != nullptr && _DestroyParameters != nullptr)
		_DestroyParameters(Parameters);

	if (_ReleaseFeature != nullptr)
		_ReleaseFeature(_p_dlssHandle);
}
