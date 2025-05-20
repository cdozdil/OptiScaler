#pragma once

#include <pch.h>

#include "shared.h"

#include <proxies/KernelBase_Proxy.h>

struct version_dll
{
    HMODULE dll;
    FARPROC GetFileVersionInfoA;
    FARPROC GetFileVersionInfoByHandle;
    FARPROC GetFileVersionInfoExA;
    FARPROC GetFileVersionInfoExW;
    FARPROC GetFileVersionInfoSizeA;
    FARPROC GetFileVersionInfoSizeExA;
    FARPROC GetFileVersionInfoSizeExW;
    FARPROC GetFileVersionInfoSizeW;
    FARPROC GetFileVersionInfoW;
    FARPROC VerFindFileA;
    FARPROC VerFindFileW;
    FARPROC VerInstallFileA;
    FARPROC VerInstallFileW;
    FARPROC VerLanguageNameA;
    FARPROC VerLanguageNameW;
    FARPROC VerQueryValueA;
    FARPROC VerQueryValueW;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);

        GetFileVersionInfoA = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoA");
        GetFileVersionInfoByHandle = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoByHandle");
        GetFileVersionInfoExA = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoExA");
        GetFileVersionInfoExW = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoExW");
        GetFileVersionInfoSizeA = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoSizeA");
        GetFileVersionInfoSizeExA = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoSizeExA");
        GetFileVersionInfoSizeExW = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoSizeExW");
        GetFileVersionInfoSizeW = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoSizeW");
        GetFileVersionInfoW = KernelBaseProxy::GetProcAddress_()(dll, "GetFileVersionInfoW");
        VerFindFileA = KernelBaseProxy::GetProcAddress_()(dll, "VerFindFileA");
        VerFindFileW = KernelBaseProxy::GetProcAddress_()(dll, "VerFindFileW");
        VerInstallFileA = KernelBaseProxy::GetProcAddress_()(dll, "VerInstallFileA");
        VerInstallFileW = KernelBaseProxy::GetProcAddress_()(dll, "VerInstallFileW");
        VerLanguageNameA = KernelBaseProxy::GetProcAddress_()(dll, "VerLanguageNameA");
        VerLanguageNameW = KernelBaseProxy::GetProcAddress_()(dll, "VerLanguageNameW");
        VerQueryValueA = KernelBaseProxy::GetProcAddress_()(dll, "VerQueryValueA");
        VerQueryValueW = KernelBaseProxy::GetProcAddress_()(dll, "VerQueryValueW");
    }
} version;

void _GetFileVersionInfoA() { version.GetFileVersionInfoA(); }
void _GetFileVersionInfoByHandle() { version.GetFileVersionInfoByHandle(); }
void _GetFileVersionInfoExA() { version.GetFileVersionInfoExA(); }
void _GetFileVersionInfoExW() { version.GetFileVersionInfoExW(); }
void _GetFileVersionInfoSizeA() { version.GetFileVersionInfoSizeA(); }
void _GetFileVersionInfoSizeExA() { version.GetFileVersionInfoSizeExA(); }
void _GetFileVersionInfoSizeExW() { version.GetFileVersionInfoSizeExW(); }
void _GetFileVersionInfoSizeW() { version.GetFileVersionInfoSizeW(); }
void _GetFileVersionInfoW() { version.GetFileVersionInfoW(); }
void _VerFindFileA() { version.VerFindFileA(); }
void _VerFindFileW() { version.VerFindFileW(); }
void _VerInstallFileA() { version.VerInstallFileA(); }
void _VerInstallFileW() { version.VerInstallFileW(); }
void _VerLanguageNameA() { version.VerLanguageNameA(); }
void _VerLanguageNameW() { version.VerLanguageNameW(); }
void _VerQueryValueA() { version.VerQueryValueA(); }
void _VerQueryValueW() { version.VerQueryValueW(); }
