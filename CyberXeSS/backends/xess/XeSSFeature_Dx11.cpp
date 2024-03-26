#pragma once
#include "../../pch.h"
#include "../../Config.h"

#include "XeSSFeature_Dx11.h"

#if _DEBUG
#include "../../d3dx/D3DX11tex.h"
#endif

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

	if (Config::Instance()->xessDebug)
	{
		xess_dump_parameters_t dumpParams{};
		dumpParams.frame_count = 3;
		dumpParams.frame_idx = 1;
		dumpParams.path = ".";
		dumpParams.dump_elements_mask = XESS_DUMP_INPUT_COLOR | XESS_DUMP_INPUT_VELOCITY | XESS_DUMP_INPUT_DEPTH | XESS_DUMP_OUTPUT | XESS_DUMP_EXECUTION_PARAMETERS;
		xessStartDump(_xessContext, &dumpParams);
		Config::Instance()->xessDebug = false;
	}

	if (Config::Instance()->changeCAS)
	{
		Config::Instance()->changeCAS = false;
		CAS.reset();
		CAS = std::make_unique<CAS_Dx12>(Dx12on11Device, DisplayWidth(), DisplayHeight(), Config::Instance()->CasColorSpaceConversion.value_or(0));
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

	if (Config::Instance()->ColorSpaceFix.value_or(false) &&
		ColorDecode->CreateBufferResource(Dx12on11Device, dx11Color.Dx12Resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) &&
		ColorDecode->Dispatch(Dx12on11Device, Dx12CommandList, dx11Color.Dx12Resource, ColorDecode->Buffer(), 16, 16))
	{
		params.pColorTexture = ColorDecode->Buffer();
	}
	else
	{
		params.pColorTexture = dx11Color.Dx12Resource;
	}

	_hasColor = params.pColorTexture != nullptr;
	params.pVelocityTexture = dx11Mv.Dx12Resource;
	_hasMV = params.pVelocityTexture != nullptr;

	if (Config::Instance()->CasEnabled.value_or(true) && sharpness > 0.0f)
	{
		if (!CAS->CreateBufferResource(Dx12on11Device, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			spdlog::error("XeSSFeatureDx12::Evaluate Can't create cas buffer!");

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

			return false;
		}

		params.pOutputTexture = CAS->Buffer();
	}
	else
	{
		if (Config::Instance()->ColorSpaceFix.value_or(false) && OutputEncode->CreateBufferResource(Dx12on11Device, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			OutputEncode->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			params.pOutputTexture = OutputEncode->Buffer();
		}
		else
			params.pOutputTexture = dx11Out.Dx12Resource;
	}

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

			Dx12CommandList->Close();
			ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
			Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

			Dx12CommandAllocator->Reset();
			Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

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
	if (Config::Instance()->CasEnabled.value_or(true) && sharpness > 0.0f)
	{
		ResourceBarrier(Dx12CommandList, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (Config::Instance()->ColorSpaceFix.value_or(false) && OutputEncode->CreateBufferResource(Dx12on11Device, params.pOutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			OutputEncode->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			if (!CAS->Dispatch(Dx12CommandList, sharpness, params.pOutputTexture, OutputEncode->Buffer()))
			{
				Config::Instance()->CasEnabled = false;

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
			CAS->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			if (!CAS->Dispatch(Dx12CommandList, sharpness, CAS->Buffer(), dx11Out.Dx12Resource))
			{
				Config::Instance()->CasEnabled = false;

				Dx12CommandList->Close();
				ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
				Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

				Dx12CommandAllocator->Reset();
				Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

				return true;
			}
		}
	}

	if (OutputEncode->Buffer() && Config::Instance()->ColorSpaceFix.value_or(false))
	{
		OutputEncode->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (CreateBufferResource(Dx12on11Device, dx11Out.Dx12Resource, &_outBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			OutputEncode->Dispatch(Dx12on11Device, Dx12CommandList, OutputEncode->Buffer(), _outBuffer, 16, 16);

			ResourceBarrier(Dx12CommandList, _outBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
			ResourceBarrier(Dx12CommandList, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

			Dx12CommandList->CopyResource(dx11Out.Dx12Resource, _outBuffer);
		}
		else
		{
			OutputEncode->SetBufferState(Dx12CommandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
			ResourceBarrier(Dx12CommandList, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);
		
			Dx12CommandList->CopyResource(dx11Out.Dx12Resource, OutputEncode->Buffer());
		}

		ResourceBarrier(Dx12CommandList, dx11Out.Dx12Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	if (!CopyBackOutput())
	{
		spdlog::error("XeSSFeatureDx11::Evaluate Can't copy output texture back!");

		Dx12CommandAllocator->Reset();
		Dx12CommandList->Reset(Dx12CommandAllocator, nullptr);

		return false;
	}

	// Execute dx12 commands to process xess
	Dx12CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { Dx12CommandList };
	Dx12CommandQueue->ExecuteCommandLists(1, ppCommandLists);

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
	if (_outBuffer != nullptr)
	{
		_outBuffer->Release();
		_outBuffer = nullptr;
	}
}

