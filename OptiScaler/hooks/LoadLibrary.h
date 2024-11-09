#pragma once
#include <pch.h>

class LoadLibraryHooks
{
private:
public:
    static void Hook();
    static void Unhook();
    static void AddLoad();
    static UINT LoadCount();
};
