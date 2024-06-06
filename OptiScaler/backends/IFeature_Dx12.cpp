#pragma once
#include "IFeature_Dx12.h"

#include "../pch.h"


void IFeature_Dx12::ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState) const
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = InResource;
	barrier.Transition.StateBefore = InBeforeState;
	barrier.Transition.StateAfter = InAfterState;
	barrier.Transition.Subresource = 0;
	InCommandList->ResourceBarrier(1, &barrier);
}

bool IFeature_Dx12::BeforeEvaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams, ID3D12Resource* OutputTexture)
{
	auto useOutputScaling = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(_createParams.DisplayResolutionMV());

	auto paramColor = (ID3D12Resource*)InParams->InputColor();
	auto paramMotion = (ID3D12Resource*)InParams->InputMotion();
	auto paramOutput = (ID3D12Resource*)InParams->OutputColor();
	auto paramDepth = (ID3D12Resource*)InParams->InputDepth();
	auto paramExposure = (ID3D12Resource*)InParams->InputExposure();
	auto paramMask = (ID3D12Resource*)InParams->InputMask();

	// Color
	if (paramColor)
	{
		spdlog::debug("IFeature_Dx12::BeforeEvaluate Color exist..");
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
	}
	else
	{
		spdlog::error("IFeature_Dx12::BeforeEvaluate Color not exist!!");
		return false;
	}

	// Motion
	if (paramMotion)
	{
		spdlog::debug("IFeature_Dx12::BeforeEvaluate MotionVectors exist..");
		paramMotion->SetName(L"paramMotion");

		if (Config::Instance()->MVResourceBarrier.has_value())
		{
			ResourceBarrier(InCommandList, paramMotion, (D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
		{
			Config::Instance()->MVResourceBarrier = (int)D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			ResourceBarrier(InCommandList, paramMotion, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
	}
	else
	{
		spdlog::error("IFeature_Dx12::BeforeEvaluate MotionVectors not exist!!");
		return false;
	}

	// Output
	if (paramOutput)
	{
		spdlog::debug("IFeature_Dx12::BeforeEvaluate Output exist..");
		paramOutput->SetName(L"paramOutput");

		if (Config::Instance()->OutputResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramOutput, (D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (useOutputScaling)
		{
			if (OutputScaler->CreateBufferResource(Device, paramOutput, TargetWidth(), TargetHeight(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			{
				OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				OutputTexture = OutputScaler->Buffer();
			}
			else
			{
				OutputTexture = paramOutput;
			}
		}
		else
		{
			OutputTexture = paramOutput;
		}

		if (Config::Instance()->RcasEnabled.value_or(UseRcas()) &&
			(InParams->Sharpness() > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
			RCAS->IsInit() && RCAS->CreateBufferResource(Device, OutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
		{
			RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			OutputTexture = RCAS->Buffer();
		}
	}
	else
	{
		spdlog::error("IFeature_Dx12::BeforeEvaluate Output not exist!!");
		return false;
	}

	// Depth
	if (paramDepth)
	{
		spdlog::debug("IFeature_Dx12::BeforeEvaluate Depth exist..");
		paramDepth->SetName(L"params.pDepthTexture");

		if (Config::Instance()->DepthResourceBarrier.has_value())
			ResourceBarrier(InCommandList, paramDepth, (D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		if (!Config::Instance()->DisplayResolution.value_or(false))
			spdlog::error("IFeature_Dx12::BeforeEvaluate Depth not exist!!");
		else
			spdlog::info("IFeature_Dx12::BeforeEvaluate Using high res motion vectors, depth is not needed!!");
	}

	// Exposure
	if (!Config::Instance()->AutoExposure.value_or(false))
	{

		if (paramExposure)
		{
			spdlog::debug("IFeature_Dx12::BeforeEvaluate ExposureTexture exist..");

			if (Config::Instance()->ExposureResourceBarrier.has_value())
				ResourceBarrier(InCommandList, paramExposure, (D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else
		{
			spdlog::warn("IFeature_Dx12::BeforeEvaluate AutoExposure disabled but ExposureTexture is not exist, it may cause problems!!");
			Config::Instance()->AutoExposure = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}
	else
		spdlog::debug("IFeature_Dx12::BeforeEvaluate AutoExposure enabled!");

	// Mask
	if (!Config::Instance()->DisableReactiveMask.value_or(true))
	{
		if (paramMask)
		{
			spdlog::debug("IFeature_Dx12::BeforeEvaluate Bias mask exist..");

			if (Config::Instance()->MaskResourceBarrier.has_value())
				ResourceBarrier(InCommandList, paramMask, (D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else
		{
			spdlog::warn("IFeature_Dx12::BeforeEvaluate Bias mask not exist and its enabled in config, it may cause problems!!");
			Config::Instance()->DisableReactiveMask = true;
			Config::Instance()->changeBackend = true;
			return true;
		}
	}

	_hasColor = paramColor != nullptr;
	_hasMV = paramMotion != nullptr;
	_hasOutput = paramOutput != nullptr;
	_hasDepth = paramDepth != nullptr;
	_hasExposure = paramExposure != nullptr;
	_hasTM = paramMask != nullptr;

	return true;
}

void IFeature_Dx12::AfterEvaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams, ID3D12Resource* OutputTexture)
{
	auto useOutputScaling = Config::Instance()->OutputScalingEnabled.value_or(false) && !Config::Instance()->DisplayResolution.value_or(_createParams.DisplayResolutionMV());

	auto paramColor = (ID3D12Resource*)InParams->InputColor();
	auto paramMotion = (ID3D12Resource*)InParams->InputMotion();
	auto paramOutput = (ID3D12Resource*)InParams->OutputColor();
	auto paramDepth = (ID3D12Resource*)InParams->InputDepth();
	auto paramExposure = (ID3D12Resource*)InParams->InputExposure();
	auto paramMask = (ID3D12Resource*)InParams->InputMask();

	// apply rcas
	if (Config::Instance()->RcasEnabled.value_or(UseRcas()) &&
		(InParams->Sharpness() > 0.0f || (Config::Instance()->MotionSharpnessEnabled.value_or(false) && Config::Instance()->MotionSharpness.value_or(0.4) > 0.0f)) &&
		RCAS->CanRender())
	{
		if (OutputTexture != RCAS->Buffer())
			ResourceBarrier(InCommandList, OutputTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		RCAS->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		RcasConstants rcasConstants{};
		rcasConstants.Sharpness = InParams->Sharpness();
		rcasConstants.DisplayWidth = TargetWidth();
		rcasConstants.DisplayHeight = TargetHeight();
		rcasConstants.MvScaleX = InParams->MvScaleX();
		rcasConstants.MvScaleY = InParams->MvScaleY();
		rcasConstants.DisplaySizeMV = _createParams.DisplayResolutionMV();
		rcasConstants.RenderHeight = RenderHeight();
		rcasConstants.RenderWidth = RenderWidth();

		if (useOutputScaling)
		{
			if (!RCAS->Dispatch(Device, InCommandList, OutputTexture, paramMotion, rcasConstants, OutputScaler->Buffer()))
			{
				Config::Instance()->RcasEnabled = false;
				return;
			}
		}
		else
		{
			if (!RCAS->Dispatch(Device, InCommandList, OutputTexture, paramMotion, rcasConstants, paramOutput))
			{
				Config::Instance()->RcasEnabled = false;
				return;
			}
		}
	}

	if (useOutputScaling)
	{
		spdlog::debug("IFeature_Dx12::AfterEvaluate output scaling output...");
		OutputScaler->SetBufferState(InCommandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		if (!OutputScaler->Dispatch(Device, InCommandList, OutputScaler->Buffer(), paramOutput))
		{
			Config::Instance()->OutputScalingEnabled = false;
			Config::Instance()->changeBackend = true;
			return;
		}
	}

	// Imgui
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

	// Restore resource states
	if (paramColor && Config::Instance()->ColorResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramColor,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ColorResourceBarrier.value());

	if (paramMotion && Config::Instance()->MVResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramMotion,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MVResourceBarrier.value());

	if (paramOutput && Config::Instance()->OutputResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramOutput,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			(D3D12_RESOURCE_STATES)Config::Instance()->OutputResourceBarrier.value());

	if (paramDepth && Config::Instance()->DepthResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramDepth,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->DepthResourceBarrier.value());

	if (paramExposure && Config::Instance()->ExposureResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramExposure,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->ExposureResourceBarrier.value());

	if (paramMask && Config::Instance()->MaskResourceBarrier.has_value())
		ResourceBarrier(InCommandList, paramMask,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			(D3D12_RESOURCE_STATES)Config::Instance()->MaskResourceBarrier.value());
}

IFeature_Dx12::IFeature_Dx12(unsigned int InHandleId, const IFeatureCreateParams InParameters)
{
}

void IFeature_Dx12::Shutdown()
{
	if (Imgui != nullptr && Imgui.get() != nullptr)
		Imgui.reset();
}

IFeature_Dx12::~IFeature_Dx12()
{
	if (Device)
	{
		ID3D12Fence* d3d12Fence = nullptr;

		do
		{
			if (Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
				break;

			d3d12Fence->Signal(999);

			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
			{
				WaitForSingleObject(fenceEvent, INFINITE);
				CloseHandle(fenceEvent);
			}

		} while (false);

		if (d3d12Fence != nullptr)
		{
			d3d12Fence->Release();
			d3d12Fence = nullptr;
		}
	}

	if (OutputScaler != nullptr && OutputScaler.get() != nullptr)
		OutputScaler.reset();

	if (RCAS != nullptr && RCAS.get() != nullptr)
		RCAS.reset();
}
