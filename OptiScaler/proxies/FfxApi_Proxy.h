#pragma once

#include "pch.h"
#include "Util.h"
#include "Config.h"
#include "Logger.h"

#include <inputs/FfxApi_Dx12.h>

#include "ffx_api.h"

#include "detours/detours.h"

class FfxApiProxy
{
private:
    inline static HMODULE _dllDx12 = nullptr;
    inline static feature_version _versionDx12{ 0, 0, 0 };
    inline static PfnFfxCreateContext _D3D12_CreateContext = nullptr;
    inline static PfnFfxDestroyContext _D3D12_DestroyContext = nullptr;
    inline static PfnFfxConfigure _D3D12_Configure = nullptr;
    inline static PfnFfxQuery _D3D12_Query = nullptr;
    inline static PfnFfxDispatch _D3D12_Dispatch = nullptr;

    inline static HMODULE _dllVk = nullptr;
    inline static feature_version _versionVk{ 0, 0, 0 };
    inline static PfnFfxCreateContext _VULKAN_CreateContext = nullptr;
    inline static PfnFfxDestroyContext _VULKAN_DestroyContext = nullptr;
    inline static PfnFfxConfigure _VULKAN_Configure = nullptr;
    inline static PfnFfxQuery _VULKAN_Query = nullptr;
    inline static PfnFfxDispatch _VULKAN_Dispatch = nullptr;

    static inline void parse_version(const char* version_str, feature_version* _version)
    {
        if (sscanf_s(version_str, "%u.%u.%u", &_version->major, &_version->minor, &_version->patch) != 3)
            LOG_WARN("can't parse {0}", version_str);
    }

public:
    static bool InitFfxDx12()
    {
        // if dll already loaded
        if (_dllDx12 != nullptr || _D3D12_CreateContext != nullptr)
            return true;

        spdlog::info("");

        State::Instance().upscalerDisableHook = true;

        LOG_DEBUG("Loading amd_fidelityfx_dx12.dll methods");

        auto file = Util::DllPath().parent_path() / "amd_fidelityfx_dx12.dll";
        LOG_INFO("Trying to load {}", file.string());

        _dllDx12 = LoadLibrary(file.wstring().c_str());
        if (_dllDx12 != nullptr)
        {
            _D3D12_Configure = (PfnFfxConfigure)GetProcAddress(_dllDx12, "ffxConfigure");
            _D3D12_CreateContext = (PfnFfxCreateContext)GetProcAddress(_dllDx12, "ffxCreateContext");
            _D3D12_DestroyContext = (PfnFfxDestroyContext)GetProcAddress(_dllDx12, "ffxDestroyContext");
            _D3D12_Dispatch = (PfnFfxDispatch)GetProcAddress(_dllDx12, "ffxDispatch");
            _D3D12_Query = (PfnFfxQuery)GetProcAddress(_dllDx12, "ffxQuery");
        }

        if (_D3D12_CreateContext == nullptr)
        {
            LOG_INFO("Trying to load amd_fidelityfx_dx12.dll with detours");

            _D3D12_Configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxConfigure");
            _D3D12_CreateContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxCreateContext");
            _D3D12_DestroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDestroyContext");
            _D3D12_Dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxDispatch");
            _D3D12_Query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_dx12.dll", "ffxQuery");
        }

        if (_D3D12_CreateContext != nullptr)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());

            if (_D3D12_Configure != nullptr)
                DetourAttach(&(PVOID&)_D3D12_Configure, ffxConfigure_Dx12);

            if (_D3D12_CreateContext != nullptr)
                DetourAttach(&(PVOID&)_D3D12_CreateContext, ffxCreateContext_Dx12);

            if (_D3D12_DestroyContext != nullptr)
                DetourAttach(&(PVOID&)_D3D12_DestroyContext, ffxDestroyContext_Dx12);

            if (_D3D12_Dispatch != nullptr)
                DetourAttach(&(PVOID&)_D3D12_Dispatch, ffxDispatch_Dx12);

            if (_D3D12_Query != nullptr)
                DetourAttach(&(PVOID&)_D3D12_Query, ffxQuery_Dx12);

            State::Instance().fsrHooks = true;

            DetourTransactionCommit();
        }

        State::Instance().upscalerDisableHook = false;

        bool loadResult = _D3D12_CreateContext != nullptr;

        LOG_INFO("LoadResult: {}", loadResult);

        if (loadResult)
            VersionDx12();

        return loadResult;
    }

    static feature_version VersionDx12()
    {
        if (_versionDx12.major == 0 && _D3D12_Query != nullptr/* && device != nullptr*/)
        {
            ffxQueryDescGetVersions versionQuery{};
            versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
            versionQuery.createDescType = 0x00010000u; // FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE
            uint64_t versionCount = 0;
            versionQuery.outputCount = &versionCount;

            auto queryResult = _D3D12_Query(nullptr, &versionQuery.header);

            // get number of versions for allocation
            if (versionCount > 0 && queryResult == FFX_API_RETURN_OK)
            {

                std::vector<uint64_t> versionIds;
                std::vector<const char*> versionNames;
                versionIds.resize(versionCount);
                versionNames.resize(versionCount);
                versionQuery.versionIds = versionIds.data();
                versionQuery.versionNames = versionNames.data();

                // fill version ids and names arrays.
                queryResult = _D3D12_Query(nullptr, &versionQuery.header);

                if (queryResult == FFX_API_RETURN_OK)
                {
                    parse_version(versionNames[0], &_versionDx12);
                    LOG_INFO("FfxApi Dx12 version: {}.{}.{}", _versionDx12.major, _versionDx12.minor, _versionDx12.patch);
                }
                else
                {
                    LOG_WARN("_D3D12_Query 2 result: {}", (UINT)queryResult);
                }
            }
            else
            {
                LOG_WARN("_D3D12_Query result: {}", (UINT)queryResult);
            }
        }

        return _versionDx12;
    }

    static PfnFfxCreateContext D3D12_CreateContext() { return _D3D12_CreateContext; }
    static PfnFfxDestroyContext D3D12_DestroyContext() { return _D3D12_DestroyContext; }
    static PfnFfxConfigure D3D12_Configure() { return _D3D12_Configure; }
    static PfnFfxQuery D3D12_Query() { return _D3D12_Query; }
    static PfnFfxDispatch D3D12_Dispatch() { return _D3D12_Dispatch; }

    static bool InitFfxVk()
    {
        // if dll already loaded
        if (_dllVk != nullptr || _VULKAN_CreateContext != nullptr)
            return true;

        spdlog::info("");

        State::Instance().upscalerDisableHook = true;

        LOG_DEBUG("Loading amd_fidelityfx_vk.dll methods");

        auto file = Util::DllPath().parent_path() / "amd_fidelityfx_vk.dll";
        LOG_INFO("Trying to load {}", file.string());

        _dllVk = LoadLibrary(file.wstring().c_str());
        if (_dllVk != nullptr)
        {
            _VULKAN_Configure = (PfnFfxConfigure)GetProcAddress(_dllVk, "ffxConfigure");
            _VULKAN_CreateContext = (PfnFfxCreateContext)GetProcAddress(_dllVk, "ffxCreateContext");
            _VULKAN_DestroyContext = (PfnFfxDestroyContext)GetProcAddress(_dllVk, "ffxDestroyContext");
            _VULKAN_Dispatch = (PfnFfxDispatch)GetProcAddress(_dllVk, "ffxDispatch");
            _VULKAN_Query = (PfnFfxQuery)GetProcAddress(_dllVk, "ffxQuery");
        }

        if (_VULKAN_CreateContext == nullptr)
        {
            LOG_INFO("Trying to load amd_fidelityfx_vk.dll with detours");

            _VULKAN_Configure = (PfnFfxConfigure)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxConfigure");
            _VULKAN_CreateContext = (PfnFfxCreateContext)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxCreateContext");
            _VULKAN_DestroyContext = (PfnFfxDestroyContext)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxDestroyContext");
            _VULKAN_Dispatch = (PfnFfxDispatch)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxDispatch");
            _VULKAN_Query = (PfnFfxQuery)DetourFindFunction("amd_fidelityfx_vk.dll", "ffxQuery");
        }

        //Config::Instance()->dxgiSkipSpoofing = false;
        State::Instance().upscalerDisableHook = false;

        bool loadResult = _VULKAN_CreateContext != nullptr;

        LOG_INFO("LoadResult: {}", loadResult);

        if (loadResult)
            VersionVk();

        return loadResult;
    }

    static feature_version VersionVk()
    {
        if (_versionVk.major == 0 && _VULKAN_Query != nullptr)
        {
            ffxQueryDescGetVersions versionQuery{};
            versionQuery.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
            versionQuery.createDescType = 0x00010000u; // FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE
            uint64_t versionCount = 0;
            versionQuery.outputCount = &versionCount;

            auto queryResult = _VULKAN_Query(nullptr, &versionQuery.header);

            // get number of versions for allocation
            if (versionCount > 0 && queryResult == FFX_API_RETURN_OK)
            {

                std::vector<uint64_t> versionIds;
                std::vector<const char*> versionNames;
                versionIds.resize(versionCount);
                versionNames.resize(versionCount);
                versionQuery.versionIds = versionIds.data();
                versionQuery.versionNames = versionNames.data();

                queryResult = _VULKAN_Query(nullptr, &versionQuery.header);

                if (queryResult == FFX_API_RETURN_OK)
                {
                    parse_version(versionNames[0], &_versionVk);
                    LOG_INFO("FfxApi Vulkan version: {}.{}.{}", _versionVk.major, _versionVk.minor, _versionVk.patch);
                }
                else
                {
                    LOG_WARN("_VULKAN_Query 2 result: {}", (UINT)queryResult);
                }
            }
            else
            {
                LOG_WARN("_VULKAN_Query result: {}", (UINT)queryResult);
            }
        }

        return _versionVk;
    }

    static PfnFfxCreateContext VULKAN_CreateContext() { return _VULKAN_CreateContext; }
    static PfnFfxDestroyContext VULKAN_DestroyContext() { return _VULKAN_DestroyContext; }
    static PfnFfxConfigure VULKAN_Configure() { return _VULKAN_Configure; }
    static PfnFfxQuery VULKAN_Query() { return _VULKAN_Query; }
    static PfnFfxDispatch VULKAN_Dispatch() { return _VULKAN_Dispatch; }

    static std::string ReturnCodeToString(ffxReturnCode_t result)
    {
        switch (result)
        {
            case FFX_API_RETURN_OK: return "The oparation was successful.";
            case FFX_API_RETURN_ERROR: return "An error occurred that is not further specified.";
            case FFX_API_RETURN_ERROR_UNKNOWN_DESCTYPE: return "The structure type given was not recognized for the function or context with which it was used. This is likely a programming error.";
            case FFX_API_RETURN_ERROR_RUNTIME_ERROR: return "The underlying runtime (e.g. D3D12, Vulkan) or effect returned an error code.";
            case FFX_API_RETURN_NO_PROVIDER: return "No provider was found for the given structure type. This is likely a programming error.";
            case FFX_API_RETURN_ERROR_MEMORY: return "A memory allocation failed.";
            case FFX_API_RETURN_ERROR_PARAMETER: return "A parameter was invalid, e.g. a null pointer, empty resource or out-of-bounds enum value.";
            default: return "Unknown";
        }
    }


};