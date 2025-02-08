#pragma once

#include <pch.h>
#include <dxgi.h>
#include <d3d12.h>
#include <nvapi.h>

#define GET_ID(name) NvApiTypes::Instance().getId(#name)

// Separate to break up a circular dependency 
class NvApiTypes {
    std::unordered_map<std::string, unsigned int> lookupTable;

    NvApiTypes();
public:
    static NvApiTypes& Instance();
    unsigned int getId(const std::string& name) const;
    typedef void* (__stdcall* PFN_NvApi_QueryInterface)(unsigned int InterfaceId);
};
