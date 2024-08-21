#pragma once
#include <pch.h>

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
        DllCanUnloadNow = GetProcAddress(module, "DllCanUnloadNow");
        DllGetClassObject = GetProcAddress(module, "DllGetClassObject");
        DllRegisterServer = GetProcAddress(module, "DllRegisterServer");
        DllUnregisterServer = GetProcAddress(module, "DllUnregisterServer");
        DebugSetMute = GetProcAddress(module, "DebugSetMute");
    }
} shared;

void _DllCanUnloadNow() { shared.DllCanUnloadNow(); }
void _DllGetClassObject() { shared.DllGetClassObject(); }
void _DllRegisterServer() { shared.DllRegisterServer(); }
void _DllUnregisterServer() { shared.DllUnregisterServer(); }
void _DebugSetMute() { shared.DebugSetMute(); }