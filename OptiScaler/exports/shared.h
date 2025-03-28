#pragma once

#include <pch.h>

#include <proxies/KernelBase_Proxy.h>

struct shared
{
    HMODULE dll;
    FARPROC DllCanUnloadNow;
    FARPROC DllGetClassObject;
    FARPROC DllRegisterServer;
    FARPROC DllUnregisterServer;
    FARPROC DebugSetMute;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        DllCanUnloadNow = KernelBaseProxy::GetProcAddress_()(module, "DllCanUnloadNow");
        DllGetClassObject = KernelBaseProxy::GetProcAddress_()(module, "DllGetClassObject");
        DllRegisterServer = KernelBaseProxy::GetProcAddress_()(module, "DllRegisterServer");
        DllUnregisterServer = KernelBaseProxy::GetProcAddress_()(module, "DllUnregisterServer");
        DebugSetMute = KernelBaseProxy::GetProcAddress_()(module, "DebugSetMute");
    }
} shared;

void _DllCanUnloadNow() { shared.DllCanUnloadNow(); }
void _DllGetClassObject() { shared.DllGetClassObject(); }
void _DllRegisterServer() { shared.DllRegisterServer(); }
void _DllUnregisterServer() { shared.DllUnregisterServer(); }
void _DebugSetMute() { shared.DebugSetMute(); }
