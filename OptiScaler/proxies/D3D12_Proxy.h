#pragma once

#include <pch.h>

#include <proxies/KernelBase_Proxy.h>

#include <detours/detours.h>

#include <d3d12.h>

class D3d12Proxy
{
public:
    typedef struct D3D12_ROOT_SIGNATURE_DESC_L
    {
        UINT NumParameters;
        D3D12_ROOT_PARAMETER* pParameters;
        UINT NumStaticSamplers;
        D3D12_STATIC_SAMPLER_DESC* pStaticSamplers;
        D3D12_ROOT_SIGNATURE_FLAGS Flags;
    } 	D3D12_ROOT_SIGNATURE_DESC_L;

    typedef HRESULT(*PFN_D3D12CreateDevice)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
    typedef HRESULT(*PFN_D3D12SerializeRootSignature)(D3D12_ROOT_SIGNATURE_DESC_L* pRootSignature, D3D_ROOT_SIGNATURE_VERSION Version, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob);
    typedef HRESULT(*PFN_D3D12CreateRootSignatureDeserializer)(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes, REFIID pRootSignatureDeserializerInterface, void** ppRootSignatureDeserializer);
    typedef HRESULT(*PFN_D3D12SerializeVersionedRootSignature)(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob);
    typedef HRESULT(*PFN_D3D12CreateVersionedRootSignatureDeserializer)(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes, REFIID pRootSignatureDeserializerInterface, void** ppRootSignatureDeserializer);
    typedef HRESULT(*PFN_D3D12GetDebugInterface)(REFIID, void**);
    typedef HRESULT(*PFN_D3D12EnableExperimentalFeatures)(UINT NumFeatures, const IID* pIIDs, void* pConfigurationStructs, UINT* pConfigurationStructSizes);
    typedef HRESULT(*PFN_D3D12GetInterface)(REFCLSID, REFIID, void**);

    static void Init(HMODULE module = nullptr)
    {
        if (_dll != nullptr)
            return;

        if (module == nullptr)
        {
            _dll = KernelBaseProxy::GetModuleHandleW_()(L"d3d12.dll");

            if (_dll == nullptr)
                _dll = KernelBaseProxy::LoadLibraryExW_()(L"d3d12.dll", NULL, 0);
        }
        else
        {
            _dll = module;
        }

        if (_dll == nullptr)
            return;

        _D3D12CreateDevice = (PFN_D3D12CreateDevice)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateDevice");
        _D3D12SerializeRootSignature = (PFN_D3D12SerializeRootSignature)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12SerializeRootSignature");
        _D3D12CreateRootSignatureDeserializer = (PFN_D3D12CreateRootSignatureDeserializer)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateRootSignatureDeserializer");
        _D3D12SerializeVersionedRootSignature = (PFN_D3D12SerializeVersionedRootSignature)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12SerializeVersionedRootSignature");
        _D3D12CreateVersionedRootSignatureDeserializer = (PFN_D3D12CreateVersionedRootSignatureDeserializer)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateVersionedRootSignatureDeserializer");
        _D3D12GetDebugInterface = (PFN_D3D12GetDebugInterface)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12GetDebugInterface");
        _D3D12EnableExperimentalFeatures = (PFN_D3D12EnableExperimentalFeatures)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12EnableExperimentalFeatures");
        _D3D12GetInterface = (PFN_D3D12GetInterface)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12GetInterface");
    }

    static HMODULE Module() { return _dll; }

    // Original methods
    static PFN_D3D12CreateDevice D3D12CreateDevice_() { return _D3D12CreateDevice; }
    static PFN_D3D12SerializeRootSignature D3D12SerializeRootSignature_() { return _D3D12SerializeRootSignature; }
    static PFN_D3D12CreateRootSignatureDeserializer D3D12CreateRootSignatureDeserializer_() { return _D3D12CreateRootSignatureDeserializer; }
    static PFN_D3D12SerializeVersionedRootSignature D3D12SerializeVersionedRootSignature_() { return _D3D12SerializeVersionedRootSignature; }
    static PFN_D3D12CreateVersionedRootSignatureDeserializer D3D12CreateVersionedRootSignatureDeserializer_() { return _D3D12CreateVersionedRootSignatureDeserializer; }
    static PFN_D3D12GetDebugInterface D3D12GetDebugInterface_() { return _D3D12GetDebugInterface; }
    static PFN_D3D12EnableExperimentalFeatures D3D12EnableExperimentalFeatures_() { return _D3D12EnableExperimentalFeatures; }
    static PFN_D3D12GetInterface D3D12GetInterface_() { return _D3D12GetInterface; }

    // Hooked methods
    static PFN_D3D12CreateDevice D3D12CreateDevice_Hooked() { return (PFN_D3D12CreateDevice)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateDevice"); }
    static PFN_D3D12SerializeRootSignature D3D12SerializeRootSignature_Hooked() { return (PFN_D3D12SerializeRootSignature)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12SerializeRootSignature"); }
    static PFN_D3D12CreateRootSignatureDeserializer D3D12CreateRootSignatureDeserializer_Hooked() { return (PFN_D3D12CreateRootSignatureDeserializer)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateRootSignatureDeserializer"); }
    static PFN_D3D12SerializeVersionedRootSignature D3D12SerializeVersionedRootSignature_Hooked() { return (PFN_D3D12SerializeVersionedRootSignature)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12SerializeVersionedRootSignature"); }
    static PFN_D3D12CreateVersionedRootSignatureDeserializer D3D12CreateVersionedRootSignatureDeserializer_Hooked() { return (PFN_D3D12CreateVersionedRootSignatureDeserializer)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12CreateVersionedRootSignatureDeserializer"); }
    static PFN_D3D12GetDebugInterface D3D12GetDebugInterface_Hooked() { return (PFN_D3D12GetDebugInterface)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12GetDebugInterface"); }
    static PFN_D3D12EnableExperimentalFeatures D3D12EnableExperimentalFeatures_Hooked() { return (PFN_D3D12EnableExperimentalFeatures)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12EnableExperimentalFeatures"); }
    static PFN_D3D12GetInterface D3D12GetInterface_Hooked() { return (PFN_D3D12GetInterface)KernelBaseProxy::GetProcAddress_()(_dll, "D3D12GetInterface"); }

    // Hook methods
    static PFN_D3D12CreateDevice Hook_D3D12CreateDevice(PVOID method)
    {
        auto addr = D3D12CreateDevice_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12CreateDevice = addr;
        return addr;
    }

    static PFN_D3D12SerializeRootSignature Hook_D3D12SerializeRootSignature(PVOID method) 
    {
        auto addr = D3D12SerializeRootSignature_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12SerializeRootSignature = addr;
        return addr;
    }

    static PFN_D3D12CreateRootSignatureDeserializer Hook_D3D12CreateRootSignatureDeserializer(PVOID method)
    {
        auto addr = D3D12CreateRootSignatureDeserializer_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12CreateRootSignatureDeserializer = addr;
        return addr;
    }

    static PFN_D3D12SerializeVersionedRootSignature Hook_D3D12SerializeVersionedRootSignature(PVOID method)
    {
        auto addr = D3D12SerializeVersionedRootSignature_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12SerializeVersionedRootSignature = addr;
        return addr;
    }

    static PFN_D3D12CreateVersionedRootSignatureDeserializer Hook_D3D12CreateVersionedRootSignatureDeserializer(PVOID method)
    {
        auto addr = D3D12CreateVersionedRootSignatureDeserializer_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12CreateVersionedRootSignatureDeserializer = addr;
        return addr;
    }

    static PFN_D3D12GetDebugInterface Hook_D3D12GetDebugInterface(PVOID method)
    {
        auto addr = D3D12GetDebugInterface_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12GetDebugInterface = addr;
        return addr;
    }

    static PFN_D3D12EnableExperimentalFeatures Hook_D3D12EnableExperimentalFeatures(PVOID method)
    {
        auto addr = D3D12EnableExperimentalFeatures_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12EnableExperimentalFeatures = addr;
        return addr;
    }

    static PFN_D3D12GetInterface Hook_D3D12GetInterface(PVOID method) 
    {
        auto addr = D3D12GetInterface_Hooked();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)addr, method);
        DetourTransactionCommit();

        _D3D12GetInterface = addr;
        return addr;
    }


private:
    inline static HMODULE _dll = nullptr;

    inline static PFN_D3D12CreateDevice _D3D12CreateDevice = nullptr;
    inline static PFN_D3D12SerializeRootSignature _D3D12SerializeRootSignature = nullptr;
    inline static PFN_D3D12CreateRootSignatureDeserializer _D3D12CreateRootSignatureDeserializer = nullptr;
    inline static PFN_D3D12SerializeVersionedRootSignature _D3D12SerializeVersionedRootSignature = nullptr;
    inline static PFN_D3D12CreateVersionedRootSignatureDeserializer _D3D12CreateVersionedRootSignatureDeserializer = nullptr;
    inline static PFN_D3D12GetDebugInterface _D3D12GetDebugInterface = nullptr;
    inline static PFN_D3D12EnableExperimentalFeatures _D3D12EnableExperimentalFeatures = nullptr;
    inline static PFN_D3D12GetInterface _D3D12GetInterface = nullptr;

    inline static FARPROC Hook(FARPROC* address, PVOID method)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)address, method);
        DetourTransactionCommit();

        return (*address);
    }
};
