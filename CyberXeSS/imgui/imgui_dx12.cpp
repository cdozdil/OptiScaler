#include "Imgui_Dx12.h"
#include "d3dx12.h"
#include "imgui/imgui_impl_dx12.h"

long frameCounter = 0;

bool Imgui_Dx12::Render(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* outTexture)
{
	if (pCmdList == nullptr || outTexture == nullptr)
		return false;

	if (!IsVisible())
		return true;

	auto outDesc = outTexture->GetDesc();

	if (!_dx12Init && ImGui::GetIO().BackendRendererUserData == nullptr)
	{
		CreateRenderTarget(outDesc);
		_dx12Init = ImGui_ImplDX12_Init(_device, 2, outDesc.Format, _srvDescHeap, _srvDescHeap->GetCPUDescriptorHandleForHeapStart(), _srvDescHeap->GetGPUDescriptorHandleForHeapStart());
	}

	if (!_dx12Init)
		return false;

	frameCounter++;

	auto backbuf = frameCounter % 2;

	ImGui_ImplDX12_NewFrame();
	Imgui_Base::RenderMenu();

	D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
	rtDesc.Format = outDesc.Format;
	rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	_device->CreateRenderTargetView(_renderTargetResource[backbuf], &rtDesc, _renderTargetDescriptor[backbuf]);

	pCmdList->OMSetRenderTargets(1, &_renderTargetDescriptor[backbuf], FALSE, NULL);
	pCmdList->SetDescriptorHeaps(1, &_srvDescHeap);

	ID3D12Fence* dx12fence;
	_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12fence));

	auto drawData = ImGui::GetDrawData();
	ImGui_ImplDX12_RenderDrawData(drawData, pCmdList);

	D3D12_RESOURCE_BARRIER bufferBarrier = { };
	bufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	bufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	bufferBarrier.Transition.pResource = _renderTargetResource[backbuf];
	bufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	pCmdList->ResourceBarrier(1, &bufferBarrier);

	D3D12_RESOURCE_BARRIER outBarrier = { };
	outBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	outBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	outBarrier.Transition.pResource = outTexture;
	outBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	pCmdList->ResourceBarrier(1, &outBarrier);

	// Copy intermediate render target to your texture
	pCmdList->CopyResource(_renderTargetResource[backbuf], outTexture);

	bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	pCmdList->ResourceBarrier(1, &bufferBarrier);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);

	bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	pCmdList->ResourceBarrier(1, &bufferBarrier);

	outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	pCmdList->ResourceBarrier(1, &outBarrier);

	// Copy intermediate render target to your texture
	pCmdList->CopyResource(outTexture, _renderTargetResource[backbuf]);

	dx12fence->Signal(10);

	// wait for end of operation
	if (dx12fence->GetCompletedValue() < 10)
	{
		auto fenceEvent12 = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (fenceEvent12)
		{
			dx12fence->SetEventOnCompletion(10, fenceEvent12);
			WaitForSingleObject(fenceEvent12, INFINITE);
			CloseHandle(fenceEvent12);
		}
	}

	outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
	pCmdList->ResourceBarrier(1, &outBarrier);

	bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	pCmdList->ResourceBarrier(1, &bufferBarrier);
}

Imgui_Dx12::Imgui_Dx12(HWND handle, ID3D12Device* pDevice) : Imgui_Base(handle), _device(pDevice)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = { };
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = 2;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDesc.NodeMask = 1;

	if (pDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&_rtvDescHeap)) != S_OK)
		return;

	SIZE_T rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvDescHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < 2; ++i) {
		_renderTargetDescriptor[i] = rtvHandle;
		rtvHandle.ptr += rtvDescriptorSize;
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvDesc = { };
	srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvDesc.NumDescriptors = 1;
	srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	if (pDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&_srvDescHeap)) != S_OK)
		return;
}

Imgui_Dx12::~Imgui_Dx12()
{
	if (!_dx12Init)
		return;


	if (auto currCtx = ImGui::GetCurrentContext(); currCtx && context != currCtx)
	{
		ImGui::SetCurrentContext(context);
		ImGui_ImplDX12_Shutdown();
		ImGui::SetCurrentContext(currCtx);
	}
	else
		ImGui_ImplDX12_Shutdown();

	// hackzor
	std::this_thread::sleep_for(std::chrono::milliseconds(400));

	if (_rtvDescHeap)
		_rtvDescHeap->Release();

	if (_srvDescHeap)
		_srvDescHeap->Release();

	if (_renderTargetResource[0])
		_renderTargetResource[0]->Release();

	if (_renderTargetResource[1])
		_renderTargetResource[1]->Release();
}

void Imgui_Dx12::CreateRenderTarget(const D3D12_RESOURCE_DESC& InDesc)
{
	for (UINT i = 0; i < 2; ++i) {

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = InDesc.Width;
		desc.Height = InDesc.Height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = InDesc.Format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		ID3D12Resource* renderTarget;
		auto result = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&renderTarget));

		if (result == S_OK) {
			D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
			rtDesc.Format = InDesc.Format;
			rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

			_device->CreateRenderTargetView(renderTarget, &rtDesc, _renderTargetDescriptor[i]);
			_renderTargetResource[i] = renderTarget;
		}
	}
}