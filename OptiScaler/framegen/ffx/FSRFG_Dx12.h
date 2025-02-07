#pragma once

#include <framegen/IFGFeature_Dx12.h>

#include <proxies/FfxApi_Proxy.h>
#include <dx12/ffx_api_dx12.h>
#include <ffx_framegeneration.h>


class FSRFG_Dx12 : public virtual IFGFeature_Dx12
{
private:
    ffxContext _swapChainContext = nullptr;
    ffxContext _fgContext = nullptr;
    

public:

    // Inherited via IFGFeature_Dx12
    UINT64 UpscaleStart() final;
    void UpscaleEnd() final;

    feature_version Version() final;

    const char* Name() final;

    bool Dispatch(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* output, double frameTime) final;

    bool DispatchHudless(bool useHudless, double frameTime) final;

    void StopAndDestroyContext(bool destroy, bool shutDown, bool useMutex) final;

    void CreateSwapchain(IDXGIFactory* factory, ID3D12CommandQueue* cmdQueue, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain) final;

    void ReleaseSwapchain(IDXGISwapChain* swapChain) final;

    void CreateContext(ID3D12Device* device, IFeature* upscalerContext) final;

    void ConfigureFramePaceTuning();

    ffxReturnCode_t DispatchCallback(ffxDispatchDescFrameGeneration* params);
    ffxReturnCode_t HudlessDispatchCallback(ffxDispatchDescFrameGeneration* params);
};
