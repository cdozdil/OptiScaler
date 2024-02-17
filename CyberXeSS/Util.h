#pragma once
#include "Config.h"

namespace Util
{
	std::filesystem::path ExePath();
	std::filesystem::path DllPath();
};

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// Set a breakpoint on this line to catch DirectX API errors
		throw std::exception();
	}
}