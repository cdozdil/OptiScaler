#pragma once
#include <filesystem>
#include <xess.h>

namespace Util
{
	typedef struct _version_t
	{
		uint16_t major;
		uint16_t minor;
		uint16_t patch;
		uint16_t reserved;
	} version_t;

	std::filesystem::path ExePath();
	std::filesystem::path DllPath();
	std::optional<std::filesystem::path> NvngxPath();
	double MillisecondsNow();


	HWND GetProcessWindow();
	bool GetDLLVersion(std::wstring dllPath, version_t* versionOut);
	bool GetDLLVersion(std::wstring dllPath, xess_version_t* versionOut);
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
}