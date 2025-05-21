#pragma once

#include "menu_dx_base.h"
#include <d3d12.h>

class Menu_Dx12 : public MenuDxBase
{
  private:
    bool _dx12Init = false;
    ID3D12Device* _device = nullptr;

    // Old resources
    ID3D12Resource* _renderTargetResource[2] = { nullptr, nullptr };
    ID3D12DescriptorHeap* _rtvDescHeap = nullptr;
    ID3D12DescriptorHeap* _srvDescHeap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE _renderTargetDescriptor[2] = {};

    void CreateRenderTarget(const D3D12_RESOURCE_DESC& InDesc);

  public:
    bool Render(ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* outTexture);

    Menu_Dx12(HWND handle, ID3D12Device* pDevice);

    ~Menu_Dx12();
};
