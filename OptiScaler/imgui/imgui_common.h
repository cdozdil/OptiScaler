#pragma once

#include "../pch.h"
#include "../Util.h"
#include "../Config.h"
#include "../resource.h"
#include "../Logger.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ImGuiCommon
{
private:
    // internal values
    inline static HWND _handle = nullptr;
    inline static WNDPROC _oWndProc = nullptr;
    inline static bool _isVisible = false;
    inline static bool _isInited = false;
    //inline static bool _isResetRequested = false;

    // mipmap calculations
    inline static bool _showMipmapCalcWindow = false;
    inline static float _mipBias = 0.0f;
    inline static float _mipBiasCalculated = 0.0f;
    inline static uint32_t _mipmapUpscalerQuality = 0;
    inline static float _mipmapUpscalerRatio = 0;
    inline static uint32_t _displayWidth = 0;
    inline static uint32_t _renderWidth = 0;

    // dlss enabler
    inline static int _deLimitFps = 500;

    // reflex
    inline static float _limitFps = INFINITY;

    // fsr3x
    inline static int _fsr3xIndex = -1;

    // output scaling
    inline static float _ssRatio = 0.0f;
    inline static bool _ssEnabled = false;
    inline static bool _ssUseFsr = false;
    inline static uint32_t _ssDownsampler = 0;

    // ui scale
    inline static int _selectedScale = 5;
    inline static bool _imguiSizeUpdate = true;

    // overlay states
    inline static bool _dx11Ready = false;
    inline static bool _dx12Ready = false;
    inline static bool _vulkanReady = false;

    // font
    inline static ImFont* _optiFont = nullptr;
    inline static ImFont* _scaledOptiFont = nullptr;

    inline static void ShowTooltip(const char* tip);

    inline static void ShowHelpMarker(const char* tip);

    inline static void SeparatorWithHelpMarker(const char* label, const char* tip);

#pragma region "Hooks & WndProc"

    // for hooking
    typedef BOOL(*PFN_SetCursorPos)(int x, int y);
    typedef BOOL(*PFN_ClipCursor)(const RECT* lpRect);
    typedef UINT(*PFN_SendInput)(UINT cInputs, LPINPUT pInputs, int cbSize);
    typedef void(*PFN_mouse_event)(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
    typedef BOOL(*PFN_GetCursorPos)(LPPOINT lpPoint);
    typedef LRESULT(*PFN_SendMessageW)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    inline static PFN_SetCursorPos pfn_SetPhysicalCursorPos = nullptr;
    inline static PFN_SetCursorPos pfn_SetCursorPos = nullptr;
    inline static PFN_ClipCursor pfn_ClipCursor = nullptr;
    inline static PFN_mouse_event pfn_mouse_event = nullptr;
    inline static PFN_SendInput pfn_SendInput = nullptr;
    inline static PFN_SendMessageW pfn_SendMessageW = nullptr;
    inline static PFN_GetCursorPos pfn_GetCursorPos = nullptr;

    inline static bool pfn_SetPhysicalCursorPos_hooked = false;
    inline static bool pfn_SetCursorPos_hooked = false;
    inline static bool pfn_ClipCursor_hooked = false;
    inline static bool pfn_mouse_event_hooked = false;
    inline static bool pfn_SendInput_hooked = false;
    inline static bool pfn_SendMessageW_hooked = false;

    inline static RECT _cursorLimit = {};
    inline static POINT _lastPoint = {};

    static LRESULT hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    static BOOL hkSetPhysicalCursorPos(int x, int y);
    static BOOL hkSetCursorPos(int x, int y);
    static BOOL hkClipCursor(RECT* lpRect);
    static void hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
    static UINT hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize);
    static BOOL hkGetPhysicalCursorPos(LPPOINT lpPoint);

    static void AttachHooks();
    static void DetachHooks();

    inline static ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam);

    //Win32 message handler
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma endregion

    static std::string GetBackendName(std::string* code);

    static std::string GetBackendCode(const NVNGX_Api api);

    static void GetCurrentBackendInfo(const NVNGX_Api api, std::string* code, std::string* name);

    static void AddDx11Backends(std::string* code, std::string* name);

    static void AddDx12Backends(std::string* code, std::string* name);

    static void AddVulkanBackends(std::string* code, std::string* name);

    static void AddResourceBarrier(std::string name, std::optional<int>* value);

    static void AddRenderPreset(std::string name, std::optional<uint32_t>* value);

    static void PopulateCombo(std::string name, std::optional<uint32_t>* value, const char* names[], const std::string desc[], int length);

public:

    static void Dx11Inited() { _dx11Ready = true; }
    static void Dx12Inited() { _dx12Ready = true; }
    static void VulkanInited() { _vulkanReady = true; }
    static bool IsInited() { return _isInited; }
    static bool IsVisible() { return _isVisible; }
    static HWND Handle() { return _handle; }

    static bool RenderMenu();
    static void Init(HWND InHwnd);
    static void Shutdown();
    static void HideMenu();
};
