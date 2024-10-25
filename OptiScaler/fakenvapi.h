#pragma once

#include "NVNGX_Proxy.h"
#include "FfxApi_Proxy.h"
#include <dxgi.h>

enum class FN_INTERFACE : uint32_t
{
    Fake_InformFGState = 0x21382138,
    Fake_InformPresentFG = 0x21392139,
    Fake_GetAntiLagCtx = 0x21402140
};

using PFN_Fake_InformFGState = uint32_t(__stdcall*)(bool fg_state);
using PFN_Fake_InformPresentFG = uint32_t(__stdcall*)(bool frame_interpolated, uint64_t reflex_frame_id);
using PFN_Fake_GetAntiLagCtx = uint32_t(__stdcall*)(void* al2_context);

class fakenvapi
{
private:
    inline static struct AntiLag2Data
    {
        void* context;
        bool enabled;
    } antilag2_data;

    inline static PFN_Fake_InformFGState Fake_InformFGState = nullptr;
    inline static PFN_Fake_InformPresentFG Fake_InformPresentFG = nullptr;
    inline static PFN_Fake_GetAntiLagCtx Fake_GetAntiLagCtx = nullptr;

    inline static bool _inited = false;
public:
    typedef void* (__stdcall* PFN_Fake_QueryInterface)(FN_INTERFACE InterfaceId);

    inline static const GUID IID_IFfxAntiLag2Data = { 0x5083ae5b, 0x8070, 0x4fca, {0x8e, 0xe5, 0x35, 0x82, 0xdd, 0x36, 0x7d, 0x13} };

    inline static void Init(PFN_Fake_QueryInterface &queryInterface) {
        if (_inited) return;

        LOG_INFO("Getting fakenvapi-specific functions");

        Fake_InformFGState = static_cast<PFN_Fake_InformFGState>(queryInterface(FN_INTERFACE::Fake_InformFGState));
        Fake_InformPresentFG = static_cast<PFN_Fake_InformPresentFG>(queryInterface(FN_INTERFACE::Fake_InformPresentFG));
        Fake_GetAntiLagCtx = static_cast<PFN_Fake_GetAntiLagCtx>(queryInterface(FN_INTERFACE::Fake_GetAntiLagCtx));

        _inited = true;
    }

    // Inform AntiLag 2 when present of interpolated frames starts
    inline static void reportFGPresent(IDXGISwapChain* pSwapChain, bool fg_state, bool frame_interpolated) {
        if (Fake_InformFGState == nullptr || Fake_InformPresentFG == nullptr || Fake_GetAntiLagCtx == nullptr) return;

        // Lets fakenvapi log and reset correctly
        Fake_InformFGState(fg_state);

        if (fg_state) {
            // Starting with FSR 3.1.1 we can provide an AntiLag 2 context to FSR FG
            // and it will call SetFrameGenFrameType for us
            auto static ffxApiVersion = FfxApiProxy::VersionDx12();
            constexpr feature_version requiredVersion = { 3, 1, 1 };
            if (isVersionOrBetter(ffxApiVersion, requiredVersion)) {
                void* antilag2Context = nullptr;
                Fake_GetAntiLagCtx(&antilag2Context);

                if (antilag2Context != nullptr) {
                    antilag2_data.context = antilag2Context;
                    antilag2_data.enabled = true;

                    pSwapChain->SetPrivateData(IID_IFfxAntiLag2Data, sizeof(antilag2_data), &antilag2_data);
                }
            }
            else
            {
                // Tell fakenvapi to call SetFrameGenFrameType by itself
                // Reflex frame id might get used in the future
                Fake_InformPresentFG(frame_interpolated, 0);
            }
        }
        else {
            // Remove it or other FG mods won't work with AL2
            pSwapChain->SetPrivateData(IID_IFfxAntiLag2Data, 0, nullptr);
        }
    }
};