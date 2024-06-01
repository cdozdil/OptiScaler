#pragma once
#include <filesystem>

namespace Util
{
	std::filesystem::path ExePath();
	std::filesystem::path DllPath();
	std::optional<std::filesystem::path> NvngxPath();

	HWND GetProcessWindow();
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
}