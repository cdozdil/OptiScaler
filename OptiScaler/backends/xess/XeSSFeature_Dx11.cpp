#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx11.h"

#if _DEBUG
#include "../../d3dx/D3DX11tex.h"
#endif

XeSSFeatureDx11::XeSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : IFeature_Dx11wDx12(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters), XeSSFeature(InHandleId, InParameters)
{
}

bool XeSSFeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Init!");

	if (IsInited())
		return true;

	Device = InDevice;
	DeviceContext = InContext;

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &_initFlags) == NVSDK_NGX_Result_Success)
		_initFlagsReady = true;

	_baseInit = false;	

	return true;
}

bool XeSSFeatureDx11::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Evaluate");

	if (!_baseInit)
	{
		// to prevent creation dx12 device if we are going to recreate feature
		if (!Config::Instance()->DisplayResolution.has_value())
		{
			ID3D11Resource* paramVelocity = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &paramVelocity) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&paramVelocity);

			if (paramVelocity != nullptr)
			{
				int featureFlags;
				InParameters->Get(NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags, &featureFlags);

				D3D11_TEXTURE2D_DESC desc;
				ID3D11Texture2D* pvTexture;
				paramVelocity->QueryInterface(IID_PPV_ARGS(&pvTexture));
				pvTexture->GetDesc(&desc);
				bool lowResMV = desc.Width < TargetWidth();
				bool displaySizeEnabled = (GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0;

				if (displaySizeEnabled && lowResMV)
				{
					spdlog::warn("XeSSFeatureDx11::Evaluate MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, DisplayWidth(), displaySizeEnabled);
					Config::Instance()->DisplayResolution = false;
				}
			}
		}

		if (!Config::Instance()->AutoExposure.has_value())
		{
			ID3D11Resource* paramExpo = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &paramExpo) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&paramExpo);

			if (paramExpo == nullptr)
			{
				spdlog::warn("XeSSFeatureDx11::Evaluate ExposureTexture is not exist, enabling AutoExposure!!");
				Config::Instance()->AutoExposure = true;
			}
		}

		if (!Config::Instance()->DisableReactiveMask.has_value())
		{
			ID3D11Resource* paramRM = nullptr;
			if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &paramRM) != NVSDK_NGX_Result_Success)
				InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&paramRM);

			if (paramRM == nullptr)
			{
				spdlog::warn("XeSSFeatureDx11::Evaluate Bias mask not exist, enabling DisableReactiveMask!!");
				Config::Instance()->DisableReactiveMask = true;
			}
		}

		if (!BaseInit(Device, InDeviceContext, InParameters))
		{
			spdlog::error("XeSSFeatureDx11::Evaluate BaseInit failed!");
			return false;
		}

		_baseInit = true;

		if (Dx12Device == nullptr)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Dx12on11Device is null!");
			return false;
		}

		spdlog::debug("XeSSFeatureDx11::Evaluate calling InitXeSS");

		if (!InitXeSS(Dx12Device, InParameters))
		{
			spdlog::error("XeSSFeatureDx11::Evaluate InitXeSS fail!");
			return false;
		}

		if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
		{
			spdlog::debug("XeSSFeatureDx11::Evaluate Create Imgui!");
			Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
		}

		if (Config::Instance()->Dx11DelayedInit.value_or(false))
		{
			spdlog::trace("XeSSFeatureDx11::Evaluate sleeping after XeSSContext creation for 1500ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		}

		OutputScaler = std::make_unique<BS_Dx12>("Output Downsample", Dx12Device, (TargetWidth() < DisplayWidth()));
		RCAS = std::make_unique<RCAS_Dx12>("RCAS", Dx12Device);
	}

	if (!IsInited() || !_xessContext || !ModuleLoaded())
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Not inited!");
		return false;
	}

	ID3D11DeviceContext4* dc;
	auto result = InDeviceContext->QueryInterface(IID_PPV_ARGS(&dc));

	if (result != S_OK)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate CreateFence d3d12fence error: {0:x}", result);
		return false;
	}

	if (dc != Dx11DeviceContext)
	{
		spdlog::warn("XeSSFeatureDx11::Evaluate Dx11DeviceContext changed!");
		ReleaseSharedResources();
		Dx11DeviceContext = dc;
	}

	if (Config::Instance()->xessDebug)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate xessDebug");

		xess_dump_parameters_t dumpParams{};
		dumpParams.frame_count = Config::Instance()->xessDebugFrames;
		dumpParams.frame_idx = dumpCount;
		dumpParams.path = ".";
		dumpParams.dump_elements_mask = XESS_DUMP_INPUT_COLOR | XESS_DUMP_INPUT_VELOCITY | XESS_DUMP_INPUT_DEPTH | XESS_DUMP_OUTPUT | XESS_DUMP_EXECUTION_PARAMETERS | XESS_DUMP_HISTORY | XESS_DUMP_INPUT_RESPONSIVE_PIXEL_MASK;

		if (!Config::Instance()->DisableReactiveMask.value_or(true))
			dumpParams.dump_elements_mask |= XESS_DUMP_INPUT_RESPONSIVE_PIXEL_MASK;

		StartDump()(_xessContext, &dumpParams);
		Config::Instance()->xessDebug = false;
		dumpCount += Config::Instance()->xessDebugFrames;
	}

	// creatimg params for XeSS
	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
		params.exposureScale = 1.0f;

	InParameters->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

	GetRenderResolution(InParameters, &params.inputWidth, &params.inputHeight);

	auto sharpness = GetSharpness(InParameters);

	bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

	spdlog::debug("XeSSFeatureDx11::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	if (!ProcessDx11Textures(InParameters))
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Can't process Dx11 textures!");

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}
	else
		spdlog::debug("XeSSFeatureDx11::Evaluate ProcessDx11Textures complete!");

	// AutoExposure or ReactiveMask is nullptr
	if (Config::Instance()->changeBackend)
	{
		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return true;
	}

	params.pColorTexture = dx11Color.Dx12Resource;

	_hasColor = params.pColorTexture != nullptr;
	params.pVelocityTexture = dx11Mv.Dx12Resource;
	_hasMV = params.pVelocityTexture != nullptr;

	if (useSS)
	{
		OutputScaler->Scale = (float)TargetWidth() / (float)DisplayWidth();

		if (OutputScaler->CreateBufferResource(Dx12Device, dx11Out.Dx12Resource, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			OutputScaler->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			params.pOutputTexture = OutputScaler->Buffer();
		}
		else
			params.pOutputTexture = dx11Out.Dx12Resource;
	}
	else
	{
		params.pOutputTexture = dx11Out.Dx12Resource;
	}

	// RCAS
	if (!Config::Instance()->changeRCAS &&
		Config::Instance()->RcasEnabled.value_or(true) &&
		(sharpness > 0.0f || Config::Instance()->MotionSharpnessEnabled.value_or(false)) &&
		RCAS != nullptr && RCAS.get() != nullptr && RCAS->IsInit() &&
		RCAS->CreateBufferResource(Dx12Device, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
	{
		RCAS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		params.pOutputTexture = RCAS->Buffer();
	}

	_hasOutput = params.pOutputTexture != nullptr;
	params.pDepthTexture = dx11Depth.Dx12Resource;
	_hasDepth = params.pDepthTexture != nullptr;
	params.pExposureScaleTexture = dx11Exp.Dx12Resource;
	_hasExposure = params.pExposureScaleTexture != nullptr;
	params.pResponsivePixelMaskTexture = dx11Tm.Dx12Resource;
	_hasTM = params.pResponsivePixelMaskTexture != nullptr;

	spdlog::debug("XeSSFeatureDx11::Evaluate Textures -> params complete!");

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		xessResult = SetVelocityScale()(_xessContext, MVScaleX, MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate xessSetVelocityScale error: {0}", ResultToString(xessResult));

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return false;
		}
	}
	else
		spdlog::warn("XeSSFeatureDx11::XeSSExecuteDx12 Can't get motion vector scales!");

	// Execute xess
	spdlog::debug("XeSSFeatureDx11::Evaluate Executing!!");
	xessResult = D3D12Execute()(_xessContext, Dx12CommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));

		Dx12CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
		Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}

	// apply rcas
	if (!Config::Instance()->changeRCAS && Config::Instance()->RcasEnabled.value_or(true) && 
		(sharpness > 0.0f || Config::Instance()->MotionSharpnessEnabled.value_or(false)) && 
		RCAS != nullptr && RCAS.get() != nullptr && RCAS->CanRender())
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate Apply CAS");

		if (params.pOutputTexture != RCAS->Buffer())
			ResourceBarrier(Dx12CommandList, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		RCAS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		RcasConstants rcasConstants{};
		rcasConstants.Sharpness = sharpness;
		rcasConstants.DisplayWidth = DisplayWidth();
		rcasConstants.DisplayHeight = DisplayHeight();
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &rcasConstants.MvScaleX);
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &rcasConstants.MvScaleY);
		rcasConstants.DisplaySizeMV = !(GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes);
		rcasConstants.RenderHeight = RenderHeight();
		rcasConstants.RenderWidth = RenderWidth();

		if (useSS)
		{
			if (!RCAS->Dispatch(Dx12Device, Dx12CommandList, params.pOutputTexture, params.pVelocityTexture, rcasConstants, OutputScaler->Buffer()))
			{
				Config::Instance()->RcasEnabled = false;

				Dx12CommandList->Close();
				ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
				Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

				Dx12CommandAllocator->Reset();
				Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

				return true;
			}
		}
		else
		{
			if (!RCAS->Dispatch(Dx12Device, Dx12CommandList, params.pOutputTexture, params.pVelocityTexture, rcasConstants, dx11Out.Dx12Resource))
			{
				Config::Instance()->RcasEnabled = false;

				Dx12CommandList->Close();
				ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
				Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

				Dx12CommandAllocator->Reset();
				Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

				return true;
			}
		}
	}

	if (useSS)
	{
		spdlog::debug("XeSSFeatureDx11::Evaluate downscaling output...");
		OutputScaler->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OutputScaler->Dispatch(Dx12Device, Dx12CommandList, OutputScaler->Buffer(), dx11Out.Dx12Resource))
		{
			Config::Instance()->OutputScalingEnabled = false;
			Config::Instance()->changeBackend = true;

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return true;
		}
	}

	if (!Config::Instance()->SyncAfterDx12.value_or(true))
	{
		if (!CopyBackOutput())
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Can't copy output texture back!");

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return false;
		}

		// imgui
		if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
		{
			if (Imgui != nullptr && Imgui.get() != nullptr)
			{
				spdlog::debug("XeSSFeatureDx11::Evaluate Apply Imgui");

				if (Imgui->IsHandleDifferent())
				{
					spdlog::debug("XeSSFeatureDx11::Evaluate Imgui handle different, reset()!");
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					Imgui.reset();
				}
				else
					Imgui->Render(InDeviceContext, paramOutput[_frameCount % 2]);
			}
			else
			{
				if (Imgui == nullptr || Imgui.get() == nullptr)
					Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
			}
		}
	}

	// Execute dx12 commands to process xess
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	if (Config::Instance()->SyncAfterDx12.value_or(true))
	{
		if (!CopyBackOutput())
		{
			spdlog::error("XeSSFeatureDx11::Evaluate Can't copy output texture back!");

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return false;
		}

		// imgui
		if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
		{
			if (Imgui != nullptr && Imgui.get() != nullptr)
			{
				spdlog::debug("XeSSFeatureDx11::Evaluate Apply Imgui");

				if (Imgui->IsHandleDifferent())
				{
					spdlog::debug("XeSSFeatureDx11::Evaluate Imgui handle different, reset()!");
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					Imgui.reset();
				}
				else
					Imgui->Render(InDeviceContext, paramOutput[_frameCount % 2]);
			}
			else
			{
				if (Imgui == nullptr || Imgui.get() == nullptr)
					Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
			}
		}
	}

	_frameCount++;

	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

XeSSFeatureDx11::~XeSSFeatureDx11()
{
	if (RCAS != nullptr && RCAS.get() != nullptr)
		RCAS.reset();

	if (_outBuffer != nullptr)
	{
		_outBuffer->Release();
		_outBuffer = nullptr;
	}
}

