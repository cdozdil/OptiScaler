#pragma once

#include "pch.h"

#include "Config.h"

#include "detours/detours.h"

typedef LONG (*PFN_WinVerifyTrust)(HWND hwnd, GUID* pgActionID, LPVOID pWVTData);
static PFN_WinVerifyTrust o_WinVerifyTrust = nullptr;

static LONG hkWinVerifyTrust(HWND hwnd, GUID* pgActionID, LPVOID pWVTData)
{
    if (State::Instance().fsr4loading)
        return ERROR_SUCCESS;

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
