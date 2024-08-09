#pragma once
#include "pch.h"

class WorkingMode
{
public:
    static void Check();

    static std::string DllNameA();
    static std::string DllNameExA();
    static std::wstring DllNameW();
    static std::wstring DllNameExW();

    static bool IsNvngxMode();
    static bool IsWorkingWithEnabler();
    static bool IsNvngxAvailable();
    static bool IsModeFound();
};