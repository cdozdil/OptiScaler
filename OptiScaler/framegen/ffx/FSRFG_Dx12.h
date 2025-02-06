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
    

    ffxReturnCode_t HudlessCallback(ffxDispatchDescFrameGeneration* params, void* pUserCtx);
    void ConfigureFramePaceTuning();

public:

    // Inherited via IFGFeature_Dx12
    void UpscaleEnd(float lastFrameTime) final;

    feature_version Version() final;

    const char* Name() final;

    bool Dispatch(UINT64 frameId, double frameTime) final;

    bool DispatchHudless(UINT64 frameId, double frameTime, ID3D12Resource* output) final;

    void StopAndDestroyContext(bool destroy, bool shutDown, bool useMutex) final;

    void CreateSwapchain(IDXGIFactory* factory, ID3D12CommandQueue* cmdQueue, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain) final;

    void ReleaseSwapchain(IDXGISwapChain* swapChain) final;

    void CreateContext(ID3D12Device* device, IFeature* upscalerContext) final;

};
