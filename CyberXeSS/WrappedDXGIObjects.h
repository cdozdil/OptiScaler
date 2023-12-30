#include "pch.h"
#include "dxgi1_6.h"

//#define BLOCK_IDXGIAdapterInternal2

class RefCountDXGIObject : public IDXGIObject
{
	IDXGIObject* m_pReal;
	unsigned int m_iRefcount;

public:
	RefCountDXGIObject(IDXGIObject* real) : m_pReal(real), m_iRefcount(1) {}
	virtual ~RefCountDXGIObject() {}
	static bool HandleWrap(const char* ifaceName, REFIID riid, void** ppvObject);
	static HRESULT WrapQueryInterface(IUnknown* real, const char* ifaceName, REFIID riid, void** ppvObject);

	//////////////////////////////
	// implement IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface( /* [in] */ REFIID riid, /* [annotation][iid_is][out] */ __RPC__deref_out void** ppvObject)
	{
		LOG("RefCountDXGIObject.QueryInterface");
		auto result = QueryInterface("IUnknown", riid, ppvObject);
		LOG("RefCountDXGIObject.QueryInterface result: " + int_to_hex(result));

		return result;
	}

	// optional overload that's useful for passing down the name of the current interface to put in
	// any 'unknown interface' query logs.
	HRESULT STDMETHODCALLTYPE QueryInterface(const char* ifaceName, REFIID riid, void** ppvObject)
	{
		if (riid == __uuidof(IUnknown))
		{
			AddRef();
			*ppvObject = (IUnknown*)(IDXGIObject*)this;
			return S_OK;
		}
		else if (riid == __uuidof(IDXGIObject))
		{
			AddRef();
			*ppvObject = (IDXGIObject*)this;
			return S_OK;
		}

		return WrapQueryInterface(m_pReal, ifaceName, riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		LOG("RefCountDXGIObject.AddRef");
		InterlockedIncrement(&m_iRefcount);
		LOG("RefCountDXGIObject.AddRef result: " + int_to_hex(m_iRefcount));
		return m_iRefcount;
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		LOG("RefCountDXGIObject.Release");

		unsigned int ret = InterlockedDecrement(&m_iRefcount);

		LOG("RefCountDXGIObject.Release result: " + int_to_hex(m_iRefcount));

		if (ret == 0)
		{
			LOG("RefCountDXGIObject.Release deleting object");
			delete this;
		}

		return ret;
	}

	//////////////////////////////
	// implement IDXGIObject

	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
		/* [in] */ REFGUID Name,
		/* [in] */ UINT DataSize,
		/* [in] */ const void* pData)
	{
		LOG("RefCountDXGIObject.SetPrivateData");
		auto result = m_pReal->SetPrivateData(Name, DataSize, pData);
		LOG("RefCountDXGIObject.SetPrivateData result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
		/* [in] */ REFGUID Name,
		/* [in] */ const IUnknown* pUnknown)
	{
		LOG("RefCountDXGIObject.SetPrivateDataInterface");
		auto result = m_pReal->SetPrivateDataInterface(Name, pUnknown);
		LOG("RefCountDXGIObject.SetPrivateDataInterface result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
		/* [in] */ REFGUID Name,
		/* [out][in] */ UINT* pDataSize,
		/* [out] */ void* pData)
	{
		LOG("RefCountDXGIObject.GetPrivateData");
		auto result = m_pReal->GetPrivateData(Name, pDataSize, pData);
		LOG("RefCountDXGIObject.GetPrivateData result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE GetParent(
		/* [in] */ REFIID riid,
		/* [retval][out] */ void** ppParent);
};

#define IMPLEMENT_IDXGIOBJECT_WITH_REFCOUNTDXGIOBJECT_CUSTOMQUERY                          \
  ULONG STDMETHODCALLTYPE AddRef()                                                         \
  {                                                                                        \
	return RefCountDXGIObject::AddRef();                                                   \
  }                                                                                        \
  ULONG STDMETHODCALLTYPE Release()                                                        \
  {                                                                                        \
	return RefCountDXGIObject::Release();                                                  \
  }                                                                                        \
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFIID Name, UINT DataSize, const void *pData)  \
  {                                                                                        \
	return RefCountDXGIObject::SetPrivateData(Name, DataSize, pData);                      \
  }                                                                                        \
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFIID Name, const IUnknown *pUnknown) \
  {                                                                                        \
	return RefCountDXGIObject::SetPrivateDataInterface(Name, pUnknown);                    \
  }                                                                                        \
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFIID Name, UINT *pDataSize, void *pData)      \
  {                                                                                        \
	return RefCountDXGIObject::GetPrivateData(Name, pDataSize, pData);                     \
  }                                                                                        \
  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppvObject)                       \
  {                                                                                        \
	return RefCountDXGIObject::GetParent(riid, ppvObject);                                 \
  }


MIDL_INTERFACE("cfdf09b3-a084-4453-a755-7d4e5389b845")
IDXGIProxyAdapter : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Spoofing(bool enable);
	virtual HRESULT STDMETHODCALLTYPE Wrapping(bool enable);
};

static bool b_spoofEnabled = true;
static bool b_wrappingEnabled = true;

class WrappedIDXGIAdapter4 : public IDXGIAdapter4, public RefCountDXGIObject, public IDXGIProxyAdapter
{
	IDXGIAdapter* m_pReal;
	IDXGIAdapter1* m_pReal1;
	IDXGIAdapter2* m_pReal2;
	IDXGIAdapter3* m_pReal3;
	IDXGIAdapter4* m_pReal4;

public:
	WrappedIDXGIAdapter4(IDXGIAdapter* real);
	virtual ~WrappedIDXGIAdapter4();

	IMPLEMENT_IDXGIOBJECT_WITH_REFCOUNTDXGIOBJECT_CUSTOMQUERY;

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	//////////////////////////////
	// implement IDXGIAdapter

	virtual HRESULT STDMETHODCALLTYPE EnumOutputs( /* [in] */ UINT Output, /* [annotation][out][in] */ __out IDXGIOutput** ppOutput)
	{
		LOG("WrappedIDXGIAdapter4.EnumOutputs");
		HRESULT ret = m_pReal->EnumOutputs(Output, ppOutput);
		LOG("WrappedIDXGIAdapter4.EnumOutputs result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDesc( /* [annotation][out] */ __out DXGI_ADAPTER_DESC* pDesc)
	{
		LOG("WrappedIDXGIAdapter4.GetDesc");

		HRESULT hr;
		hr = m_pReal->GetDesc(pDesc);

		if (hr == S_OK && b_spoofEnabled && pDesc != nullptr && (pDesc->VendorId == 0x8086 || pDesc->VendorId == 0x1002))
		{
			LOG("WrappedIDXGIAdapter4.GetDesc Spoofing card info");
			pDesc->VendorId = 0x10de;
			pDesc->DeviceId = 0x24c9;
			pDesc->SubSysId = 0x88ac1043;
			pDesc->Revision = 0x00a1;

			std::wstring name(L"NVIDIA GeForce RTX 3060 Ti");
			const wchar_t* szName = name.c_str();
			std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
			std::memcpy(pDesc->Description, szName, 54);

			LUID luid = LUID{ 0, 56090 };
			std::memcpy(&pDesc->AdapterLuid, &luid, 8);
		}

		//b_spoofEnabled = true;

		LOG("WrappedIDXGIAdapter4.GetDesc result: " + int_to_hex(hr));
		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(
		/* [annotation][in] */
		__in REFGUID InterfaceName,
		/* [annotation][out] */
		__out LARGE_INTEGER* pUMDVersion)
	{
		LOG("WrappedIDXGIAdapter4.CheckInterfaceSupport");
		auto result = m_pReal->CheckInterfaceSupport(InterfaceName, pUMDVersion);
		LOG("WrappedIDXGIAdapter4.CheckInterfaceSupport result: " + int_to_hex(result));
		return result;
	}

	//////////////////////////////
	// implement IDXGIAdapter1

	virtual HRESULT STDMETHODCALLTYPE GetDesc1(
		/* [out] */ DXGI_ADAPTER_DESC1* pDesc)
	{
		LOG("WrappedIDXGIAdapter4.GetDesc1");

		if (!m_pReal1)
		{
			LOG("WrappedIDXGIAdapter4.GetDesc1 no adapter!");
			return E_NOINTERFACE;
		}


		HRESULT hr;
		hr = m_pReal1->GetDesc1(pDesc);

		if (hr == S_OK && b_spoofEnabled && pDesc != nullptr && (pDesc->VendorId == 0x8086 || pDesc->VendorId == 0x1002))
		{
			LOG("WrappedIDXGIAdapter4.GetDesc1 Spoofing card info");

			pDesc->VendorId = 0x10de;
			pDesc->DeviceId = 0x24c9;
			pDesc->SubSysId = 0x88ac1043;
			pDesc->Revision = 0x00a1;

			std::wstring name(L"NVIDIA GeForce RTX 3060 Ti");
			const wchar_t* szName = name.c_str();
			std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
			std::memcpy(pDesc->Description, szName, 54);

			LUID luid = LUID{ 0, 56090 };
			std::memcpy(&pDesc->AdapterLuid, &luid, 8);
		}

		//b_spoofEnabled = true;

		LOG("WrappedIDXGIAdapter4.GetDesc1 result: " + int_to_hex(hr));
		return hr;
	}

	//////////////////////////////
	// implement IDXGIAdapter2

	virtual HRESULT STDMETHODCALLTYPE GetDesc2(
		/* [annotation][out] */
		_Out_ DXGI_ADAPTER_DESC2* pDesc)
	{
		LOG("WrappedIDXGIAdapter4.GetDesc2");

		if (!m_pReal2)
		{
			LOG("WrappedIDXGIAdapter4.GetDesc2 no adapter!");
			return E_NOINTERFACE;
		}

		HRESULT hr;
		hr = m_pReal2->GetDesc2(pDesc);

		if (hr == S_OK && b_spoofEnabled && pDesc != nullptr && (pDesc->VendorId == 0x8086 || pDesc->VendorId == 0x1002))
		{
			LOG("WrappedIDXGIAdapter4.GetDesc2 Spoofing card info");

			pDesc->VendorId = 0x10de;
			pDesc->DeviceId = 0x24c9;
			pDesc->SubSysId = 0x88ac1043;
			pDesc->Revision = 0x00a1;

			std::wstring name(L"NVIDIA GeForce RTX 3060 Ti");
			const wchar_t* szName = name.c_str();
			std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
			std::memcpy(pDesc->Description, szName, 54);

			LUID luid = LUID{ 0, 56090 };
			std::memcpy(&pDesc->AdapterLuid, &luid, 8);
		}

		//b_spoofEnabled = true;

		LOG("WrappedIDXGIAdapter4.GetDesc2 result: " + int_to_hex(hr));

		return hr;
	}

	//////////////////////////////
	// implement IDXGIAdapter3

	virtual HRESULT STDMETHODCALLTYPE RegisterHardwareContentProtectionTeardownStatusEvent(
		/* [annotation][in] */
		_In_ HANDLE hEvent,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIAdapter4.RegisterHardwareContentProtectionTeardownStatusEvent");
		auto result = m_pReal3->RegisterHardwareContentProtectionTeardownStatusEvent(hEvent, pdwCookie);
		LOG("WrappedIDXGIAdapter4.RegisterHardwareContentProtectionTeardownStatusEvent result: " + int_to_hex(result));
		return result;
	}

	virtual void STDMETHODCALLTYPE UnregisterHardwareContentProtectionTeardownStatus(
		/* [annotation][in] */
		_In_ DWORD dwCookie)
	{
		LOG("WrappedIDXGIAdapter4.UnregisterHardwareContentProtectionTeardownStatus");
		m_pReal3->UnregisterHardwareContentProtectionTeardownStatus(dwCookie);
		LOG("WrappedIDXGIAdapter4.UnregisterHardwareContentProtectionTeardownStatus done");
	}

	virtual HRESULT STDMETHODCALLTYPE QueryVideoMemoryInfo(
		/* [annotation][in] */
		_In_ UINT NodeIndex,
		/* [annotation][in] */
		_In_ DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
		/* [annotation][out] */
		_Out_ DXGI_QUERY_VIDEO_MEMORY_INFO* pVideoMemoryInfo)
	{
		LOG("WrappedIDXGIAdapter4.QueryVideoMemoryInfo");

		auto result = m_pReal3->QueryVideoMemoryInfo(NodeIndex, MemorySegmentGroup, pVideoMemoryInfo);
		LOG("WrappedIDXGIAdapter4.QueryVideoMemoryInfo result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE SetVideoMemoryReservation(
		/* [annotation][in] */
		_In_ UINT NodeIndex,
		/* [annotation][in] */
		_In_ DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup,
		/* [annotation][in] */
		_In_ UINT64 Reservation)
	{
		LOG("WrappedIDXGIAdapter4.SetVideoMemoryReservation");

		auto result = m_pReal3->SetVideoMemoryReservation(NodeIndex, MemorySegmentGroup, Reservation);
		LOG("WrappedIDXGIAdapter4.SetVideoMemoryReservation result: " + int_to_hex(result));
		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE RegisterVideoMemoryBudgetChangeNotificationEvent(
		/* [annotation][in] */
		_In_ HANDLE hEvent,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIAdapter4.RegisterVideoMemoryBudgetChangeNotificationEvent");

		auto result = m_pReal3->RegisterVideoMemoryBudgetChangeNotificationEvent(hEvent, pdwCookie);
		LOG("WrappedIDXGIAdapter4.RegisterVideoMemoryBudgetChangeNotificationEvent result: " + int_to_hex(result));
		return result;
	}

	virtual void STDMETHODCALLTYPE UnregisterVideoMemoryBudgetChangeNotification(
		/* [annotation][in] */
		_In_ DWORD dwCookie)
	{
		LOG("WrappedIDXGIAdapter4.UnregisterVideoMemoryBudgetChangeNotification");

		m_pReal3->UnregisterVideoMemoryBudgetChangeNotification(dwCookie);
		LOG("WrappedIDXGIAdapter4.UnregisterVideoMemoryBudgetChangeNotification done");
	}

	//////////////////////////////
	// implement IDXGIAdapter4

	virtual HRESULT STDMETHODCALLTYPE GetDesc3(
		/* [annotation][out] */
		_Out_ DXGI_ADAPTER_DESC3* pDesc)
	{
		LOG("WrappedIDXGIAdapter4.GetDesc3");

		if (!m_pReal4)
		{
			LOG("WrappedIDXGIAdapter4.GetDesc3 no adapter!");
			return E_NOINTERFACE;
		}

		HRESULT hr;
		hr = m_pReal4->GetDesc3(pDesc);

		if (hr == S_OK && b_spoofEnabled && pDesc != nullptr && (pDesc->VendorId == 0x8086 || pDesc->VendorId == 0x1002))
		{
			LOG("WrappedIDXGIAdapter4.GetDesc3 Spoofing card info");

			pDesc->VendorId = 0x10de;
			pDesc->DeviceId = 0x24c9;
			pDesc->SubSysId = 0x88ac1043;
			pDesc->Revision = 0x00a1;

			std::wstring name(L"NVIDIA GeForce RTX 3060 Ti");
			const wchar_t* szName = name.c_str();
			std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
			std::memcpy(pDesc->Description, szName, 54);

			LUID luid = LUID{ 0, 56090 };
			std::memcpy(&pDesc->AdapterLuid, &luid, 8);
		}

		//b_spoofEnabled = true;

		LOG("WrappedIDXGIAdapter4.GetDesc3 result: " + int_to_hex(hr));

		return hr;
	}

	virtual HRESULT STDMETHODCALLTYPE Spoofing(bool enable)
	{
		LOG("WrappedIDXGIAdapter4.Spoofing : " + std::to_string(enable));
		b_spoofEnabled = enable;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Wrapping(bool enable)
	{
		LOG("WrappedIDXGIAdapter4.Wrapping : " + std::to_string(enable));
		b_wrappingEnabled = enable;
		return S_OK;
	}
};

class WrappedIDXGIFactory : public IDXGIFactory7, public RefCountDXGIObject
{
	IDXGIFactory* m_pReal;
	IDXGIFactory1* m_pReal1;
	IDXGIFactory2* m_pReal2;
	IDXGIFactory3* m_pReal3;
	IDXGIFactory4* m_pReal4;
	IDXGIFactory5* m_pReal5;
	IDXGIFactory6* m_pReal6;
	IDXGIFactory7* m_pReal7;

public:
	WrappedIDXGIFactory(IDXGIFactory* real);
	virtual ~WrappedIDXGIFactory();

	IMPLEMENT_IDXGIOBJECT_WITH_REFCOUNTDXGIOBJECT_CUSTOMQUERY;
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	//////////////////////////////
	// implement IDXGIFactory

	virtual HRESULT STDMETHODCALLTYPE EnumAdapters(
		/* [in] */ UINT Adapter,
		/* [annotation][out] */
		__out IDXGIAdapter** ppAdapter)
	{
		LOG("WrappedIDXGIFactory.EnumAdapters " + std::to_string(Adapter));

		HRESULT ret = m_pReal->EnumAdapters(Adapter, ppAdapter);
		
		if (ret == S_OK && b_wrappingEnabled)
			*ppAdapter = (IDXGIAdapter*)(new WrappedIDXGIAdapter4(*ppAdapter));

		LOG("WrappedIDXGIFactory.EnumAdapters result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
	{
		LOG("WrappedIDXGIFactory.MakeWindowAssociation");

		auto ret = m_pReal->MakeWindowAssociation(WindowHandle, Flags);
		LOG("WrappedIDXGIFactory.MakeWindowAssociation result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation(
		/* [annotation][out] */
		__out HWND* pWindowHandle)
	{
		LOG("WrappedIDXGIFactory.GetWindowAssociation");

		auto ret = m_pReal->GetWindowAssociation(pWindowHandle);
		LOG("WrappedIDXGIFactory.GetWindowAssociation result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE CreateSwapChain(
		/* [annotation][in] */
		__in IUnknown* pDevice,
		/* [annotation][in] */
		__in DXGI_SWAP_CHAIN_DESC* pDesc,
		/* [annotation][out] */
		__out IDXGISwapChain** ppSwapChain);

	virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(
		/* [in] */ HMODULE Module,
		/* [annotation][out] */
		__out IDXGIAdapter** ppAdapter)
	{
		LOG("WrappedIDXGIFactory.CreateSoftwareAdapter");

		HRESULT ret = m_pReal->CreateSoftwareAdapter(Module, ppAdapter);
		
		if (ret == S_OK && b_wrappingEnabled)
			*ppAdapter = (IDXGIAdapter*)(new WrappedIDXGIAdapter4(*ppAdapter));

		LOG("WrappedIDXGIFactory.CreateSoftwareAdapter result: " + int_to_hex(ret));
		return ret;
	}

	//////////////////////////////
	// implement IDXGIFactory1

	virtual HRESULT STDMETHODCALLTYPE EnumAdapters1(
		/* [in] */ UINT Adapter,
		/* [annotation][out] */
		__out IDXGIAdapter1** ppAdapter)
	{
		LOG("WrappedIDXGIFactory.EnumAdapters1 " + std::to_string(Adapter));

		IDXGIFactory1* factory = m_pReal1;
		if (m_pReal1 == NULL)
		{
			// see comment in RefCountDXGIObject::HandleWrap for IDXGIFactory
			//RDCWARN("Calling EnumAdapters1 with no IDXGIFactory1 - assuming weird internal call");
			factory = (IDXGIFactory1*)m_pReal;
		}

		HRESULT ret = factory->EnumAdapters1(Adapter, ppAdapter);

		if (ret == S_OK && b_wrappingEnabled)
			*ppAdapter = (IDXGIAdapter1*)(new WrappedIDXGIAdapter4(*ppAdapter));

		LOG("WrappedIDXGIFactory.EnumAdapters1 result: " + int_to_hex(ret));
		return ret;
	}

	virtual BOOL STDMETHODCALLTYPE IsCurrent(void)
	{
		LOG("WrappedIDXGIFactory.IsCurrent");

		auto ret = m_pReal1->IsCurrent();
		LOG("WrappedIDXGIFactory.IsCurrent result: " + std::to_string(ret));
		return ret;
	}
	//////////////////////////////
	// implement IDXGIFactory2

	virtual BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled(void)
	{
		LOG("WrappedIDXGIFactory.IsWindowedStereoEnabled");

		auto ret = m_pReal2->IsWindowedStereoEnabled();
		LOG("WrappedIDXGIFactory.IsCurrent result: " + std::to_string(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(
		/* [annotation][in] */
		_In_ IUnknown* pDevice,
		/* [annotation][in] */
		_In_ HWND hWnd,
		/* [annotation][in] */
		_In_ const DXGI_SWAP_CHAIN_DESC1* pDesc,
		/* [annotation][in] */
		_In_opt_ const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc,
		/* [annotation][in] */
		_In_opt_ IDXGIOutput* pRestrictToOutput,
		/* [annotation][out] */
		_Out_ IDXGISwapChain1** ppSwapChain);

	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(
		/* [annotation][in] */
		_In_ IUnknown* pDevice,
		/* [annotation][in] */
		_In_ IUnknown* pWindow,
		/* [annotation][in] */
		_In_ const DXGI_SWAP_CHAIN_DESC1* pDesc,
		/* [annotation][in] */
		_In_opt_ IDXGIOutput* pRestrictToOutput,
		/* [annotation][out] */
		_Out_ IDXGISwapChain1** ppSwapChain);

	virtual HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(
		/* [annotation] */
		_In_ HANDLE hResource,
		/* [annotation] */
		_Out_ LUID* pLuid)
	{
		LOG("WrappedIDXGIFactory.GetSharedResourceAdapterLuid");

		auto ret = m_pReal2->GetSharedResourceAdapterLuid(hResource, pLuid);
		LOG("WrappedIDXGIFactory.GetSharedResourceAdapterLuid result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(
		/* [annotation][in] */
		_In_ HWND WindowHandle,
		/* [annotation][in] */
		_In_ UINT wMsg,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIFactory.RegisterStereoStatusWindow");

		auto ret = m_pReal2->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
		LOG("WrappedIDXGIFactory.RegisterStereoStatusWindow result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(
		/* [annotation][in] */
		_In_ HANDLE hEvent,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIFactory.RegisterStereoStatusEvent");

		auto ret = m_pReal2->RegisterStereoStatusEvent(hEvent, pdwCookie);
		LOG("WrappedIDXGIFactory.RegisterStereoStatusEvent result: " + int_to_hex(ret));
		return ret;
	}

	virtual void STDMETHODCALLTYPE UnregisterStereoStatus(
		/* [annotation][in] */
		_In_ DWORD dwCookie)
	{
		LOG("WrappedIDXGIFactory.UnregisterStereoStatus");

		m_pReal2->UnregisterStereoStatus(dwCookie);
		LOG("WrappedIDXGIFactory.UnregisterStereoStatus done");
	}

	virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(
		/* [annotation][in] */
		_In_ HWND WindowHandle,
		/* [annotation][in] */
		_In_ UINT wMsg,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIFactory.RegisterOcclusionStatusWindow");

		auto ret = m_pReal2->RegisterOcclusionStatusWindow(WindowHandle, wMsg, pdwCookie);
		LOG("WrappedIDXGIFactory.RegisterOcclusionStatusWindow result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(
		/* [annotation][in] */
		_In_ HANDLE hEvent,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIFactory.RegisterOcclusionStatusEvent");

		auto ret = m_pReal2->RegisterOcclusionStatusEvent(hEvent, pdwCookie);
		LOG("WrappedIDXGIFactory.RegisterOcclusionStatusEvent result: " + int_to_hex(ret));
		return ret;
	}

	virtual void STDMETHODCALLTYPE UnregisterOcclusionStatus(
		/* [annotation][in] */
		_In_ DWORD dwCookie)
	{
		LOG("WrappedIDXGIFactory.UnregisterOcclusionStatus");

		m_pReal2->UnregisterOcclusionStatus(dwCookie);
		LOG("WrappedIDXGIFactory.UnregisterOcclusionStatus done");
	}

	virtual HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(
		/* [annotation][in] */
		_In_ IUnknown* pDevice,
		/* [annotation][in] */
		_In_ const DXGI_SWAP_CHAIN_DESC1* pDesc,
		/* [annotation][in] */
		_In_opt_ IDXGIOutput* pRestrictToOutput,
		/* [annotation][out] */
		_Outptr_ IDXGISwapChain1** ppSwapChain);

	//////////////////////////////
	// implement IDXGIFactory3

	virtual UINT STDMETHODCALLTYPE GetCreationFlags(void)
	{
		LOG("WrappedIDXGIFactory.GetCreationFlags");

		auto ret = m_pReal3->GetCreationFlags();
		LOG("WrappedIDXGIFactory.GetCreationFlags result: " + int_to_hex(ret));
		return ret;
	}
	//////////////////////////////
	// implement IDXGIFactory4

	bool WrapAdapter(REFIID riid, void** ppvAdapter)
	{
		LOG("WrappedIDXGIFactory.WrapAdapter");

		if (ppvAdapter == NULL || *ppvAdapter == NULL)
		{
			LOG("WrappedIDXGIFactory.WrapAdapter ppvAdapter is NULL!");
			return false;
		}

		if (riid == __uuidof(IDXGIAdapter4) && b_wrappingEnabled)
		{
			IDXGIAdapter4* adapter = (IDXGIAdapter4*)*ppvAdapter;
			*ppvAdapter = (IDXGIAdapter4*)(new WrappedIDXGIAdapter4(adapter));
			return true;
		}
		else if (riid == __uuidof(IDXGIAdapter3) && b_wrappingEnabled)
		{
			IDXGIAdapter3* adapter = (IDXGIAdapter3*)*ppvAdapter;
			*ppvAdapter = (IDXGIAdapter3*)(new WrappedIDXGIAdapter4(adapter));
			return true;
		}
		else if (riid == __uuidof(IDXGIAdapter2) && b_wrappingEnabled)
		{
			IDXGIAdapter2* adapter = (IDXGIAdapter2*)*ppvAdapter;
			*ppvAdapter = (IDXGIAdapter2*)(new WrappedIDXGIAdapter4(adapter));
			return true;
		}
		else if (riid == __uuidof(IDXGIAdapter1) && b_wrappingEnabled)
		{
			IDXGIAdapter1* adapter = (IDXGIAdapter1*)*ppvAdapter;
			*ppvAdapter = (IDXGIAdapter1*)(new WrappedIDXGIAdapter4(adapter));
			return true;
		}
		else if (riid == __uuidof(IDXGIAdapter) && b_wrappingEnabled)
		{
			IDXGIAdapter* adapter = (IDXGIAdapter*)*ppvAdapter;
			*ppvAdapter = (IDXGIAdapter*)(new WrappedIDXGIAdapter4(adapter));
			return true;
		}
		else
		{
			return RefCountDXGIObject::HandleWrap("IDXGIAdapter", riid, ppvAdapter);
		}

		return false;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumAdapterByLuid( /* [annotation] */ _In_ LUID AdapterLuid, /* [annotation] */ _In_ REFIID riid, /* [annotation] */ _COM_Outptr_ void** ppvAdapter)

	{
#ifdef BLOCK_IDXGIAdapterInternal2
		// unknown/undocumented internal interface
		// {7abb6563-02bc-47c4-8ef9-acc4795edbcf}
		static const GUID IDXGIAdapterInternal2_uuid = {
			0x7abb6563, 0x02bc, 0x47c4, {0x8e, 0xf9, 0xac, 0xc4, 0x79, 0x5e, 0xdb, 0xcf} };

		if (riid == IDXGIAdapterInternal2_uuid)
		{
			LOG("WrappedIDXGIFactory.EnumAdapterByLuid IDXGIAdapterInternal2_uuid result: " + int_to_hex(DXGI_ERROR_NOT_FOUND));
			return DXGI_ERROR_NOT_FOUND;
		}
#endif

		LOG("WrappedIDXGIFactory.EnumAdapterByLuid LUID: " + int_to_hex(AdapterLuid.HighPart) + "-" + int_to_hex(AdapterLuid.LowPart) + " riid: " + ToString(riid));

		HRESULT ret = m_pReal4->EnumAdapterByLuid(AdapterLuid, riid, ppvAdapter);

		if (ret == S_OK && b_wrappingEnabled)
		{
			auto wrapResult = this->WrapAdapter(riid, ppvAdapter);

			if (!wrapResult)
			{
				LOG("WrappedIDXGIFactory.EnumAdapterByLuid wrapResult result: " + int_to_hex(DXGI_ERROR_NOT_FOUND));
				return DXGI_ERROR_NOT_FOUND;
			}
		}
		else
		{
			LOG("WrappedIDXGIFactory.EnumAdapterByLuid can't get adapter by LUID, user first adapter");
			IDXGIAdapter* wrappedAdapter;
			ret = this->EnumAdapters(0, &wrappedAdapter);

			if (ret == S_OK)
				*ppvAdapter = wrappedAdapter;
		}

		LOG("WrappedIDXGIFactory.EnumAdapterByLuid result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumWarpAdapter(
		/* [annotation] */
		_In_ REFIID riid,
		/* [annotation] */
		_COM_Outptr_ void** ppvAdapter)
	{
		LOG("WrappedIDXGIFactory.EnumWarpAdapter");

		HRESULT ret = m_pReal4->EnumWarpAdapter(riid, ppvAdapter);
		
		if (ret == S_OK && b_wrappingEnabled)
			WrapAdapter(riid, ppvAdapter);

		LOG("WrappedIDXGIFactory.EnumWarpAdapter result: " + int_to_hex(ret));
		return ret;
	}

	//////////////////////////////
	// implement IDXGIFactory5

	virtual HRESULT STDMETHODCALLTYPE
		CheckFeatureSupport(DXGI_FEATURE Feature,
			/* [annotation] */
			_Inout_updates_bytes_(FeatureSupportDataSize) void* pFeatureSupportData,
			UINT FeatureSupportDataSize)
	{
		LOG("WrappedIDXGIFactory.CheckFeatureSupport");

		auto ret = m_pReal5->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
		LOG("WrappedIDXGIFactory.CheckFeatureSupport result: " + int_to_hex(ret));
		return ret;
	}

	//////////////////////////////
	// implement IDXGIFactory6

	virtual HRESULT STDMETHODCALLTYPE EnumAdapterByGpuPreference(
		/* [annotation] */
		_In_ UINT Adapter,
		/* [annotation] */
		_In_ DXGI_GPU_PREFERENCE GpuPreference,
		/* [annotation] */
		_In_ REFIID riid,
		/* [annotation] */
		_COM_Outptr_ void** ppvAdapter)
	{
		LOG("WrappedIDXGIFactory.EnumAdapterByGpuPreference " + std::to_string(Adapter) + ", GpuPreference: " + int_to_hex(GpuPreference));

		HRESULT ret = m_pReal6->EnumAdapterByGpuPreference(Adapter, GpuPreference, riid, ppvAdapter);

		if (ret == S_OK && b_wrappingEnabled)
			WrapAdapter(riid, ppvAdapter);

		LOG("WrappedIDXGIFactory.EnumAdapterByGpuPreference result: " + int_to_hex(ret));
		return ret;
	}

	//////////////////////////////
	// implement IDXGIFactory7

	virtual HRESULT STDMETHODCALLTYPE RegisterAdaptersChangedEvent(
		/* [annotation][in] */
		_In_ HANDLE hEvent,
		/* [annotation][out] */
		_Out_ DWORD* pdwCookie)
	{
		LOG("WrappedIDXGIFactory.RegisterAdaptersChangedEvent");

		auto ret = m_pReal7->RegisterAdaptersChangedEvent(hEvent, pdwCookie);
		LOG("WrappedIDXGIFactory.RegisterAdaptersChangedEvent result: " + int_to_hex(ret));
		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE UnregisterAdaptersChangedEvent(
		/* [annotation][in] */
		_In_ DWORD dwCookie)
	{
		LOG("WrappedIDXGIFactory.UnregisterAdaptersChangedEvent");

		auto ret = m_pReal7->UnregisterAdaptersChangedEvent(dwCookie);
		LOG("WrappedIDXGIFactory.UnregisterAdaptersChangedEvent result: " + int_to_hex(ret));
		return ret;
	}
};

