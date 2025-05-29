#pragma once

#include "pch.h"

#include "Config.h"

#include "detours/detours.h"
#include <wincrypt.h>

typedef BOOL (*PFN_CryptQueryObject)(DWORD dwObjectType, const void* pvObject, DWORD dwExpectedContentTypeFlags,
                                     DWORD dwExpectedFormatTypeFlags, DWORD dwFlags, DWORD* pdwMsgAndCertEncodingType,
                                     DWORD* pdwContentType, DWORD* pdwFormatType, HCERTSTORE* phCertStore,
                                     HCRYPTMSG* phMsg, const void** ppvContext);
static PFN_CryptQueryObject o_CryptQueryObject = nullptr;

static BOOL hkCryptQueryObject(DWORD dwObjectType, const void* pvObject, DWORD dwExpectedContentTypeFlags,
                               DWORD dwExpectedFormatTypeFlags, DWORD dwFlags, DWORD* pdwMsgAndCertEncodingType,
                               DWORD* pdwContentType, DWORD* pdwFormatType, HCERTSTORE* phCertStore, HCRYPTMSG* phMsg,
                               const void** ppvContext)
{
    if (State::Instance().fsr4loading) // TODO: make this check better, only when required? check the dll that is being attempted?
    {
        LOG_DEBUG("Replacing FFX with a signed dll");
        WCHAR path[256] {};
        GetModuleFileNameW(fsr4Module, path, 256);

        return o_CryptQueryObject(dwObjectType, path, dwExpectedContentTypeFlags, dwExpectedFormatTypeFlags,
                                         dwFlags, pdwMsgAndCertEncodingType, pdwContentType, pdwFormatType, phCertStore,
                                         phMsg, ppvContext);
    }

    return o_CryptQueryObject(dwObjectType, pvObject, dwExpectedContentTypeFlags, dwExpectedFormatTypeFlags, dwFlags,
                              pdwMsgAndCertEncodingType, pdwContentType, pdwFormatType, phCertStore, phMsg, ppvContext);
}

static void hookCrypt32()
{
    LOG_FUNC();

    o_CryptQueryObject = reinterpret_cast<PFN_CryptQueryObject>(DetourFindFunction("crypt32.dll", "CryptQueryObject"));

    if (o_CryptQueryObject != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&) o_CryptQueryObject, hkCryptQueryObject);

        DetourTransactionCommit();
    }
}

static void unhookCrypt32()
{
    if (o_CryptQueryObject != nullptr)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourDetach(&(PVOID&) o_CryptQueryObject, hkCryptQueryObject);
        o_CryptQueryObject = nullptr;

        DetourTransactionCommit();
    }
}
