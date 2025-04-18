#pragma once

#include <pch.h>

#include <proxies/D3D12_Proxy.h>
#include <proxies/KernelBase_Proxy.h>

struct d3d12_dll
{
    HMODULE dll = nullptr;
    FARPROC GetBehaviorValue = nullptr;
    FARPROC SetAppCompatStringPointer = nullptr;
    FARPROC D3D12CoreCreateLayeredDevice = nullptr;
    FARPROC D3D12CoreGetLayeredDeviceSize = nullptr;
    FARPROC D3D12CoreRegisterLayers = nullptr;
    FARPROC D3D12DeviceRemovedExtendedData = nullptr;
    FARPROC D3D12PIXEventsReplaceBlock = nullptr;
    FARPROC D3D12PIXGetThreadInfo = nullptr;
    FARPROC D3D12PIXNotifyWakeFromFenceSignal = nullptr;
    FARPROC D3D12PIXReportCounter = nullptr;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        GetBehaviorValue = KernelBaseProxy::GetProcAddress_()(dll, "GetBehaviorValue");
        SetAppCompatStringPointer = KernelBaseProxy::GetProcAddress_()(dll, "SetAppCompatStringPointer");
        D3D12CoreCreateLayeredDevice = KernelBaseProxy::GetProcAddress_()(dll, "D3D12CoreCreateLayeredDevice");
        D3D12CoreGetLayeredDeviceSize = KernelBaseProxy::GetProcAddress_()(dll, "D3D12CoreGetLayeredDeviceSize");
        D3D12CoreRegisterLayers = KernelBaseProxy::GetProcAddress_()(dll, "D3D12CoreRegisterLayers");
        D3D12DeviceRemovedExtendedData = KernelBaseProxy::GetProcAddress_()(dll, "D3D12DeviceRemovedExtendedData");
        D3D12PIXEventsReplaceBlock = KernelBaseProxy::GetProcAddress_()(dll, "D3D12PIXEventsReplaceBlock");
        D3D12PIXGetThreadInfo = KernelBaseProxy::GetProcAddress_()(dll, "D3D12PIXGetThreadInfo");
        D3D12PIXNotifyWakeFromFenceSignal = KernelBaseProxy::GetProcAddress_()(dll, "D3D12PIXNotifyWakeFromFenceSignal");
        D3D12PIXReportCounter = KernelBaseProxy::GetProcAddress_()(dll, "D3D12PIXReportCounter");
    }
} d3d12;

HRESULT _D3D12CreateDeviceExport(IUnknown* adapter, D3D_FEATURE_LEVEL minLevel, REFIID riid, void** ppDevice)
{
    return D3d12Proxy::D3D12CreateDevice_Hooked()(adapter, minLevel, riid, ppDevice);
}

HRESULT _D3D12SerializeRootSignatureExport(D3d12Proxy::D3D12_ROOT_SIGNATURE_DESC_L* pRootSignature, D3D_ROOT_SIGNATURE_VERSION Version, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob)
{
    return D3d12Proxy::D3D12SerializeRootSignature_Hooked()(pRootSignature, Version, ppBlob, ppErrorBlob);
}

HRESULT _D3D12CreateRootSignatureDeserializerExport(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes, REFIID pRootSignatureDeserializerInterface, void** ppRootSignatureDeserializer)
{
    return D3d12Proxy::D3D12CreateRootSignatureDeserializer_Hooked()(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT _D3D12SerializeVersionedRootSignatureExport(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* pRootSignature, ID3DBlob** ppBlob, ID3DBlob** ppErrorBlob)
{
    return D3d12Proxy::D3D12SerializeVersionedRootSignature_Hooked()(pRootSignature, ppBlob, ppErrorBlob);
}

HRESULT _D3D12CreateVersionedRootSignatureDeserializerExport(LPCVOID pSrcData, SIZE_T SrcDataSizeInBytes, REFIID pRootSignatureDeserializerInterface, void** ppRootSignatureDeserializer)
{
    return D3d12Proxy::D3D12CreateVersionedRootSignatureDeserializer_Hooked()(pSrcData, SrcDataSizeInBytes, pRootSignatureDeserializerInterface, ppRootSignatureDeserializer);
}

HRESULT _D3D12GetDebugInterfaceExport(REFIID riid, void** ppDebug)
{
    return D3d12Proxy::D3D12GetDebugInterface_Hooked()(riid, ppDebug);
}

HRESULT _D3D12EnableExperimentalFeaturesExport(UINT NumFeatures, const IID* pIIDs, void* pConfigurationStructs, UINT* pConfigurationStructSizes)
{
    return D3d12Proxy::D3D12EnableExperimentalFeatures_Hooked()(NumFeatures, pIIDs, pConfigurationStructs, pConfigurationStructSizes);
}

HRESULT _D3D12GetInterfaceExport(REFCLSID clsid, REFIID riid, void** ppInterface)
{
    return D3d12Proxy::D3D12GetInterface_Hooked()(clsid, riid, ppInterface);
}

void _GetBehaviorValue() { d3d12.GetBehaviorValue(); }
void _D3D12CoreCreateLayeredDevice() { d3d12.D3D12CoreCreateLayeredDevice(); }
void _D3D12CoreGetLayeredDeviceSize() { d3d12.D3D12CoreGetLayeredDeviceSize(); }
void _D3D12CoreRegisterLayers() { d3d12.D3D12CoreRegisterLayers(); }
void _D3D12DeviceRemovedExtendedData() { d3d12.D3D12DeviceRemovedExtendedData(); }
void _D3D12PIXEventsReplaceBlock() { d3d12.D3D12PIXEventsReplaceBlock(); }
void _D3D12PIXGetThreadInfo() { d3d12.D3D12PIXGetThreadInfo(); }
void _D3D12PIXNotifyWakeFromFenceSignal() { d3d12.D3D12PIXNotifyWakeFromFenceSignal(); }
void _D3D12PIXReportCounter() { d3d12.D3D12PIXReportCounter(); }

// Disabled to prevent crashes
//void _SetAppCompatStringPointer() { d3d12.SetAppCompatStringPointer(); }


