// Version 1.3.0

#pragma once

#include <cstdint>
#include <nvapi.h>

struct FAKENVAPI_INTERFACE_TABLE { const char * func; unsigned int id; };
inline struct FAKENVAPI_INTERFACE_TABLE fakenvapi_interface_table[] = {
    { "Fake_GetLatency", 0x21372137 },
    { "Fake_InformFGState", 0x21382138 },
    { "Fake_InformPresentFG", 0x21392139 },
    { "Fake_GetAntiLagCtx", 0x21402140 },
    { "Fake_GetLowLatencyCtx", 0x21412141 }
};

enum class Mode {
    AntiLag2,
    LatencyFlex,
    XeLL,
    AntiLagVk
};

// Removed in 1.3.0
NvAPI_Status __cdecl Fake_GetLatency(uint64_t* call_spot, uint64_t* target, uint64_t* latency, uint64_t* frame_time);

NvAPI_Status __cdecl Fake_InformFGState(bool fg_state);

NvAPI_Status __cdecl Fake_InformPresentFG(bool frame_interpolated, uint64_t reflex_frame_id);

// Deprecated
NvAPI_Status __cdecl Fake_GetAntiLagCtx(void** antilag2_context);

NvAPI_Status __cdecl Fake_GetLowLatencyCtx(void** low_latency_context, Mode* mode);