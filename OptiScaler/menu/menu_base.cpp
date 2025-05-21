#include "font/Hack_Compressed.h"
#include "imgui/imgui.h"
#include "imgui/misc/freetype/imgui_freetype.h"
#include "menu_base.h"
#include <Config.h>

// TODO: maybe a vector of fonts?
void MenuBase::LoadCustomFonts(ImGuiIO& io, float menuScale)
{
    LOG_INFO("Loading fonts with scale of: {}", menuScale);
    ImFontAtlas* atlas = io.Fonts;
    atlas->Clear();
    constexpr float fontSize = 14.0f; // just changing this doesn't make other elements scale ideally

    // This automatically becomes the next default font
    ImFontConfig fontConfig;
    // fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
    font = atlas->AddFontFromMemoryCompressedBase85TTF(hack_compressed_compressed_data_base85,
                                                       std::round(menuScale * fontSize), &fontConfig);
    if (!font)
    {
        LOG_ERROR("Couldn't create font");
        return;
    }

    ImFontConfig scaledFontConfig;
    constexpr auto scaledFontScale = 3.0f;
    // fontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
    scaledFont = atlas->AddFontFromMemoryCompressedBase85TTF(
        hack_compressed_compressed_data_base85, std::round(scaledFontScale * menuScale * fontSize), &scaledFontConfig);
    if (!scaledFont)
    {
        LOG_ERROR("Couldn't create scaled font");
        return;
    }

    io.Fonts->Build();
}

void MenuBase::UpdateFonts(ImGuiIO& io, float rasterizerDensity)
{
    static float lastDensity = 0.0f;

    if (lastDensity != rasterizerDensity || io.Fonts->Fonts.empty())
    {
        if (Config::Instance()->UseHQFont.value_or_default())
            LoadCustomFonts(io, rasterizerDensity);

        lastDensity = rasterizerDensity;
    }
}
