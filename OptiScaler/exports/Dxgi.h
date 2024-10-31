#pragma once

#include <pch.h>

#include <dxgi1_6.h>

typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY)(REFIID riid, _COM_Outptr_ void** ppFactory);
typedef HRESULT(WINAPI* PFN_CREATE_DXGI_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

inline struct dxgi_dll
{
    HMODULE dll = nullptr;

    PFN_CREATE_DXGI_FACTORY CreateDxgiFactory;
    PFN_CREATE_DXGI_FACTORY CreateDxgiFactory1;
    PFN_CREATE_DXGI_FACTORY_2 CreateDxgiFactory2;

    FARPROC DeclareAdapterRemovalSupport;
    FARPROC GetDebugInterface;
    FARPROC ApplyCompatResolutionQuirking = nullptr;
    FARPROC CompatString = nullptr;
    FARPROC CompatValue = nullptr;
    FARPROC D3D10CreateDevice = nullptr;
    FARPROC D3D10CreateLayeredDevice = nullptr;
    FARPROC D3D10GetLayeredDeviceSize = nullptr;
    FARPROC D3D10RegisterLayers = nullptr;
    FARPROC D3D10ETWRundown = nullptr;
    FARPROC DumpJournal = nullptr;
    FARPROC ReportAdapterConfiguration = nullptr;
    FARPROC PIXBeginCapture = nullptr;
    FARPROC PIXEndCapture = nullptr;
    FARPROC PIXGetCaptureState = nullptr;
    FARPROC SetAppCompatStringPointer = nullptr;
    FARPROC UpdateHMDEmulationStatus = nullptr;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(module, "CreateDXGIFactory");
        CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(module, "CreateDXGIFactory1");
        CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)GetProcAddress(module, "CreateDXGIFactory2");

        DeclareAdapterRemovalSupport = GetProcAddress(module, "DXGIDeclareAdapterRemovalSupport");
        GetDebugInterface = GetProcAddress(module, "DXGIGetDebugInterface1");
        ApplyCompatResolutionQuirking = GetProcAddress(module, "ApplyCompatResolutionQuirking");
        CompatString = GetProcAddress(module, "CompatString");
        CompatValue = GetProcAddress(module, "CompatValue");
        D3D10CreateDevice = GetProcAddress(module, "DXGID3D10CreateDevice");
        D3D10CreateLayeredDevice = GetProcAddress(module, "DXGID3D10CreateLayeredDevice");
        D3D10GetLayeredDeviceSize = GetProcAddress(module, "DXGID3D10GetLayeredDeviceSize");
        D3D10RegisterLayers = GetProcAddress(module, "DXGID3D10RegisterLayers");
        D3D10ETWRundown = GetProcAddress(module, "DXGID3D10ETWRundown");
        DumpJournal = GetProcAddress(module, "DXGIDumpJournal");
        ReportAdapterConfiguration = GetProcAddress(module, "DXGIReportAdapterConfiguration");
        PIXBeginCapture = GetProcAddress(module, "PIXBeginCapture");
        PIXEndCapture = GetProcAddress(module, "PIXEndCapture");
        PIXGetCaptureState = GetProcAddress(module, "PIXGetCaptureState");
        SetAppCompatStringPointer = GetProcAddress(module, "SetAppCompatStringPointer");
        UpdateHMDEmulationStatus = GetProcAddress(module, "UpdateHMDEmulationStatus");
    }
} dxgi;

HRESULT _CreateDXGIFactory(REFIID riid, _COM_Outptr_ void** ppFactory);
HRESULT _CreateDXGIFactory1(REFIID riid, _COM_Outptr_ void** ppFactory);
HRESULT _CreateDXGIFactory2(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

void _DXGIDeclareAdapterRemovalSupport();
void _DXGIGetDebugInterface1();
void _ApplyCompatResolutionQuirking();
void _CompatString(); 
void _CompatValue();
void _DXGID3D10CreateDevice();
void _DXGID3D10CreateLayeredDevice();
void _DXGID3D10GetLayeredDeviceSize();
void _DXGID3D10RegisterLayers();
void _DXGID3D10ETWRundown();
void _DXGIDumpJournal();
void _DXGIReportAdapterConfiguration();
void _PIXBeginCapture();
void _PIXEndCapture();
void _PIXGetCaptureState();
void _SetAppCompatStringPointer();
void _UpdateHMDEmulationStatus();
