#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx12.h"

bool XeSSFeatureDx12::Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx12::Init");

	if (IsInited())
		return true;

	Device = InDevice;

	if (InitXeSS(InDevice, InParameters))
	{
		if (Imgui == nullptr || Imgui.get() == nullptr)
			Imgui = std::make_unique<Imgui_Dx12>(GetForegroundWindow(), InDevice);

		OUT_DS = std::make_unique<DS_Dx12>("Output Downsample", InDevice, (TargetWidth() < DisplayWidth()));

		return true;
	}

	return false;
}

bool XeSSFeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx12::Evaluate");

	if (!IsInited() || !_xessContext || !ModuleLoaded())
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Not inited!");
		return false;
	}

	if (Config::Instance()->xessDebug)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessDebug");

		xess_dump_parameters_t dumpParams{};
		dumpParams.frame_count = Config::Instance()->xessDebugFrames;
		dumpParams.frame_idx = dumpCount;
		dumpParams.path = ".";
		dumpParams.dump_elements_mask = XESS_DUMP_INPUT_COLOR | XESS_DUMP_INPUT_VELOCITY | XESS_DUMP_INPUT_DEPTH | XESS_DUMP_OUTPUT | XESS_DUMP_EXECUTION_PARAMETERS | XESS_DUMP_HISTORY;

		if (!Config::Instance()->DisableReactiveMask.value_or(true))
			dumpParams.dump_elements_mask |= XESS_DUMP_INPUT_RESPONSIVE_PIXEL_MASK;

		StartDump()(_xessContext, &dumpParams);
		Config::Instance()->xessDebug = false;
		dumpCount += Config::Instance()->xessDebugFrames;
	}

	//if (Config::Instance()->changeCAS)
	//{
	//	if (CAS != nullptr && CAS.get() != nullptr)
	//	{
	//		spdlog::trace("XeSSFeatureDx12::Evaluate sleeping before CAS.reset() for 250ms");
	//		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	//		CAS.reset();
	//	}
	//	else
	//	{
	//		Config::Instance()->changeCAS = false;
	//		spdlog::trace("XeSSFeatureDx12::Evaluate sleeping before CAS creation for 250ms");
	//		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	//		CAS = std::make_unique<CAS_Dx12>(Device, TargetWidth(), TargetHeight(), Config::Instance()->CasColorSpaceConversion.value_or(0));
	//	}
	//}

	if (Config::Instance()->changeCAS)
	{
		if (RCAS != nullptr && RCAS.get() != nullptr)
		{
			spdlog::trace("XeSSFeatureDx12::Evaluate sleeping before RCAS.reset() for 250ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			RCAS.reset();
		}
		else
		{
			Config::Instance()->changeCAS = false;
			spdlog::trace("XeSSFeatureDx12::Evaluate sleeping before CAS creation for 250ms");
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			RCAS = std::make_unique<RCAS_Dx12>("RCAS", Device);
		}
	}

	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
		params.exposureScale = 1.0f;

	InParameters->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

	GetRenderResolution(InParameters, &params.inputWidth, &params.inputHeight);

	auto sharpness = GetSharpness(InParameters);

	float ssMulti = Config::Instance()->OutputScalingMultiplier.value_or(1.5f);

	bool useSS = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(false);

	spdlog::debug("XeSSFeatureDx12::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	ID3D12Resource* paramColor;
	if (InParameters->Get(NVSDK_NGX_Parameter_Color, &paramColor) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Color, (void**)&paramColor);

	if (paramColor)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Color exist..");
		paramColor->SetName(L"paramColor");

		if (Config::Instance()->ColorResourceBarrier.has_value())
		{
			ResourceBarrier(InCommandList, paramColor, (D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
		{
			Config::Instance()->ColorResourceBarrier = (int)D3D12_RESOURCE_STATE_RENDER_TARGET;
			ResourceBarrier(InCommandList, paramColor, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		params.pColorTexture = paramColor;
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Color not exist!!");
		return false;
	}

	if (InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, &params.pVelocityTexture) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_MotionVectors, (void**)&params.pVelocityTexture);

	if (params.pVelocityTexture)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate MotionVectors exist..");
		params.pVelocityTexture->SetName(L"pVelocityTexture");

		if (Config::Instance()->MVResourceBarrier.has_value())
		{
			ResourceBarrier(InCommandList, params.pVelocityTexture, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
		{
			Config::Instance()->MVResourceBarrier = (int)D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			ResourceBarrier(InCommandList, params.pVelocityTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		if (!Config::Instance()->DisplayResolution.has_value())
		{
			auto desc = params.pVelocityTexture->GetDesc();
			bool lowResMV = desc.Width < DisplayWidth();
			bool displaySizeEnabled = (GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes) == 0;

			if (displaySizeEnabled && lowResMV)
			{
				spdlog::warn("XeSSFeatureDx12::Evaluate MotionVectors MVWidth: {0}, DisplayWidth: {1}, Flag: {2} Disabling DisplaySizeMV!!", desc.Width, DisplayWidth(), displaySizeEnabled);
				Config::Instance()->DisplayResolution = false;
				Config::Instance()->changeBackend = true;
				return true;
			}
		}
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate MotionVectors not exist!!");
		return false;
	}

	ID3D12Resource* paramOutput;

	if (InParameters->Get(NVSDK_NGX_Parameter_Output, &paramOutput) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Output, (void**)&paramOutput);

	if (paramOutput)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Output exist..");
		paramOutput->SetName(L"paramOutput");

		if (Config::Instance()->OutputResourceBarrier.has_value())
		{
			ResourceBarrier(InCommandList, paramOutput, (D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

		if (useSS)
		{
			OUT_DS->Scale = (float)TargetWidth() / (float)DisplayWidth();

			if (OUT_DS->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				OUT_DS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				params.pOutputTexture = OUT_DS->Buffer();
			}
			else
				params.pOutputTexture = paramOutput;
		}
		else
			params.pOutputTexture = paramOutput;

		if (!Config::Instance()->changeCAS && Config::Instance()->CasEnabled.value_or(true) && sharpness > 0.0f &&
			RCAS != nullptr && RCAS.get() != nullptr &&
			RCAS->CreateBufferResource(Device, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			params.pOutputTexture = RCAS->Buffer();
		}
	}
	else
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Output not exist!!");
		return false;
	}

	if (InParameters->Get(NVSDK_NGX_Parameter_Depth, &params.pDepthTexture) != NVSDK_NGX_Result_Success)
		InParameters->Get(NVSDK_NGX_Parameter_Depth, (void**)&params.pDepthTexture);

	if (params.pDepthTexture)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate Depth exist..");
		params.pDepthTexture->SetName(L"params.pDepthTexture");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(InCommandList, params.pDepthTexture, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("XeSSFeatureDx12::Evaluate Depth not exist!!");
		else
			spdlog::info("XeSSFeatureDx12::Evaluate Using high res motion vectors, depth is not needed!!");

		params.pDepthTexture = nullptr;
	}

	if (!Config::Instance()->AutoExposure.value_or(false))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, &params.pExposureScaleTexture) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_ExposureTexture, (void**)&params.pExposureScaleTexture);

		if (params.pExposureScaleTexture)
		{
			spdlog::debug("XeSSFeatureDx12::Evaluate ExposureTexture exist..");

			if (Config::Instance()->ExposureResourceBarrier.has_value())
				ResourceBarrier(InCommandList, params.pExposureScaleTexture, (D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else
		{
			spdlog::warn("XeSSFeatureDx12::Evaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
			Config::Instance()->AutoExposure = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}
	else
		spdlog::debug("XeSSFeatureDx12::Evaluate AutoExposure enabled!");

	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, &params.pResponsivePixelMaskTexture) != NVSDK_NGX_Result_Success)
			InParameters->Get(NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask, (void**)&params.pResponsivePixelMaskTexture);

		if (params.pResponsivePixelMaskTexture)
		{
			spdlog::debug("XeSSFeatureDx12::Evaluate Bias mask exist..");

			if (Config::Instance()->MaskResourceBarrier.has_value())
				ResourceBarrier(InCommandList, params.pResponsivePixelMaskTexture, (D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else
		{
			spdlog::warn("XeSSFeatureDx12::Evaluate Bias mask not exist and its enabled in config, it may cause problems!!");
			Config::Instance()->DisableReactiveMask = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}

	_hasColor = params.pColorTexture != nullptr;
	_hasMV = params.pVelocityTexture != nullptr;
	_hasOutput = params.pOutputTexture != nullptr;
	_hasDepth = params.pDepthTexture != nullptr;
	_hasExposure = params.pExposureScaleTexture != nullptr;
	_hasTM = params.pResponsivePixelMaskTexture != nullptr;

	float MVScaleX = 1.0f;
	float MVScaleY = 1.0f;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		xessResult = SetVelocityScale()(_xessContext, MVScaleX, MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeatureDx12::Evaluate xessSetVelocityScale: {0}", ResultToString(xessResult));
			return false;
		}
	}
	else
		spdlog::warn("XeSSFeatureDx12::Evaluate Can't get motion vector scales!");

	spdlog::debug("XeSSFeatureDx12::Evaluate Executing!!");
	xessResult = D3D12Execute()(_xessContext, InCommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	//apply cas
	if (!Config::Instance()->changeCAS && Config::Instance()->CasEnabled.value_or(true) && sharpness > 0.0f && RCAS != nullptr && RCAS.get() != nullptr && RCAS->Buffer() != nullptr)
	{
		ResourceBarrier(InCommandList, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		RCAS->Sharpness = sharpness;

		if (useSS)
		{
			if (!RCAS->Dispatch(Device, InCommandList, params.pOutputTexture, OUT_DS->Buffer()))
			{
				Config::Instance()->CasEnabled = false;
				return true;
			}
		}
		else
		{
			if (!RCAS->Dispatch(Device, InCommandList, params.pOutputTexture, paramOutput))
			{
				Config::Instance()->CasEnabled = false;
				return true;
			}
		}
	}

	//if (!Config::Instance()->changeCAS && Config::Instance()->CasEnabled.value_or(false) && sharpness > 0.0f && CAS != nullptr && CAS.get() != nullptr && CAS->Buffer() != nullptr)
	//{
	//	ResourceBarrier(InCommandList, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//	CAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//	if (useSS)
	//	{
	//		if (!CAS->Dispatch(InCommandList, sharpness, params.pOutputTexture, OUT_DS->Buffer()))
	//		{
	//			Config::Instance()->CasEnabled = false;
	//			return true;
	//		}
	//	}
	//	else
	//	{
	//		if (!CAS->Dispatch(InCommandList, sharpness, params.pOutputTexture, paramOutput))
	//		{
	//			Config::Instance()->CasEnabled = false;
	//			return true;
	//		}
	//	}
	//}

	if (useSS)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate downscaling output...");
		OUT_DS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OUT_DS->Dispatch(Device, InCommandList, OUT_DS->Buffer(), paramOutput))
		{
			Config::Instance()->OutputScalingEnabled = false;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}

	// imgui
	if (_frameCount > 20)
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

	// restore resource states
	if (params.pColorTexture && Config::Instance()->ColorResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pColorTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	if (params.pVelocityTexture && Config::Instance()->MVResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pVelocityTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramOutput,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	if (params.pDepthTexture && Config::Instance()->DepthResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pDepthTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	if (params.pExposureScaleTexture && Config::Instance()->ExposureResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pExposureScaleTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	if (params.pResponsivePixelMaskTexture && Config::Instance()->MaskResourceBarrier.has_value())
		ResourceBarrier(InCommandList, params.pResponsivePixelMaskTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());

	_frameCount++;

	return true;
}

XeSSFeatureDx12::~XeSSFeatureDx12() = default;
