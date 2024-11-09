#include "DLSSFeature_Dx11.h"

#include <Config.h>

#include <dxgi.h>

bool DLSSFeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, NVSDK_NGX_Parameter* InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		LOG_ERROR("nvngx.dll not loaded!");

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

			_moduleLoaded = (NVNGXProxy::D3D11_Init_ProjectID() != nullptr || NVNGXProxy::D3D11_Init_Ext() != nullptr) &&
				(NVNGXProxy::D3D11_Shutdown() != nullptr || NVNGXProxy::D3D11_Shutdown1() != nullptr) &&
				(NVNGXProxy::D3D11_GetParameters() != nullptr || NVNGXProxy::D3D11_AllocateParameters() != nullptr) && NVNGXProxy::D3D11_DestroyParameters() != nullptr &&
				NVNGXProxy::D3D11_CreateFeature() != nullptr && NVNGXProxy::D3D11_ReleaseFeature() != nullptr && NVNGXProxy::D3D11_EvaluateFeature() != nullptr;

			//delay between init and create feature
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		LOG_INFO("Creating DLSS feature");

		if (NVNGXProxy::D3D11_CreateFeature() != nullptr)
		{
			ProcessInitParams(InParameters);

			_p_dlssHandle = &_dlssHandle;
			nvResult = NVNGXProxy::D3D11_CreateFeature()(InContext, NVSDK_NGX_Feature_SuperSampling, InParameters, &_p_dlssHandle);

			if (nvResult != NVSDK_NGX_Result_Success)
			{
				LOG_ERROR("_CreateFeature result: {0:X}", (unsigned int)nvResult);
				break;
			}
		}
		else
		{
			LOG_ERROR("_CreateFeature is nullptr");
			break;
		}

		ReadVersion();

		initResult = true;

	} while (false);

	if (initResult)
	{
		if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
			Imgui = std::make_unique<OldMenuDx11>(GetForegroundWindow(), InDevice);
	
		OutputScaler = std::make_unique<OS_Dx11>("Output Scaling", InDevice, (TargetWidth() < DisplayWidth()));
		RCAS = std::make_unique<RCAS_Dx11>("RCAS", InDevice);
	}

	SetInit(initResult);

	return initResult;
}

bool DLSSFeatureDx11::Evaluate(ID3D11DeviceContext* InDeviceContext, NVSDK_NGX_Parameter* InParameters)
{
	if (!_moduleLoaded)
	{
		LOG_ERROR("nvngx.dll or _nvngx.dll is not loaded!");
		return false;
	}

	NVSDK_NGX_Result nvResult;

	bool rcasEnabled = (Version().major > 2 || (Version().major == 2 && Version().minor >= 5 && Version().patch >= 1));

	if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) && (RCAS == nullptr || RCAS.get() == nullptr || !RCAS->IsInit()))
		Config::Instance()->RcasEnabled = false;

	if (!OutputScaler->IsInit())
		Config::Instance()->OutputScalingEnabled = false;

	if (NVNGXProxy::D3D11_EvaluateFeature() != nullptr)
	{
		ProcessEvaluateParams(InParameters);
		
		ID3D11Resource* paramOutput = nullptr;
		ID3D11Resource* paramMotion = nullptr;
		ID3D11Resource* setBuffer = nullptr;

		bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

		InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput);
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramMotion);

		// supersampling
		if (useSS)
		{
			if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight()))
			{
				setBuffer = OutputScaler->Buffer();
			}
			else
				setBuffer = paramOutput;
		}
		else
			setBuffer = paramOutput;

		// RCAS sharpness & preperation
		_sharpness = GetSharpness(InParameters);

		if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) &&
			(_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
			RCAS->IsInit() && RCAS->CreateBufferResource(Device, setBuffer))
		{
			// Disable DLSS sharpness
			InParameters->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);
			setBuffer = RCAS->Buffer();
		}

		InParameters->Set(NVSDK_NGX_Parameter_Output, setBuffer);

		nvResult = NVNGXProxy::D3D11_EvaluateFeature()(InDeviceContext, _p_dlssHandle, InParameters, NULL);

		if (nvResult != NVSDK_NGX_Result_Success)
		{
			LOG_ERROR("_EvaluateFeature result: {0:X}", (unsigned int)nvResult);
			return false;
		}

		LOG_TRACE("_EvaluateFeature ok!");

		// Apply CAS
		if (Config::Instance()->RcasEnabled.value_or(rcasEnabled) &&
			(_sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
			RCAS->CanRender())
		{
			RcasConstants rcasConstants{};

			rcasConstants.Sharpness = _sharpness;
			rcasConstants.DisplayWidth = TargetWidth();
			rcasConstants.DisplayHeight = TargetHeight();
			InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
			InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
			rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
			rcasConstants.RenderHeight = RenderHeight();
			rcasConstants.RenderWidth = RenderWidth();

			if (useSS)
			{
				if (!RCAS->Dispatch(Device, InDeviceContext, (ID3D11Texture2D*)setBuffer, (ID3D11Texture2D*)paramMotion, rcasConstants, OutputScaler->Buffer()))
				{
					Config::Instance()->RcasEnabled = false;
					return true;
				}
			}
			else
			{
				if (!RCAS->Dispatch(Device, InDeviceContext, (ID3D11Texture2D*)setBuffer, (ID3D11Texture2D*)paramMotion, rcasConstants, (ID3D11Texture2D*)paramOutput))
				{
					Config::Instance()->RcasEnabled = false;
					return true;
				}
			}
		}

		// Downsampling
		if (useSS)
		{
			LOG_DEBUG("downscaling output...");
			
			if (!OutputScaler->Dispatch(Device, InDeviceContext, OutputScaler->Buffer(), (ID3D11Texture2D*)paramOutput))
			{
				Config::Instance()->OutputScalingEnabled = false;
				Config::Instance()->changeBackend = true;
				return true;
			}
		}

		// imgui
		if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30 && paramOutput != nullptr)
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
					Imgui = std::make_unique<OldMenuDx11>(Util::GetProcessWindow(), Device);
			}
		}

		// set original output texture back
		InParameters->Set(NVSDK_NGX_Parameter_Output, paramOutput);
	}
	else
	{
		LOG_ERROR("_EvaluateFeature is nullptr");
		return false;
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

DLSSFeatureDx11::DLSSFeatureDx11(unsigned int InHandleId, NVSDK_NGX_Parameter* InParameters) : IFeature(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), DLSSFeature(InHandleId, InParameters)
{
	if (NVNGXProxy::NVNGXModule() == nullptr)
	{
		LOG_INFO("nvngx.dll not loaded, now loading");
		NVNGXProxy::InitNVNGX();
	}

	LOG_INFO("binding complete!");
}

DLSSFeatureDx11::~DLSSFeatureDx11()
{
	if (NVNGXProxy::D3D11_ReleaseFeature() != nullptr && _p_dlssHandle != nullptr)
		NVNGXProxy::D3D11_ReleaseFeature()(_p_dlssHandle);
}
