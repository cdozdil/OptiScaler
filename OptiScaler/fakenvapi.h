#pragma once

#include "NVNGX_Proxy.h"

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
    inline static bool _inited = false;
public:
    typedef void* (__stdcall* PFN_NvApi_QueryInterface)(FN_INTERFACE InterfaceId);

    inline static PFN_Fake_InformFGState Fake_InformFGState = nullptr;
    inline static PFN_Fake_InformPresentFG Fake_InformPresentFG = nullptr;
    inline static PFN_Fake_GetAntiLagCtx Fake_GetAntiLagCtx = nullptr;

    inline static void Init(PFN_NvApi_QueryInterface &queryInterface) {
        if (_inited) return;

        LOG_INFO("Getting fakenvapi-specific functions");

        Fake_InformFGState = static_cast<PFN_Fake_InformFGState>(queryInterface(FN_INTERFACE::Fake_InformFGState));
        Fake_InformPresentFG = static_cast<PFN_Fake_InformPresentFG>(queryInterface(FN_INTERFACE::Fake_InformPresentFG));
        Fake_GetAntiLagCtx = static_cast<PFN_Fake_GetAntiLagCtx>(queryInterface(FN_INTERFACE::Fake_GetAntiLagCtx));

        _inited = true;
    }
};