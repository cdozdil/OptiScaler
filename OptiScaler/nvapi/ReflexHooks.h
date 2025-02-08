#pragma once

#include <d3d12.h>
#include "NvApiTypes.h"

class ReflexHooks {
    inline static bool _inited = false;
    inline static uint32_t _minimumIntervalUs = 0;
    inline static NV_SET_SLEEP_MODE_PARAMS _lastSleepParams{};
    inline static IUnknown* _lastSleepDev = nullptr;
    inline static bool _dlssgDetected = false;
    inline static uint64_t _lastAsyncMarkerFrameId = 0;
    inline static uint64_t _updatesWithoutMarker = 0;

    inline static decltype(&NvAPI_D3D_SetSleepMode) o_NvAPI_D3D_SetSleepMode = nullptr;
    inline static decltype(&NvAPI_D3D_Sleep) o_NvAPI_D3D_Sleep = nullptr;
    inline static decltype(&NvAPI_D3D_GetLatency) o_NvAPI_D3D_GetLatency = nullptr;
    inline static decltype(&NvAPI_D3D_SetLatencyMarker) o_NvAPI_D3D_SetLatencyMarker = nullptr;
    inline static decltype(&NvAPI_D3D12_SetAsyncFrameMarker) o_NvAPI_D3D12_SetAsyncFrameMarker = nullptr;

    static NvAPI_Status hkNvAPI_D3D_SetSleepMode(IUnknown* pDev, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams);
    static NvAPI_Status hkNvAPI_D3D_Sleep(IUnknown* pDev);
    static NvAPI_Status hkNvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams);
    static NvAPI_Status hkNvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams);
    static NvAPI_Status hkNvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams);

public:
    static void hookReflex(NvApiTypes::PFN_NvApi_QueryInterface& queryInterface);
    static bool isDlssgDetected();
    static bool isReflexHooked();
    static void* getHookedReflex(unsigned int InterfaceId);

    // For updating information about Reflex hooks
    static void update(bool optiFg_FgState);

    // 0 - disables the fps cap
    inline static void setFPSLimit(float fps);
};