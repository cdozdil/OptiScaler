#include "fakenvapi.h"
#include "proxies/FfxApi_Proxy.h"

void fakenvapi::Init(PFN_NvApi_QueryInterface& queryInterface) {
    if (_inited) return;

    LOG_INFO("Trying to get fakenvapi-specific functions");

    Fake_InformFGState = static_cast<PFN_Fake_InformFGState>(queryInterface(GET_ID(Fake_InformFGState)));
    Fake_InformPresentFG = static_cast<PFN_Fake_InformPresentFG>(queryInterface(GET_ID(Fake_InformPresentFG)));
    Fake_GetAntiLagCtx = static_cast<PFN_Fake_GetAntiLagCtx>(queryInterface(GET_ID(Fake_GetAntiLagCtx)));

    if (Fake_InformFGState == nullptr)
        LOG_INFO("Couldn't get InformFGState");
    if (Fake_InformPresentFG == nullptr)
        LOG_INFO("Couldn't get InformPresentFG");
    if (Fake_GetAntiLagCtx == nullptr)
        LOG_INFO("Couldn't get GetAntiLagCtx");

    _inited = true;
}

// Inform AntiLag 2 when present of interpolated frames starts
void fakenvapi::reportFGPresent(IDXGISwapChain* pSwapChain, bool fg_state, bool frame_interpolated) {
    if (!Fake_InformFGState || !Fake_InformPresentFG) return;

    // Lets fakenvapi log and reset correctly
    Fake_InformFGState(fg_state);

    if (fg_state) {
        // Starting with FSR 3.1.1 we can provide an AntiLag 2 context to FSR FG
        // and it will call SetFrameGenFrameType for us
        auto static ffxApiVersion = FfxApiProxy::VersionDx12();
        constexpr feature_version requiredVersion = { 3, 1, 1 };
        if (isVersionOrBetter(ffxApiVersion, requiredVersion) && Fake_GetAntiLagCtx != nullptr) {
            void* antilag2Context = nullptr;
            Fake_GetAntiLagCtx(&antilag2Context);
            LOG_TRACE("Fake_GetAntiLagCtx: {}", antilag2Context != nullptr);

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
            LOG_TRACE("Fake_InformPresentFG: {}", frame_interpolated);
            Fake_InformPresentFG(frame_interpolated, 0);
        }
    }
    else {
        // Remove it or other FG mods won't work with AL2
        pSwapChain->SetPrivateData(IID_IFfxAntiLag2Data, 0, nullptr);
    }
}

// won't work with older fakenvapi builds
bool fakenvapi::isUsingFakenvapi() {
    return Fake_InformFGState || Fake_InformPresentFG || Fake_GetAntiLagCtx;
}