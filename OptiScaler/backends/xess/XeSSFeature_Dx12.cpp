#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx12.h"

bool XeSSFeatureDx12::Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList)
{
	spdlog::debug("XeSSFeatureDx12::Init");

	if (IsInited())
		return true;

	Device = InDevice;

	if (InitXeSS(InDevice, _createParams))
	{
		if (!Config::Instance()->OverlayMenu.value_or(true) && (Imgui == nullptr || Imgui.get() == nullptr))
			Imgui = std::make_unique<Imgui_Dx12>(Util::GetProcessWindow(), InDevice);

		OutputScaler = std::make_unique<BS_Dx12>("Output Downsample", InDevice, (TargetWidth() < DisplayWidth()));
		RCAS = std::make_unique<RCAS_Dx12>("RCAS", InDevice);

		return true;
	}

	return false;
}

bool XeSSFeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams)
{
	spdlog::debug("XeSSFeatureDx12::Evaluate");

	if (!IsInited() || !_xessContext || !ModuleLoaded())
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Not inited!");
		return false;
	}

	if (!RCAS->IsInit())
		Config::Instance()->RcasEnabled = false;

	if (!OutputScaler->IsInit())
		Config::Instance()->OutputScalingEnabled = false;

	if (Config::Instance()->xessDebug)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessDebug");

		xess_dump_parameters_t dumpParams{};
		dumpParams.frame_count = Config::Instance()->xessDebugFrames;
		dumpParams.frame_idx = dumpCount;
		dumpParams.path = ".";
		dumpParams.dump_elements_mask = XESS_DUMP_INPUT_COLOR | XESS_DUMP_INPUT_VELOCITY | XESS_DUMP_INPUT_DEPTH | 
										XESS_DUMP_OUTPUT | XESS_DUMP_EXECUTION_PARAMETERS | XESS_DUMP_HISTORY;

		if (!Config::Instance()->DisableReactiveMask.value_or(true))
			dumpParams.dump_elements_mask |= XESS_DUMP_INPUT_RESPONSIVE_PIXEL_MASK;

		StartDump()(_xessContext, &dumpParams);
		Config::Instance()->xessDebug = false;
		dumpCount += Config::Instance()->xessDebugFrames;
	}

	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	params.jitterOffsetX = InParams->JitterOffsetX();
	params.jitterOffsetY = InParams->JitterOffsetY();
	params.exposureScale = InParams->ExposureScale();
	params.resetHistory = InParams->Reset() ? 1 : 0;

	if (InParams->RenderWidth() > 0)
		params.inputWidth = InParams->RenderWidth();
	else
		params.inputWidth = RenderWidth();

	if (InParams->RenderHeight() > 0)
		params.inputHeight = InParams->RenderHeight();
	else
		params.inputHeight = RenderHeight();

	spdlog::debug("XeSSFeatureDx12::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	auto sharpness = InParams->Sharpness();

	params.pOutputTexture = (ID3D12Resource*)InParams->OutputColor();
	auto beforeResult = BeforeEvaluate(InCommandList, InParams, params.pOutputTexture);

	// Autoexposure Reset
	if (Config::Instance()->changeBackend)
		return true;

	params.pColorTexture = (ID3D12Resource*)InParams->InputColor();
	params.pDepthTexture = (ID3D12Resource*)InParams->InputDepth();
	params.pExposureScaleTexture = (ID3D12Resource*)InParams->InputExposure();
	params.pVelocityTexture = (ID3D12Resource*)InParams->InputMotion();
	params.pResponsivePixelMaskTexture = (ID3D12Resource*)InParams->InputMask();

	xessResult = SetVelocityScale()(_xessContext, InParams->MvScaleX(), InParams->MvScaleY());

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessSetVelocityScale: {0}", ResultToString(xessResult));
		return false;
	}

	spdlog::debug("XeSSFeatureDx12::Evaluate Executing!!");
	xessResult = D3D12Execute()(_xessContext, InCommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	// apply rcas
	if (Config::Instance()->RcasEnabled.value_or(true) && 
		(sharpness > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
		RCAS->CanRender())
	{
		if (params.pOutputTexture != RCAS->Buffer())
			ResourceBarrier(InCommandList, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

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
			if (!RCAS->Dispatch(Device, InCommandList, params.pOutputTexture, params.pVelocityTexture, rcasConstants, OutputScaler->Buffer()))
			{
				Config::Instance()->RcasEnabled = false;
				return true;
			}
		}
		else
		{
			if (!RCAS->Dispatch(Device, InCommandList, params.pOutputTexture, params.pVelocityTexture, rcasConstants, paramOutput))
			{
				Config::Instance()->RcasEnabled = false;
				return true;
			}
		}
	}

	if (useSS)
	{
		spdlog::debug("XeSSFeatureDx12::Evaluate downscaling output...");
		OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OutputScaler->Dispatch(Device, InCommandList, OutputScaler->Buffer(), paramOutput))
		{
			Config::Instance()->OutputScalingEnabled = false;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}

	// imgui
	if (!Config::Instance()->OverlayMenu.value_or(true) && _frameCount > 30)
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

	AfterEvaluate(InCommandList, InParams, params.pOutputTexture);

	_frameCount++;

	return true;
}

bool XeSSFeatureDx12::Evaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams)
{
	spdlog::debug("XeSSFeatureDx12::Evaluate");

	if (!IsInited() || !_xessContext || !ModuleLoaded())
	{
		spdlog::error("XeSSFeatureDx12::Evaluate Not inited!");
		return false;
	}

	if (!RCAS->IsInit())
		Config::Instance()->RcasEnabled = false;

	if (!OutputScaler->IsInit())
		Config::Instance()->OutputScalingEnabled = false;

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

	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	params.jitterOffsetX = InParams->JitterOffsetX();
	params.jitterOffsetY = InParams->JitterOffsetY();
	params.exposureScale = InParams->ExposureScale();
	params.resetHistory = InParams->Reset() ? 1 : 0;

	if (InParams->RenderWidth() > 0)
		params.inputWidth = InParams->RenderWidth();
	else
		params.inputWidth = RenderWidth();

	if (InParams->RenderHeight() > 0)
		params.inputHeight = InParams->RenderHeight();
	else
		params.inputHeight = RenderHeight();

	spdlog::debug("XeSSFeatureDx12::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	auto sharpness = InParams->Sharpness();

	auto beforeResult = BeforeEvaluate(InCommandList, InParams, params.pOutputTexture);

	// Autoexposure Reset
	if (Config::Instance()->changeBackend)
		return true;

	params.pColorTexture = (ID3D12Resource*)InParams->InputColor();
	params.pDepthTexture = (ID3D12Resource*)InParams->InputDepth();
	params.pExposureScaleTexture = (ID3D12Resource*)InParams->InputExposure();
	params.pVelocityTexture = (ID3D12Resource*)InParams->InputMotion();
	params.pResponsivePixelMaskTexture = (ID3D12Resource*)InParams->InputMask();

	xessResult = SetVelocityScale()(_xessContext, InParams->MvScaleX(), InParams->MvScaleY());

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessSetVelocityScale: {0}", ResultToString(xessResult));
		return false;
	}

	spdlog::debug("XeSSFeatureDx12::Evaluate Executing!!");
	xessResult = D3D12Execute()(_xessContext, InCommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx12::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	AfterEvaluate(InCommandList, InParams, params.pOutputTexture);

	_frameCount++;

	return true;
}

XeSSFeatureDx12::~XeSSFeatureDx12()
{
}
