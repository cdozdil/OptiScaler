#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx11.h"

XeSSFeatureDx11::XeSSFeatureDx11(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : XeSSFeature(InHandleId, InParameters), IFeature_Dx11wDx12(InHandleId, InParameters), IFeature_Dx11(InHandleId, InParameters), IFeature(InHandleId, InParameters)
{
}

bool XeSSFeatureDx11::Init(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Init!");

	if (IsInited())
		return true;

	if (!BaseInit(InDevice, InContext, InParameters))
	{
		spdlog::debug("XeSSFeatureDx11::Init BaseInit failed!");
		return false;
	}

	spdlog::debug("XeSSFeatureDx11::Init calling InitXeSS");

	if (Dx12on11Device && !InitXeSS(Dx12on11Device, InParameters))
	{
		spdlog::error("XeSSFeatureDx11::Init InitXeSS fail!");
		return false;
	}

	Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);
	return true;
}

bool XeSSFeatureDx11::Evaluate(ID3D11DeviceContext* InDeviceContext, const NVSDK_NGX_Parameter* InParameters)
{
	spdlog::debug("XeSSFeatureDx11::Evaluate");

	if (!IsInited() || !_xessContext)
		return false;

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

	// creatimg params for XeSS
	xess_result_t xessResult;
	xess_d3d12_execute_params_t params{};

	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_X, &params.jitterOffsetX);
	InParameters->Get(NVSDK_NGX_Parameter_Jitter_Offset_Y, &params.jitterOffsetY);

	if (InParameters->Get(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, &params.exposureScale) != NVSDK_NGX_Result_Success)
		params.exposureScale = 1.0f;

	InParameters->Get(NVSDK_NGX_Parameter_Reset, &params.resetHistory);

	GetRenderResolution(InParameters, &params.inputWidth, &params.inputHeight);

	spdlog::debug("XeSSFeatureDx11::Evaluate Input Resolution: {0}x{1}", params.inputWidth, params.inputHeight);

	if (!ProcessDx11Textures(InParameters))
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Can't process Dx11 textures!");
		return false;
	}

	params.pColorTexture = dx11Color.Dx12Resource;
	_hasColor = params.pColorTexture != nullptr;
	params.pVelocityTexture = dx11Mv.Dx12Resource;
	_hasMV = params.pVelocityTexture != nullptr;

	if (Config::Instance()->CasEnabled.value_or(true) && casSharpness > 0.0f)
	{
		if (casBuffer == nullptr && !CreateCasBufferResource(dx11Out.Dx12Resource, Dx12on11Device))
		{
			spdlog::error("XeSSFeatureDx12::Evaluate Can't create cas buffer!");
			return false;
		}

		params.pOutputTexture = casBuffer;
	}
	else
		params.pOutputTexture = dx11Out.Dx12Resource;
	
	_hasOutput = params.pOutputTexture != nullptr;

	params.pDepthTexture = dx11Depth.Dx12Resource;
	_hasDepth = params.pDepthTexture != nullptr;
	params.pExposureScaleTexture = dx11Exp.Dx12Resource;
	_hasExposure = params.pExposureScaleTexture != nullptr;
	params.pResponsivePixelMaskTexture = dx11Tm.Dx12Resource;
	_hasTM = params.pResponsivePixelMaskTexture != nullptr;

	float MVScaleX;
	float MVScaleY;

	if (InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_X, &MVScaleX) == NVSDK_NGX_Result_Success &&
		InParameters->Get(NVSDK_NGX_Parameter_MV_Scale_Y, &MVScaleY) == NVSDK_NGX_Result_Success)
	{
		xessResult = xessSetVelocityScale(_xessContext, MVScaleX, MVScaleY);

		if (xessResult != XESS_RESULT_SUCCESS)
		{
			spdlog::error("XeSSFeatureDx11::Evaluate xessSetVelocityScale error: {0}", ResultToString(xessResult));
			return false;
		}
	}
	else
		spdlog::warn("FeatureContext::XeSSExecuteDx12 Can't get motion vector scales!");

	// Execute xess
	spdlog::debug("XeSSFeatureDx11::Evaluate Executing!!");
	xessResult = xessD3D12Execute(_xessContext, Dx12CommandList, &params);

	if (xessResult != XESS_RESULT_SUCCESS)
	{
		spdlog::error("XeSSFeatureDx11::Evaluate xessD3D12Execute error: {0}", ResultToString(xessResult));
		return false;
	}

	//apply cas
	if (Config::Instance()->CasEnabled.value_or(true) && !CasDispatch(Dx12CommandList, InParameters, casBuffer, dx11Out.Dx12Resource))
		Config::Instance()->CasEnabled = false;

	// Execute dx12 commands to process xess
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	if (!CopyBackOutput())
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Can't copy output texture back!");
		return false;
	}

	// imgui
	if (Imgui)
	{
		if (Imgui->IsHandleDifferent())
			Imgui.reset();
		else
			Imgui->Render(DeviceContext, paramOutput);
	}
	else
		Imgui = std::make_unique<Imgui_Dx11>(GetForegroundWindow(), Device);


	Dx12CommandAllocator->Reset();
	Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

	return true;
}

XeSSFeatureDx11::~XeSSFeatureDx11()
{
	spdlog::debug("XeSSFeatureDx11::XeSSFeatureDx11");
}

