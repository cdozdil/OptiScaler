#pragma once
#include "pch.h"

namespace WorkingMode
{
    void Check();
    void DetachHooks();

    std::string DllNameA();
    std::string DllNameExA();
    std::wstring DllNameW();
    std::wstring DllNameExW();

    bool IsNvngxMode();
    bool IsWorkingWithEnabler();
    bool IsNvngxAvailable();
    bool IsModeFound();
};