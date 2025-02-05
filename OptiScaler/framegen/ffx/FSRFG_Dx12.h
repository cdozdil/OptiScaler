#pragma once

#include <framegen/IFGFeature_Dx12.h>

#include <proxies/FfxApi_Proxy.h>
#include <dx12/ffx_api_dx12.h>
#include <ffx_framegeneration.h>


class FSRFG_Dx12 : public virtual IFGFeature_Dx12
{
private:
    ffxContext fgSwapChainContext = nullptr;
    ffxContext fgContext = nullptr;

    ankerl::unordered_dense::map <HWND, SwapChainInfo> fgSwapChains;
    DXGI_SWAP_CHAIN_DESC fgScDesc{};

    void ConfigureFramePaceTuning();

public:
    UINT NewFrame();
    UINT GetFrame();
    void ResetIndexes();
    void ReleaseFGSwapchain(HWND hWnd);
    void ReleaseFGObjects();
    void CreateFGObjects(ID3D12Device* InDevice);
    void CreateFGContext(ID3D12Device* InDevice, IFeature* deviceContext);
    void StopAndDestroyFGContext(bool destroy, bool shutDown, bool useMutex = true);
    void CheckUpscaledFrame(ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InUpscaled);
    void AddFrameTime(float ft);
    float GetFrameTime();
}
