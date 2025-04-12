#pragma once

#include <pch.h>

#define DEFINE_NAME_VECTORS(varName, libName) \
    inline std::vector<std::string> varName##Names = { libName ".dll", libName }; \
    inline std::vector<std::wstring> varName##NamesW = { L##libName L".dll", L##libName };

inline std::vector<std::string> dllNames;
inline std::vector<std::wstring> dllNamesW;

inline std::vector<std::string> overlayNames = { "eosovh-win32-shipping.dll", "eosovh-win32-shipping", "eosovh-win64-shipping.dll", "eosovh-win64-shipping",
                                                 "gameoverlayrenderer64", "gameoverlayrenderer64.dll", "gameoverlayrenderer", "gameoverlayrenderer.dll",
                                                 "overlay64", "overlay64.dll", "overlay", "overlay.dll",
                                                 "owclient.dll", "owclient",  };
inline std::vector<std::wstring> overlayNamesW = { L"eosovh-win32-shipping.dll", L"eosovh-win32-shipping", L"eosovh-win64-shipping.dll", L"eosovh-win64-shipping",
                                                   L"gameoverlayrenderer64", L"gameoverlayrenderer64.dll", L"gameoverlayrenderer", L"gameoverlayrenderer.dll", 
                                                   L"overlay64", L"overlay64.dll", L"overlay", L"overlay.dll",
                                                   L"owclient.dll", L"owclient" };

DEFINE_NAME_VECTORS(nvngx, "nvngx");
DEFINE_NAME_VECTORS(xess, "libxess");
DEFINE_NAME_VECTORS(nvngxDlss, "nvngx_dlss");
DEFINE_NAME_VECTORS(nvapi, "nvapi64");
DEFINE_NAME_VECTORS(dx11, "d3d11");
DEFINE_NAME_VECTORS(dx12, "d3d12");
DEFINE_NAME_VECTORS(dxgi, "dxgi");
DEFINE_NAME_VECTORS(vk, "vulkan-1");
DEFINE_NAME_VECTORS(streamline, "sl.interposer");

DEFINE_NAME_VECTORS(fsr2, "ffx_fsr2_api_x64");
DEFINE_NAME_VECTORS(fsr2BE, "ffx_fsr2_api_dx12_x64");

DEFINE_NAME_VECTORS(fsr3, "ffx_fsr3upscaler_x64");
DEFINE_NAME_VECTORS(fsr3BE, "ffx_backend_dx12_x64");

DEFINE_NAME_VECTORS(ffxDx12, "amd_fidelityfx_dx12");
DEFINE_NAME_VECTORS(ffxVk, "amd_fidelityfx_vk");

inline static bool CheckDllName(std::string* dllName, std::vector<std::string>* namesList)
{
    for (size_t i = 0; i < namesList->size(); i++)
    {
        auto name = namesList->at(i);
        auto pos = dllName->rfind(name);

        if (pos != std::string::npos && pos == (dllName->size() - name.size()))
            return true;
    }

    return false;
}

inline static bool CheckDllNameW(std::wstring* dllName, std::vector<std::wstring>* namesList)
{
    for (size_t i = 0; i < namesList->size(); i++)
    {
        auto name = namesList->at(i);
        auto pos = dllName->rfind(name);

        if (pos != std::string::npos && pos == (dllName->size() - name.size()))
            return true;
    }

    return false;
}
