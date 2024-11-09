#pragma once
#include "pch.h"

class WorkingMode
{
private:
    inline static std::vector<std::string> dllNames;
    inline static std::vector<std::wstring> dllNamesW;
    inline static std::string filename;
    inline static std::string lCaseFilename;
    inline static bool isNvngxMode = false;
    inline static bool isDxgiMode = false;
    inline static bool isWorkingWithEnabler = false;


public:
    static bool Check();

    static std::string FileName() { return filename; }
    static std::string FileNameLCase() { return lCaseFilename; }
    static bool IsNvngxMode() { return isNvngxMode; }
    static bool IsWorkingWithEnabler() { return isWorkingWithEnabler; }
    static bool IsDxgiMode() { return isDxgiMode; }
    static std::vector<std::string> DllNames() { return dllNames; }
    static std::vector<std::wstring> DllNamesW() { return dllNamesW; }
};