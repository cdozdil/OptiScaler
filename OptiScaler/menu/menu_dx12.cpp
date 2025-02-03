#include "menu_dx12.h"

#include <d3dx/d3dx12.h>

#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

long frameCounter = 0;

bool Menu_Dx12::Render(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* outTexture)
{
    if (Config::Instance()->OverlayMenu.value_or_default())
        return false;

    if (pCmdList == nullptr || outTexture == nullptr)
        return false;

    frameCounter++;

    //if (!IsVisible())
    //	return true;

    auto outDesc = outTexture->GetDesc();

    CreateRenderTarget(outDesc);

    if (!_dx12Init && ImGui::GetIO().BackendRendererUserData == nullptr)
    {
        _dx12Init = ImGui_ImplDX12_Init(_device, 2, outDesc.Format, _srvDescHeap,
                                        _srvDescHeap->GetCPUDescriptorHandleForHeapStart(), _srvDescHeap->GetGPUDescriptorHandleForHeapStart());
    }

    if (!_dx12Init)
        return false;

    frameCounter++;

    auto backbuf = frameCounter % 2;

    D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
    rtDesc.Format = outDesc.Format;
    rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    if ((outDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) > 0)
    {
        D3D12_RESOURCE_BARRIER outBarrier = { };
        outBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        outBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        outBarrier.Transition.pResource = outTexture;
        outBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pCmdList->ResourceBarrier(1, &outBarrier);

        // Create RTV for out
        pCmdList->SetDescriptorHeaps(1, &_srvDescHeap);

        _device->CreateRenderTargetView(outTexture, &rtDesc, _renderTargetDescriptor[backbuf]);
        pCmdList->OMSetRenderTargets(1, &_renderTargetDescriptor[backbuf], FALSE, NULL);

        ImGui_ImplDX12_NewFrame();
        //ImGui_ImplWin32_NewFrame();

        // Render
        MenuDxBase::RenderMenu();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);

        outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        pCmdList->ResourceBarrier(1, &outBarrier);

        return true;
    }

    D3D12_RESOURCE_BARRIER outBarrier = { };
    outBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    outBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    outBarrier.Transition.pResource = outTexture;
    outBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

    D3D12_RESOURCE_BARRIER bufferBarrier = { };
    bufferBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    bufferBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    bufferBarrier.Transition.pResource = _renderTargetResource[backbuf];
    bufferBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

    D3D12_RESOURCE_BARRIER barriers[] = { bufferBarrier, outBarrier };
    pCmdList->ResourceBarrier(2, barriers);

    // Copy out to buffer
    pCmdList->CopyResource(_renderTargetResource[backbuf], outTexture);

    // Set as render target
    bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    pCmdList->ResourceBarrier(1, &bufferBarrier);

    // Create RTV for buffer
    pCmdList->SetDescriptorHeaps(1, &_srvDescHeap);

    _device->CreateRenderTargetView(_renderTargetResource[backbuf], &rtDesc, _renderTargetDescriptor[backbuf]);
    pCmdList->OMSetRenderTargets(1, &_renderTargetDescriptor[backbuf], FALSE, nullptr);

    ImGui_ImplDX12_NewFrame();
    //ImGui_ImplWin32_NewFrame();

    // Render to buffer
    if (MenuDxBase::RenderMenu())
    {
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);

        outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

        bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        pCmdList->ResourceBarrier(2, barriers);

        // Copy back buffer to out
        pCmdList->CopyResource(outTexture, _renderTargetResource[backbuf]);

        // fix states
        bufferBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        bufferBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        outBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        outBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        pCmdList->ResourceBarrier(2, barriers);
    }

    return true;
}

Menu_Dx12::Menu_Dx12(HWND handle, ID3D12Device* pDevice) : MenuDxBase(handle), _device(pDevice)
{
    if (Config::Instance()->OverlayMenu.value_or_default())
        return;

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = { };
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = 2;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDesc.NodeMask = 1;

    if (pDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&_rtvDescHeap)) != S_OK)
        return;

    _rtvDescHeap->SetName(L"Imgui_Dx12_rtvHeap");

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

    State::Instance().skipHeapCapture = true;

    if (pDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&_srvDescHeap)) != S_OK)
        return;

    State::Instance().skipHeapCapture = false;

    Dx12Ready();

    _srvDescHeap->SetName(L"Imgui_Dx12_srvDescHeap");
}

Menu_Dx12::~Menu_Dx12()
{
    if (!_dx12Init)
        return;

    if (State::Instance().isShuttingDown)
        return;

    ImGui_ImplDX12_Shutdown();
    MenuCommon::Shutdown();

    if (_rtvDescHeap)
    {
        _rtvDescHeap->Release();
        _rtvDescHeap = nullptr;
    }

    if (_srvDescHeap)
    {
        _srvDescHeap->Release();
        _srvDescHeap = nullptr;
    }

    if (_renderTargetResource[0])
    {
        _renderTargetResource[0]->Release();
        _renderTargetResource[0] = nullptr;
    }

    if (_renderTargetResource[1])
    {
        _renderTargetResource[1]->Release();
        _renderTargetResource[1] = nullptr;
    }
}

void Menu_Dx12::CreateRenderTarget(const D3D12_RESOURCE_DESC& InDesc)
{
    if (_renderTargetResource[0] != nullptr)
    {
        auto rtDesc = _renderTargetResource[0]->GetDesc();

        if (InDesc.Width != rtDesc.Width || InDesc.Height != rtDesc.Height || InDesc.Format != rtDesc.Format)
        {
            _renderTargetResource[0]->Release();
            _renderTargetResource[0] = nullptr;
            _renderTargetResource[1]->Release();
            _renderTargetResource[1] = nullptr;
        }
        else
            return;
    }

    for (UINT i = 0; i < 2; ++i)
    {
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = InDesc.Width;
        desc.Height = InDesc.Height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = TranslateTypelessFormats(InDesc.Format);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        ID3D12Resource* renderTarget;
        auto result = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&renderTarget));

        if (result == S_OK) {
            renderTarget->SetName(L"Imgui_Dx12_renderTarget");

            D3D12_RENDER_TARGET_VIEW_DESC rtDesc = { };
            rtDesc.Format = InDesc.Format;
            rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            _device->CreateRenderTargetView(renderTarget, &rtDesc, _renderTargetDescriptor[i]);
            _renderTargetResource[i] = renderTarget;
        }
    }
}