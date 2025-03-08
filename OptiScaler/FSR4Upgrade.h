#pragma once

#include <pch.h>
#include <Unknwn.h>

/* Potato_of_Doom's Implementation */

MIDL_INTERFACE("b58d6601-7401-4234-8180-6febfc0e484c")
IAmdExtFfxApi : public IUnknown
{
public:
	virtual HRESULT UpdateFfxApiProvider(void* pData, uint32_t dataSizeInBytes) = 0;
};

typedef HRESULT(STDMETHODCALLTYPE* PFN_UpdateFfxApiProvider)(void* pData, uint32_t dataSizeInBytes);

struct AmdExtFfxApi : public IAmdExtFfxApi
{
	PFN_UpdateFfxApiProvider pfnUpdateFfxApiProvider = nullptr;

	HRESULT STDMETHODCALLTYPE UpdateFfxApiProvider(void* pData, uint32_t dataSizeInBytes) override
	{
		LOG_INFO("UpdateFfxApiProvider called");

		if (pfnUpdateFfxApiProvider == nullptr)
		{
			// TODO: Add ini option for it
			auto fsr4Module = LoadLibrary(L"amdxcffx64.dll");

			if (fsr4Module == nullptr)
			{
				LOG_ERROR("Failed to load amdxcffx64.dll");
				return E_NOINTERFACE;
			}

			pfnUpdateFfxApiProvider = (PFN_UpdateFfxApiProvider)GetProcAddress(fsr4Module, "UpdateFfxApiProvider");

			if (pfnUpdateFfxApiProvider == nullptr)
			{
				LOG_ERROR("Failed to get UpdateFfxApiProvider");
				return E_NOINTERFACE;
			}

		}

		if (pfnUpdateFfxApiProvider != nullptr)
			return pfnUpdateFfxApiProvider(pData, dataSizeInBytes);

		return E_NOINTERFACE;
	}

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
	{
		return E_NOTIMPL;
	}

	ULONG __stdcall AddRef(void) override
	{
		return 0;
	}
	
	ULONG __stdcall Release(void) override
	{
		return 0;
	}
};
