#pragma once
#include <pch.h>

#include <Config.h>
#include <State.h>
#include <Util.h>

#include <shaders/format_transfer/FT_Dx12.h>

#include <set>
#include <d3d12.h>

// Remove duplicate definitions later, collect them in another header
enum ResourceType
{
    SRV,
    RTV,
    UAV
};

// Remove duplicate definitions later, collect them in another header
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

    // Captured hudless resources
    inline static ID3D12Resource* _capturedHudless[4] = { nullptr, nullptr, nullptr, nullptr };

    // Buffer for Format Transfer
    inline static ID3D12Resource* _captureBuffer[4] = { nullptr, nullptr, nullptr, nullptr };

    // Capture List
    inline static std::set<ID3D12Resource*> _captureList;

    inline static std::shared_mutex _captureMutex;
    inline static std::shared_mutex counterMutex;
    inline static INT64 _captureCounter[4] = { 0, 0, 0, 0 };
    inline static FT_Dx12* _formatTransfer = nullptr;

    inline static bool CreateBufferResource(ID3D12Device* InDevice, ResourceInfo* InSource, D3D12_RESOURCE_STATES InState, ID3D12Resource** OutResource);
    inline static void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState);
    
    // Check _captureCounter for current frame
    inline static bool CheckCapture();    

    inline static int GetIndex();

public:
    // Trig for upscaling start
    inline static void UpscaleStart();

    // Trig for upscaling end
    inline static void UpscaleEnd(float lastFrameTime);
    
    // Trig for present start
    inline static void PresentStart();

    // Trig for present end
    inline static void PresentEnd();

    inline static UINT64 ActiveUpscaleFrame();
    inline static UINT64 ActivePresentFrame();

    // Is Hudfix active
    inline static bool IsResourceCheckActive();

    // Check resource for hudless
    inline static bool CheckForHudless(std::string callerName, ID3D12GraphicsCommandList* cmdList, ResourceInfo* resource, D3D12_RESOURCE_STATES state);
};