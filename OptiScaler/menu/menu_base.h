#pragma once
#include "pch.h"
#include "imgui/imgui.h"

class MenuBase
{
	static void LoadCustomFonts(ImGuiIO& io, float menuScale);
public:
    static void UpdateFonts(ImGuiIO& io, float rasterizerDensity);

	inline static ImFont* font = nullptr;
	inline static ImFont* scaledFont = nullptr;
};
