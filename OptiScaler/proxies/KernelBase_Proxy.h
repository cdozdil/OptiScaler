#pragma once

#include <pch.h>

#include <detours/detours.h>

class KernelBaseProxy
{
  public:
    typedef BOOL (*PFN_FreeLibrary)(HMODULE lpLibrary);
    typedef HMODULE (*PFN_LoadLibraryA)(LPCSTR lpLibFileName);
    typedef HMODULE (*PFN_LoadLibraryW)(LPCWSTR lpLibFileName);
    typedef HMODULE (*PFN_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    typedef HMODULE (*PFN_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    typedef FARPROC (*PFN_GetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
    typedef HMODULE (*PFN_GetModuleHandleA)(LPCSTR lpModuleName);
    typedef HMODULE (*PFN_GetModuleHandleW)(LPCWSTR lpModuleName);
    typedef BOOL (*PFN_GetModuleHandleExA)(DWORD dwFlags, LPCSTR lpModuleName, HMODULE* phModule);
    typedef BOOL (*PFN_GetModuleHandleExW)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule);

    static void Init()
    {
        if (_dll != nullptr)
            return;

        _dll = GetModuleHandle(L"kernelbase.dll");

        if (_dll == nullptr)
            _dll = LoadLibrary(L"kernelbase.dll");

        if (_dll == nullptr)
            return;

        _FreeLibrary = (PFN_FreeLibrary) (GetProcAddress(_dll, "FreeLibrary"));
        _LoadLibraryA = (PFN_LoadLibraryA) (GetProcAddress(_dll, "LoadLibraryA"));
        _LoadLibraryW = (PFN_LoadLibraryW) (GetProcAddress(_dll, "LoadLibraryW"));
        _LoadLibraryExA = (PFN_LoadLibraryExA) (GetProcAddress(_dll, "LoadLibraryExA"));
        _LoadLibraryExW = (PFN_LoadLibraryExW) (GetProcAddress(_dll, "LoadLibraryExW"));
        _GetModuleHandleA = (PFN_GetModuleHandleA) (GetProcAddress(_dll, "GetModuleHandleA"));
        _GetModuleHandleW = (PFN_GetModuleHandleW) (GetProcAddress(_dll, "GetModuleHandleW"));
        _GetModuleHandleExA = (PFN_GetModuleHandleExA) (GetProcAddress(_dll, "GetModuleHandleExA"));
        _GetModuleHandleExW = (PFN_GetModuleHandleExW) (GetProcAddress(_dll, "GetModuleHandleExW"));
        _GetProcAddress = (PFN_GetProcAddress) (GetProcAddress(_dll, "GetProcAddress"));
    }

    static HMODULE Module() { return _dll; }

    static PFN_FreeLibrary FreeLibrary_() { return _FreeLibrary; }
    static PFN_LoadLibraryA LoadLibraryA_() { return _LoadLibraryA; }
    static PFN_LoadLibraryW LoadLibraryW_() { return _LoadLibraryW; }
    static PFN_LoadLibraryExA LoadLibraryExA_() { return _LoadLibraryExA; }
    static PFN_LoadLibraryExW LoadLibraryExW_() { return _LoadLibraryExW; }
    static PFN_GetProcAddress GetProcAddress_() { return _GetProcAddress; }
    static PFN_GetModuleHandleA GetModuleHandleA_() { return _GetModuleHandleA; }
    static PFN_GetModuleHandleW GetModuleHandleW_() { return _GetModuleHandleW; }
    static PFN_GetModuleHandleExA GetModuleHandleExA_() { return _GetModuleHandleExA; }
    static PFN_GetModuleHandleExW GetModuleHandleExW_() { return _GetModuleHandleExW; }

    static PFN_FreeLibrary FreeLibrary_Hooked() { return (PFN_FreeLibrary) GetProcAddress_()(_dll, "FreeLibrary"); }
    static PFN_LoadLibraryA LoadLibraryA_Hooked() { return (PFN_LoadLibraryA) _GetProcAddress(_dll, "LoadLibraryA"); }
    static PFN_LoadLibraryW LoadLibraryW_Hooked() { return (PFN_LoadLibraryW) _GetProcAddress(_dll, "LoadLibraryW"); }
    static PFN_LoadLibraryExA LoadLibraryExA_Hooked()
    {
        return (PFN_LoadLibraryExA) _GetProcAddress(_dll, "LoadLibraryExA");
    }
    static PFN_LoadLibraryExW LoadLibraryExW_Hooked()
    {
        return (PFN_LoadLibraryExW) _GetProcAddress(_dll, "LoadLibraryExW");
    }
    static PFN_GetProcAddress GetProcAddress_Hooked()
    {
        return (PFN_GetProcAddress) _GetProcAddress(_dll, "GetProcAddress");
    }
    static PFN_GetModuleHandleA GetModuleHandleA_Hooked()
    {
        return (PFN_GetModuleHandleA) KernelBaseProxy::GetProcAddress_()(_dll, "GetModuleHandleA");
    }
    static PFN_GetModuleHandleW GetModuleHandleW_Hooked()
    {
        return (PFN_GetModuleHandleW) KernelBaseProxy::GetProcAddress_()(_dll, "GetModuleHandleW");
    }
    static PFN_GetModuleHandleExA GetModuleHandleExA_Hooked()
    {
        return (PFN_GetModuleHandleExA) KernelBaseProxy::GetProcAddress_()(_dll, "GetModuleHandleExA");
    }
    static PFN_GetModuleHandleExW GetModuleHandleExW_Hooked()
    {
        return (PFN_GetModuleHandleExW) KernelBaseProxy::GetProcAddress_()(_dll, "GetModuleHandleExW");
    }

    static PFN_FreeLibrary Hook_FreeLibrary(PVOID method)
    {
        auto addr = FreeLibrary_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _FreeLibrary = addr;
        return addr;
    }

    static PFN_LoadLibraryA Hook_LoadLibraryA(PVOID method)
    {
        auto addr = LoadLibraryA_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _LoadLibraryA = addr;
        return addr;
    }

    static PFN_LoadLibraryW Hook_LoadLibraryW(PVOID method)
    {
        auto addr = LoadLibraryW_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _LoadLibraryW = addr;
        return addr;
    }

    static PFN_LoadLibraryExA Hook_LoadLibraryExA(PVOID method)
    {
        auto addr = LoadLibraryExA_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _LoadLibraryExA = addr;
        return addr;
    }

    static PFN_LoadLibraryExW Hook_LoadLibraryExW(PVOID method)
    {
        auto addr = LoadLibraryExW_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _LoadLibraryExW = addr;
        return addr;
    }

    static PFN_GetProcAddress Hook_GetProcAddress(PVOID method)
    {
        auto addr = GetProcAddress_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _GetProcAddress = addr;
        return addr;
    }

    static PFN_GetModuleHandleA Hook_GetModuleHandleA(PVOID method)
    {
        auto addr = GetModuleHandleA_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _GetModuleHandleA = addr;
        return addr;
    }

    static PFN_GetModuleHandleW Hook_GetModuleHandleW(PVOID method)
    {
        auto addr = GetModuleHandleW_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _GetModuleHandleW = addr;
        return addr;
    }

    static PFN_GetModuleHandleExA Hook_GetModuleHandleExA(PVOID method)
    {
        auto addr = GetModuleHandleExA_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _GetModuleHandleExA = addr;
        return addr;
    }

    static PFN_GetModuleHandleExW Hook_GetModuleHandleExW(PVOID method)
    {
        auto addr = GetModuleHandleExW_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&) addr, method);
        DetourTransactionCommit();

        _GetModuleHandleExW = addr;
        return addr;
    }

  private:
    inline static HMODULE _dll = nullptr;

    inline static PFN_FreeLibrary _FreeLibrary = nullptr;
    inline static PFN_LoadLibraryA _LoadLibraryA = nullptr;
    inline static PFN_LoadLibraryW _LoadLibraryW = nullptr;
    inline static PFN_LoadLibraryExA _LoadLibraryExA = nullptr;
    inline static PFN_LoadLibraryExW _LoadLibraryExW = nullptr;
    inline static PFN_GetProcAddress _GetProcAddress = nullptr;
    inline static PFN_GetModuleHandleA _GetModuleHandleA = nullptr;
    inline static PFN_GetModuleHandleW _GetModuleHandleW = nullptr;
    inline static PFN_GetModuleHandleExA _GetModuleHandleExA = nullptr;
    inline static PFN_GetModuleHandleExW _GetModuleHandleExW = nullptr;
};
