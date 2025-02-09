#pragma once

#include <pch.h>
#include <dxgi.h>
#include <d3d12.h>
#include <nvapi.h>

#define GET_ID(name) NvApiTypes::Instance().getId(#name)

typedef void* (__stdcall* PFN_NvApi_QueryInterface)(unsigned int InterfaceId);
typedef NvAPI_Status(__stdcall* PFN_Fake_InformFGState)(bool fg_state);
typedef NvAPI_Status(__stdcall* PFN_Fake_InformPresentFG)(bool frame_interpolated, uint64_t reflex_frame_id);
typedef NvAPI_Status(__stdcall* PFN_Fake_GetAntiLagCtx)(void* al2_context);

// Separate to break up a circular dependency 
class NvApiTypes {
    std::unordered_map<std::string, unsigned int> lookupTable;

    NvApiTypes();
public:
    static NvApiTypes& Instance();
    unsigned int getId(const std::string& name) const;
};
