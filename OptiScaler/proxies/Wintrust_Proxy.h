#pragma once

#include "pch.h"

#include "Config.h"

#include "detours/detours.h"
#include <WinTrust.h>
#include <Softpub.h>

typedef LONG (*PFN_WinVerifyTrust)(HWND hwnd, GUID* pgActionID, LPVOID pWVTData);
static PFN_WinVerifyTrust o_WinVerifyTrust = nullptr;

static LONG hkWinVerifyTrust(HWND hwnd, GUID* pgActionID, LPVOID pWVTData)
{
    if (!pWVTData || !IsEqualGUID(*pgActionID, WINTRUST_ACTION_GENERIC_VERIFY_V2))
        return o_WinVerifyTrust(hwnd, pgActionID, pWVTData);

    const auto data = reinterpret_cast<WINTRUST_DATA*>(pWVTData);

    if (!data->pFile || !data->pFile->pcwszFilePath)
        return o_WinVerifyTrust(hwnd, pgActionID, pWVTData);

    const auto path = wstring_to_string(std::wstring(data->pFile->pcwszFilePath));

    if (path.contains("amd_fidelityfx_dx12.dll") || path.contains("amd_fidelityfx_vk.dll"))
    {
        return ERROR_SUCCESS;
    }

    return o_WinVerifyTrust(hwnd, pgActionID, pWVTData);
}

static void hookWintrust()
{
    LOG_FUNC();

    o_WinVerifyTrust = reinterpret_cast<PFN_WinVerifyTrust>(DetourFindFunction("Wintrust.dll", "WinVerifyTrust"));

    if (o_WinVerifyTrust != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&) o_WinVerifyTrust, hkWinVerifyTrust);

        DetourTransactionCommit();
    }
}

static void unhookWintrust()
{
    if (o_WinVerifyTrust != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&(PVOID&) o_WinVerifyTrust, hkWinVerifyTrust);
        o_WinVerifyTrust = nullptr;

        DetourTransactionCommit();
    }
}
