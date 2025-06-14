#pragma once

#include <dxgi.h>
#include <fakenvapi_inc.h>
#include "NvApiTypes.h"

class fakenvapi
{
    inline static struct AntiLag2Data
    {
        void* context;
        bool enabled;
    } antilag2_data;

    inline static decltype(&Fake_InformFGState) Fake_InformFGState = nullptr;
    inline static decltype(&Fake_InformPresentFG) Fake_InformPresentFG = nullptr;
    inline static decltype(&Fake_GetAntiLagCtx) Fake_GetAntiLagCtx = nullptr;
    inline static decltype(&Fake_GetLowLatencyCtx) Fake_GetLowLatencyCtx = nullptr;

    inline static bool _inited = false;
    static void* _lowLatencyContext;
    static Mode _lowLatencyMode;

  public:
    inline static const GUID IID_IFfxAntiLag2Data = {
        0x5083ae5b, 0x8070, 0x4fca, { 0x8e, 0xe5, 0x35, 0x82, 0xdd, 0x36, 0x7d, 0x13 }
    };

    static void Init(PFN_NvApi_QueryInterface& queryInterface);
    static void reportFGPresent(IDXGISwapChain* pSwapChain, bool fg_state, bool frame_interpolated);
    static bool updateModeAndContext();
    static Mode getCurrentMode();
    static bool isUsingFakenvapi();
};
