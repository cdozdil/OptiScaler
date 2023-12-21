#include "pch.h"
#include "dxgi1_6.h"
#include "WrappedDXGIObjects.h"

bool RefCountDXGIObject::HandleWrap(const char* ifaceName, REFIID riid, void** ppvObject)
{
	LOG("RefCountDXGIObject.HandleWrap");

	if (ppvObject == NULL || *ppvObject == NULL)
	{
		std::string str(ifaceName);
		LOG("RefCountDXGIObject.HandleWrap called with NULL ppvObject querying " + str);
		return false;
	}

	// unknown GUID that we only want to print once to avoid log spam
	  // {79D2046C-22EF-451B-9E74-2245D9C760EA}
	static const GUID Unknown_uuid = { 0x79d2046c, 0x22ef, 0x451b, {0x9e, 0x74, 0x22, 0x45, 0xd9, 0xc7, 0x60, 0xea} };

	// ditto
	// {9B7E4C04-342C-4106-A19F-4F2704F689F0}
	static const GUID ID3D10Texture2D_uuid = { 0x9b7e4c04, 0x342c, 0x4106, {0xa1, 0x9f, 0x4f, 0x27, 0x04, 0xf6, 0x89, 0xf0} };

#ifdef BLOCK_IDXGIAdapterInternal2
	// unknown/undocumented internal interface
	// {7abb6563-02bc-47c4-8ef9-acc4795edbcf}
	static const GUID IDXGIAdapterInternal2_uuid = { 0x7abb6563, 0x02bc, 0x47c4, {0x8e, 0xf9, 0xac, 0xc4, 0x79, 0x5e, 0xdb, 0xcf} };
#endif 


	if (riid == __uuidof(IDXGIDevice) ||
		riid == __uuidof(IDXGIDevice1))
	{
		// should have been handled elsewhere, so we can properly create this device
		std::string str(ifaceName);
		LOG("Unexpected uuid in RefCountDXGIObject::HandleWrap querying : " + str);
		return false;
	}
	else if (riid == __uuidof(IDXGIAdapter))
	{
		if (b_wrappingEnabled)
		{
			IDXGIAdapter* real = (IDXGIAdapter*)(*ppvObject);
			*ppvObject = (IDXGIAdapter*)(new WrappedIDXGIAdapter4(real));
		}
	}
	else if (riid == __uuidof(IDXGIAdapter1))
	{
		if (b_wrappingEnabled)
		{
			IDXGIAdapter1* real = (IDXGIAdapter1*)(*ppvObject);
			*ppvObject = (IDXGIAdapter1*)(new WrappedIDXGIAdapter4(real));
		}
	}
	else if (riid == __uuidof(IDXGIAdapter2))
	{
		if (b_wrappingEnabled)
		{
			IDXGIAdapter2* real = (IDXGIAdapter2*)(*ppvObject);
			*ppvObject = (IDXGIAdapter2*)(new WrappedIDXGIAdapter4(real));
		}
	}
	else if (riid == __uuidof(IDXGIAdapter3))
	{
		if (b_wrappingEnabled)
		{
			IDXGIAdapter3* real = (IDXGIAdapter3*)(*ppvObject);
			*ppvObject = (IDXGIAdapter3*)(new WrappedIDXGIAdapter4(real));
		}
	}
	else if (riid == __uuidof(IDXGIAdapter4))
	{
		if (b_spoofEnabled)
		{
			IDXGIAdapter4* real = (IDXGIAdapter4*)(*ppvObject);
			*ppvObject = (IDXGIAdapter4*)(new WrappedIDXGIAdapter4(real));
		}
	}
	else if (riid == __uuidof(IDXGIFactory))
	{
		// yes I know PRECISELY how fucked up this is. Speak to microsoft - after KB2670838 the internal
		// D3D11 device creation function will pass in __uuidof(IDXGIFactory) then attempt to call
		// EnumDevices1 (which is in the IDXGIFactory1 vtable). Doing this *should* be safe as using a
		// IDXGIFactory1 like a IDXGIFactory should all just work by definition, but there's no way to
		// know now if someone trying to create a IDXGIFactory really means it or not.
		IDXGIFactory* real = (IDXGIFactory*)(*ppvObject);
		*ppvObject = (IDXGIFactory*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory1))
	{
		IDXGIFactory1* real = (IDXGIFactory1*)(*ppvObject);
		*ppvObject = (IDXGIFactory1*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory2))
	{
		IDXGIFactory2* real = (IDXGIFactory2*)(*ppvObject);
		*ppvObject = (IDXGIFactory2*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory3))
	{
		IDXGIFactory3* real = (IDXGIFactory3*)(*ppvObject);
		*ppvObject = (IDXGIFactory3*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory4))
	{
		IDXGIFactory4* real = (IDXGIFactory4*)(*ppvObject);
		*ppvObject = (IDXGIFactory4*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory5))
	{
		IDXGIFactory5* real = (IDXGIFactory5*)(*ppvObject);
		*ppvObject = (IDXGIFactory5*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory6))
	{
		IDXGIFactory6* real = (IDXGIFactory6*)(*ppvObject);
		*ppvObject = (IDXGIFactory6*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == __uuidof(IDXGIFactory7))
	{
		IDXGIFactory7* real = (IDXGIFactory7*)(*ppvObject);
		*ppvObject = (IDXGIFactory7*)(new WrappedIDXGIFactory(real));
	}
	else if (riid == ID3D10Texture2D_uuid)
	{
		static bool printed = false;

		if (!printed)
		{
			printed = true;
			std::string str(ifaceName);
			LOG("RefCountDXGIObject.HandleWrap Querying " + str + "  for unsupported ID3D10Texture2D_uuid interface: " + ToString(riid));
		}

		return false;
	}
	else if (riid == Unknown_uuid)
	{
		static bool printed = false;
		if (!printed)
		{
			printed = true;
			std::string str(ifaceName);
			LOG("RefCountDXGIObject.HandleWrap Querying " + str + "  for unknown GUID: " + ToString(riid));
		}

		return false;
	}
#ifdef BLOCK_IDXGIAdapterInternal2
	else if (riid == IDXGIAdapterInternal2_uuid)
	{
		static bool printed = false;
		if (!printed)
		{
			printed = true;
			std::string str(ifaceName);
			LOG("RefCountDXGIObject.HandleWrap Querying " + str + " for unsupported/undocumented interface: IDXGIAdapterInternal2");
		}

		return false;
	}
#endif
	else
	{
		std::string str(ifaceName);
		LOG("RefCountDXGIObject.HandleWrap Querying " + str + "  for unrecognized GUID: " + ToString(riid));
	}

	return true;
}

HRESULT STDMETHODCALLTYPE RefCountDXGIObject::GetParent(
	/* [in] */ REFIID riid,
	/* [retval][out] */ void** ppParent)
{
	LOG("RefCountDXGIObject.GetParent");

	HRESULT ret = m_pReal->GetParent(riid, ppParent);

	if (ret == S_OK)
		HandleWrap("GetParent", riid, ppParent);

	return ret;
}

HRESULT RefCountDXGIObject::WrapQueryInterface(IUnknown* real, const char* ifaceName, REFIID riid, void** ppvObject)
{
	LOG("RefCountDXGIObject.WrapQueryInterface riid: " + ToString(riid));

#ifdef BLOCK_IDXGIAdapterInternal2
	// unknown/undocumented internal interface
	// {7abb6563-02bc-47c4-8ef9-acc4795edbcf}
	static const GUID IDXGIAdapterInternal2_uuid = { 0x7abb6563, 0x02bc, 0x47c4, {0x8e, 0xf9, 0xac, 0xc4, 0x79, 0x5e, 0xdb, 0xcf} };

	if (riid == IDXGIAdapterInternal2_uuid)
	{
		LOG("RefCountDXGIObject.WrapQueryInterface IDXGIAdapterInternal2 result: " + int_to_hex(E_NOINTERFACE));
		return E_NOINTERFACE;
	}
#endif

	HRESULT ret = real->QueryInterface(riid, ppvObject);
	LOG("RefCountDXGIObject.WrapQueryInterface real->QueryInterface result: " + int_to_hex(ret));

	if (ret == S_OK && HandleWrap(ifaceName, riid, ppvObject))
	{
		LOG("RefCountDXGIObject.WrapQueryInterface HandleWrap result: " + int_to_hex(ret));
		return ret;
	}

	LOG("RefCountDXGIObject.WrapQueryInterface result: E_NOINTERFACE");
	return E_NOINTERFACE;
}


WrappedIDXGIAdapter4::WrappedIDXGIAdapter4(IDXGIAdapter* real)
	: RefCountDXGIObject(real), m_pReal(real)
{
	LOG("WrappedIDXGIAdapter4.ctor");

	m_pReal1 = NULL;
	real->QueryInterface(__uuidof(IDXGIAdapter1), (void**)&m_pReal1);
	m_pReal2 = NULL;
	real->QueryInterface(__uuidof(IDXGIAdapter2), (void**)&m_pReal2);
	m_pReal3 = NULL;
	real->QueryInterface(__uuidof(IDXGIAdapter3), (void**)&m_pReal3);
	m_pReal4 = NULL;
	real->QueryInterface(__uuidof(IDXGIAdapter4), (void**)&m_pReal4);

	b_spoofEnabled = true;
}

WrappedIDXGIAdapter4::~WrappedIDXGIAdapter4()
{
	LOG("WrappedIDXGIAdapter4.dtor");

	SAFE_RELEASE(m_pReal1);
	SAFE_RELEASE(m_pReal2);
	SAFE_RELEASE(m_pReal3);
	SAFE_RELEASE(m_pReal4);
	SAFE_RELEASE(m_pReal);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGIAdapter4::QueryInterface(REFIID riid, void** ppvObject)
{
	LOG("WrappedIDXGIAdapter4.QueryInterface riid: " + ToString(riid));

#ifndef BLOCK_IDXGIAdapterInternal2
	// unknown/undocumented internal interface
	// {7abb6563-02bc-47c4-8ef9-acc4795edbcf}
	static const GUID IDXGIAdapterInternal2_uuid = { 0x7abb6563, 0x02bc, 0x47c4, {0x8e, 0xf9, 0xac, 0xc4, 0x79, 0x5e, 0xdb, 0xcf} };
#endif 


	if (riid == __uuidof(IDXGIAdapter))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter");
		AddRef();
		*ppvObject = (IDXGIAdapter*)this;
		return S_OK;
	}
	else if (riid == __uuidof(IDXGIProxyAdapter))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIProxyAdapter");
		AddRef();
		*ppvObject = (IDXGIProxyAdapter*)this;
		return S_OK;
	}
	else if (riid == __uuidof(IDXGIAdapter1))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter1");

		if (m_pReal1)
		{
			AddRef();
			*ppvObject = (IDXGIAdapter1*)this;
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter1 result: OK");
			return S_OK;
		}
		else
		{
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter1 result: E_NOINTERFACE");
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIAdapter2))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter2");

		if (m_pReal2)
		{
			AddRef();
			*ppvObject = (IDXGIAdapter2*)this;
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter2 result: OK");
			return S_OK;
		}
		else
		{
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter2 result: E_NOINTERFACE");
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIAdapter3))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter3");

		if (m_pReal3)
		{
			AddRef();
			*ppvObject = (IDXGIAdapter3*)this;
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter3 result: OK");
			return S_OK;
		}
		else
		{
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter3 result: E_NOINTERFACE");
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIAdapter4))
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter4");

		if (m_pReal4)
		{
			AddRef();
			*ppvObject = (IDXGIAdapter4*)this;
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter4 result: OK");
			return S_OK;
		}
		else
		{
			LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapter4 result: E_NOINTERFACE");
			return E_NOINTERFACE;
		}
	}
#ifndef BLOCK_IDXGIAdapterInternal2
	else if (riid == IDXGIAdapterInternal2_uuid && m_pReal != nullptr)
	{
		LOG("WrappedIDXGIAdapter4.QueryInterface for IDXGIAdapterInternal2, returning real adapter");
		return m_pReal->QueryInterface(riid, ppvObject);
	}
#endif

	return RefCountDXGIObject::QueryInterface("IDXGIAdapter", riid, ppvObject);
}

WrappedIDXGIFactory::WrappedIDXGIFactory(IDXGIFactory* real)
	: RefCountDXGIObject(real), m_pReal(real)
{
	LOG("WrappedIDXGIFactory.ctor");

	m_pReal1 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory1), (void**)&m_pReal1);
	m_pReal2 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory2), (void**)&m_pReal2);
	m_pReal3 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory3), (void**)&m_pReal3);
	m_pReal4 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory4), (void**)&m_pReal4);
	m_pReal5 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory5), (void**)&m_pReal5);
	m_pReal6 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory6), (void**)&m_pReal6);
	m_pReal7 = NULL;
	real->QueryInterface(__uuidof(IDXGIFactory7), (void**)&m_pReal7);
}

WrappedIDXGIFactory::~WrappedIDXGIFactory()
{
	LOG("WrappedIDXGIFactory.dtor");

	SAFE_RELEASE(m_pReal1);
	SAFE_RELEASE(m_pReal2);
	SAFE_RELEASE(m_pReal3);
	SAFE_RELEASE(m_pReal4);
	SAFE_RELEASE(m_pReal5);
	SAFE_RELEASE(m_pReal6);
	SAFE_RELEASE(m_pReal7);
	SAFE_RELEASE(m_pReal);
}

HRESULT STDMETHODCALLTYPE WrappedIDXGIFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	LOG("WrappedIDXGIFactory.QueryInterface");

	// {713f394e-92ca-47e7-ab81-1159c2791e54}
	static const GUID IDXGIFactoryDWM_uuid = {
		0x713f394e, 0x92ca, 0x47e7, {0xab, 0x81, 0x11, 0x59, 0xc2, 0x79, 0x1e, 0x54} };

	// {1ddd77aa-9a4a-4cc8-9e55-98c196bafc8f}
	static const GUID IDXGIFactoryDWM8_uuid = {
		0x1ddd77aa, 0x9a4a, 0x4cc8, {0x9e, 0x55, 0x98, 0xc1, 0x96, 0xba, 0xfc, 0x8f} };

	if (riid == __uuidof(IDXGIFactory))
	{
		AddRef();
		*ppvObject = (IDXGIFactory*)this;
		return S_OK;
	}
	else if (riid == __uuidof(IDXGIFactory1))
	{
		if (m_pReal1)
		{
			AddRef();
			*ppvObject = (IDXGIFactory1*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory2))
	{
		if (m_pReal2)
		{
			AddRef();
			*ppvObject = (IDXGIFactory2*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory3))
	{
		if (m_pReal3)
		{
			AddRef();
			*ppvObject = (IDXGIFactory3*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory4))
	{
		if (m_pReal4)
		{
			AddRef();
			*ppvObject = (IDXGIFactory4*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory5))
	{
		if (m_pReal5)
		{
			AddRef();
			*ppvObject = (IDXGIFactory5*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory6))
	{
		if (m_pReal6)
		{
			AddRef();
			*ppvObject = (IDXGIFactory6*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == __uuidof(IDXGIFactory7))
	{
		if (m_pReal7)
		{
			AddRef();
			*ppvObject = (IDXGIFactory7*)this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	else if (riid == IDXGIFactoryDWM_uuid)
	{
		//RDCWARN("Blocking QueryInterface for IDXGIFactoryDWM");
		return E_NOINTERFACE;
	}
	else if (riid == IDXGIFactoryDWM8_uuid)
	{
		//RDCWARN("Blocking QueryInterface for IDXGIFactoryDWM8");
		return E_NOINTERFACE;
	}

	return RefCountDXGIObject::QueryInterface("IDXGIFactory", riid, ppvObject);
}

HRESULT WrappedIDXGIFactory::CreateSwapChain(IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc,
	IDXGISwapChain** ppSwapChain)
{
	LOG("WrappedIDXGIFactory.CreateSwapChain");

	return m_pReal->CreateSwapChain(pDevice, pDesc, ppSwapChain);
}

HRESULT WrappedIDXGIFactory::CreateSwapChainForHwnd(
	IUnknown* pDevice, HWND hWnd, const DXGI_SWAP_CHAIN_DESC1* pDesc,
	const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc, IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain)
{
	LOG("WrappedIDXGIFactory.CreateSwapChainForHwnd");

	return m_pReal2->CreateSwapChainForHwnd(pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT WrappedIDXGIFactory::CreateSwapChainForCoreWindow(IUnknown* pDevice, IUnknown* pWindow,
	const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain)
{
	LOG("WrappedIDXGIFactory.CreateSwapChainForCoreWindow");

	return m_pReal2->CreateSwapChainForCoreWindow(pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
}

HRESULT WrappedIDXGIFactory::CreateSwapChainForComposition(IUnknown* pDevice,
	const DXGI_SWAP_CHAIN_DESC1* pDesc,
	IDXGIOutput* pRestrictToOutput,
	IDXGISwapChain1** ppSwapChain)
{
	LOG("WrappedIDXGIFactory.CreateSwapChainForComposition");

	return m_pReal2->CreateSwapChainForComposition(pDevice, pDesc, pRestrictToOutput, ppSwapChain);
}
