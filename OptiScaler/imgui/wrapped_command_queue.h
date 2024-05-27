#pragma once

#include <d3d12.h>

class WrappedID3D12CommandQueue : public ID3D12CommandQueue
{
    ID3D12CommandQueue* m_pReal = nullptr;

    unsigned int m_iRefcount;

public:
    WrappedID3D12CommandQueue(ID3D12CommandQueue* real) : m_pReal(real), m_iRefcount(1)
    {
    }
    
    ~WrappedID3D12CommandQueue()
    {
        if (m_pReal != nullptr)
        {
            m_pReal->Release();
            m_pReal = nullptr;
        }
    }

    //////////////////////////////
    // implement IUnknown

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == __uuidof(ID3D12CommandQueue))
        {
            AddRef();
            *ppvObject = (ID3D12CommandQueue*)this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        InterlockedIncrement(&m_iRefcount);
        m_pReal->AddRef();
        return m_iRefcount;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        m_pReal->Release();

        unsigned int ret = InterlockedDecrement(&m_iRefcount);

        if (ret == 0)
        {
            //if (ClearTrig != nullptr)
            //    ClearTrig();

            delete this;
        }

        return ret;
    }

    //////////////////////////////
    // implement ID3D12Object

    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData)
    {
        return m_pReal->GetPrivateData(guid, pDataSize, pData);
    }

    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData)
    {
        return m_pReal->SetPrivateData(guid, DataSize, pData);
    }

    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData)
    {
        return m_pReal->SetPrivateDataInterface(guid, pData);
    }

    HRESULT STDMETHODCALLTYPE SetName(LPCWSTR Name)
    {
        return m_pReal->SetName(Name);
    }

    //////////////////////////////
    // implement ID3D12DeviceChild

    HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, _COM_Outptr_opt_ void** ppvDevice)
    {
        return m_pReal->GetDevice(riid, ppvDevice);
    }

    //////////////////////////////
    // implement ID3D12CommandQueue

    void STDMETHODCALLTYPE UpdateTileMappings(
        ID3D12Resource* pResource, UINT NumResourceRegions,
        const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
        const D3D12_TILE_REGION_SIZE* pResourceRegionSizes, ID3D12Heap* pHeap,
        UINT NumRanges, const D3D12_TILE_RANGE_FLAGS* pRangeFlags,
        const UINT* pHeapRangeStartOffsets, const UINT* pRangeTileCounts,
        D3D12_TILE_MAPPING_FLAGS Flags)
    {
        return m_pReal->UpdateTileMappings(pResource, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeap,
            NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
    }

    void STDMETHODCALLTYPE CopyTileMappings(
        ID3D12Resource* pDstResource,
        const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate,
        ID3D12Resource* pSrcResource,
        const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate,
        const D3D12_TILE_REGION_SIZE* pRegionSize,
        D3D12_TILE_MAPPING_FLAGS Flags)
    {
        return m_pReal->CopyTileMappings(pDstResource, pDstRegionStartCoordinate, pSrcResource, pSrcRegionStartCoordinate, pRegionSize, Flags);
    }

    void STDMETHODCALLTYPE ExecuteCommandLists(
        UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
    {
        return m_pReal->ExecuteCommandLists(NumCommandLists, ppCommandLists);
    }

    void STDMETHODCALLTYPE SetMarker(UINT Metadata,
        const void* pData, UINT Size)
    {
        return m_pReal->SetMarker(Metadata, pData, Size);
    }

    void STDMETHODCALLTYPE BeginEvent(UINT Metadata,
        const void* pData, UINT Size)
    {
        return m_pReal->BeginEvent(Metadata, pData, Size);
    }

    void STDMETHODCALLTYPE EndEvent()
    {
        return m_pReal->EndEvent();
    }

    HRESULT STDMETHODCALLTYPE Signal(ID3D12Fence* pFence,
        UINT64 Value)
    {
        return m_pReal->Signal(pFence, Value);
    }

    HRESULT STDMETHODCALLTYPE Wait(ID3D12Fence* pFence,
        UINT64 Value)
    {
        return m_pReal->Wait(pFence, Value);
    }

    HRESULT STDMETHODCALLTYPE GetTimestampFrequency(UINT64* pFrequency)
    {
        return m_pReal->GetTimestampFrequency(pFrequency);
    }

    HRESULT STDMETHODCALLTYPE GetClockCalibration(UINT64* pGpuTimestamp, UINT64* pCpuTimestamp)
    {
        return m_pReal->GetClockCalibration(pGpuTimestamp, pCpuTimestamp);
    }

    D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE GetDesc() { return m_pReal->GetDesc(); }
    
};
