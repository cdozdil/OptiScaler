#pragma once
#include <pch.h>

class VulkanHooks
{
private:
public:
    static void Hook();
    static void HookExtension();
    static void Unhook();
    static void UnhookExtension();
};