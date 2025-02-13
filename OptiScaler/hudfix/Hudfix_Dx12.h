#pragma once
#include <pch.h>

#include <shaders/format_transfer/FT_Dx12.h>

#include <set>
#include <dxgi.h>
#include <d3d12.h>
#include <shared_mutex>

enum ResourceType
{
    SRV,
    RTV,
    UAV
};

typedef struct ResourceInfo
{
    ID3D12Resource* buffer = nullptr;
    UINT64 width = 0;
    UINT height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
    ResourceType type = SRV;
    double lastUsedFrame = 0;
} resource_info;

class Hudfix_Dx12
{
private:
    // Last upscaled frame
    inline static UINT64 _upscaleCounter = 0;

    // Last presented frame
    inline static UINT64 _fgCounter = 0;

    // Limit until calling FG without hudless
    inline static double _lastDiffTime = 0.0;
    inline static double _upscaleEndTime = 0.0;
    inline static double _targetTime = 0.0;
    inline static double _frameTime = 0.0;

    // Buffer for Format Transfer
    inline static ID3D12Resource* _captureBuffer[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };

    // Capture List
    inline static std::set<ID3D12Resource*> _captureList;

    inline static std::shared_mutex _captureMutex;
    inline static std::shared_mutex _counterMutex;
    inline static INT64 _captureCounter[BUFFER_COUNT] = { 0, 0, 0, 0 };
    inline static FT_Dx12* _formatTransfer = nullptr;

    inline static ID3D12CommandQueue* _commandQueue = nullptr;
    inline static ID3D12GraphicsCommandList* _commandList[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    inline static ID3D12CommandAllocator* _commandAllocator[BUFFER_COUNT] = { nullptr, nullptr, nullptr, nullptr };
    
    inline static bool _skipHudlessChecks = false;

    static bool CreateObjects();    
    static bool CreateBufferResource(ID3D12Device* InDevice, ResourceInfo* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource);
    static void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState);
    
    // Check _captureCounter for current frame
    static bool CheckCapture();    
    static bool CheckResource(ResourceInfo* resource);

    static int GetIndex();
    static void DispatchFG(bool useHudless);


public:
    // Trig for upscaling start
    static void UpscaleStart();

    // Trig for upscaling end
    static void UpscaleEnd(UINT64 frameId, float lastFrameTime);
    
    // Trig for present start
    static void PresentStart();

    // Trig for present end
    static void PresentEnd();

    static UINT64 ActiveUpscaleFrame();
    static UINT64 ActivePresentFrame();

    // Is Hudfix active
    static bool IsResourceCheckActive();
    
    // For resource tracking in hooks
    static bool SkipHudlessChecks();

    // Check resource for hudless
    static bool CheckForHudless(std::string callerName, ID3D12GraphicsCommandList* cmdList, ResourceInfo* resource, D3D12_RESOURCE_STATES state);

    // Reset frame counters
    static void ResetCounters();

};