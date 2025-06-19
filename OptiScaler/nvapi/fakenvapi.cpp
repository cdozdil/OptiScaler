#include "fakenvapi.h"
#include "proxies/FfxApi_Proxy.h"

void* fakenvapi::_lowLatencyContext = nullptr;
Mode fakenvapi::_lowLatencyMode = Mode::LatencyFlex;

void fakenvapi::Init(PFN_NvApi_QueryInterface& queryInterface)
{
    if (_inited)
        return;

    LOG_INFO("Trying to get fakenvapi-specific functions");

    Fake_InformFGState = static_cast<decltype(Fake_InformFGState)>(queryInterface(GET_ID(Fake_InformFGState)));
    Fake_InformPresentFG = static_cast<decltype(Fake_InformPresentFG)>(queryInterface(GET_ID(Fake_InformPresentFG)));
    Fake_GetAntiLagCtx = static_cast<decltype(Fake_GetAntiLagCtx)>(queryInterface(GET_ID(Fake_GetAntiLagCtx)));
    Fake_GetLowLatencyCtx = static_cast<decltype(Fake_GetLowLatencyCtx)>(queryInterface(GET_ID(Fake_GetLowLatencyCtx)));

    if (Fake_InformFGState == nullptr)
        LOG_INFO("Couldn't get InformFGState");

    if (Fake_InformPresentFG == nullptr)
        LOG_INFO("Couldn't get InformPresentFG");

    if (Fake_GetAntiLagCtx == nullptr)
        LOG_INFO("Couldn't get GetAntiLagCtx");

    if (Fake_GetLowLatencyCtx == nullptr)
        LOG_INFO("Couldn't get GetLowLatencyCtx");

    _inited = true;
}

// Inform AntiLag 2 when present of interpolated frames starts
void fakenvapi::reportFGPresent(IDXGISwapChain* pSwapChain, bool fg_state, bool frame_interpolated)
{
    if (!isUsingFakenvapi())
        return;

    // Lets fakenvapi log and reset correctly
    Fake_InformFGState(fg_state);

    if (fg_state)
    {
        // Starting with FSR 3.1.1 we can provide an AntiLag 2 context to FSR FG
        // and it will call SetFrameGenFrameType for us
        auto static ffxApiVersion = FfxApiProxy::VersionDx12();
        constexpr feature_version requiredVersion = { 3, 1, 1 };
        if (ffxApiVersion >= requiredVersion && updateModeAndContext())
        {
            if (_lowLatencyContext != nullptr && _lowLatencyMode == Mode::AntiLag2)
            {
                antilag2_data.context = _lowLatencyContext;
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
    else
    {
        // Remove it or other FG mods won't work with AL2
        pSwapChain->SetPrivateData(IID_IFfxAntiLag2Data, 0, nullptr);
    }
}

bool fakenvapi::updateModeAndContext()
{
    if (!isUsingFakenvapi())
        return false;

    LOG_FUNC();

    if (Fake_GetLowLatencyCtx)
    {
        auto result = Fake_GetLowLatencyCtx(&_lowLatencyContext, &_lowLatencyMode);

        if (result != NVAPI_OK)
            LOG_ERROR("Can't get Low Latency context from fakenvapi");

        return result == NVAPI_OK;
    }

    // fallback for older fakenvapi builds
    if (Fake_GetAntiLagCtx)
    {
        auto result = Fake_GetAntiLagCtx(&_lowLatencyContext);

        if (result != NVAPI_OK)
            _lowLatencyMode = Mode::LatencyFlex;
        else
            _lowLatencyMode = Mode::AntiLag2;

        return result == NVAPI_OK;
    }

    _lowLatencyContext = nullptr;
    _lowLatencyMode = Mode::LatencyFlex;

    return false;
}

// updateModeAndContext needs to be called before that
Mode fakenvapi::getCurrentMode() { return _lowLatencyMode; }

// won't work with older fakenvapi builds
bool fakenvapi::isUsingFakenvapi() { return Fake_InformFGState || Fake_InformPresentFG; }
