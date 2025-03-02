#include "menu_common.h"

#include "DLSSG_Mod.h"

#include "font/Hack_Compressed.h"
#include <nvapi/fakenvapi.h>
#include <nvapi/ReflexHooks.h>
#include <proxies/FfxApi_Proxy.h>
#include <hooks/HooksDx.h>

#include <imgui/imgui_internal.h>

static ImVec2 overlayPosition(-1000.0f, -1000.0f);
static bool _hdrTonemapApplied = false;
static ImVec4 SdrColors[ImGuiCol_COUNT];
static bool receivingWmInputs = false;
static bool inputMenu = false;
static bool inputFps = false;
static bool inputFpsCycle = false;

void MenuCommon::ShowTooltip(const char* tip) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::BeginTooltip();
        ImGui::Text(tip);
        ImGui::EndTooltip();
    }
}

void MenuCommon::ShowHelpMarker(const char* tip)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    ShowTooltip(tip);
}

void MenuCommon::SeparatorWithHelpMarker(const char* label, const char* tip)
{
    auto marker = "(?) ";
    ImGui::SeparatorTextEx(0, label, ImGui::FindRenderedTextEnd(label), ImGui::CalcTextSize(marker, ImGui::FindRenderedTextEnd(marker)).x);
    ShowHelpMarker(tip);
}

LRESULT MenuCommon::hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (_isVisible && Msg == 0x0020)
        return TRUE;
    else
        return pfn_SendMessageW(hWnd, Msg, wParam, lParam);
}

BOOL MenuCommon::hkSetPhysicalCursorPos(int x, int y)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SetPhysicalCursorPos(x, y);
}

BOOL MenuCommon::hkGetPhysicalCursorPos(LPPOINT lpPoint)
{
    if (_isVisible)
    {
        lpPoint->x = _lastPoint.x;
        lpPoint->y = _lastPoint.y;
        return TRUE;
    }
    else
        return pfn_GetCursorPos(lpPoint);
}

BOOL MenuCommon::hkSetCursorPos(int x, int y)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SetCursorPos(x, y);
}

BOOL MenuCommon::hkClipCursor(RECT* lpRect)
{
    if (_isVisible)
        return TRUE;
    else
    {
        return pfn_ClipCursor(lpRect);
    }
}

void MenuCommon::hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
{
    if (_isVisible)
        return;
    else
        pfn_mouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
}

UINT MenuCommon::hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SendInput(cInputs, pInputs, cbSize);
}

void MenuCommon::AttachHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Detour the functions
    pfn_SetPhysicalCursorPos = reinterpret_cast<PFN_SetCursorPos>(DetourFindFunction("user32.dll", "SetPhysicalCursorPos"));
    pfn_SetCursorPos = reinterpret_cast<PFN_SetCursorPos>(DetourFindFunction("user32.dll", "SetCursorPos"));
    pfn_ClipCursor = reinterpret_cast<PFN_ClipCursor>(DetourFindFunction("user32.dll", "ClipCursor"));
    pfn_mouse_event = reinterpret_cast<PFN_mouse_event>(DetourFindFunction("user32.dll", "mouse_event"));
    pfn_SendInput = reinterpret_cast<PFN_SendInput>(DetourFindFunction("user32.dll", "SendInput"));
    pfn_SendMessageW = reinterpret_cast<PFN_SendMessageW>(DetourFindFunction("user32.dll", "SendMessageW"));

    if (pfn_SetPhysicalCursorPos && (pfn_SetPhysicalCursorPos != pfn_SetCursorPos))
        pfn_SetPhysicalCursorPos_hooked = (DetourAttach(&(PVOID&)pfn_SetPhysicalCursorPos, hkSetPhysicalCursorPos) == 0);

    if (pfn_SetCursorPos)
        pfn_SetCursorPos_hooked = (DetourAttach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos) == 0);

    if (pfn_ClipCursor)
        pfn_ClipCursor_hooked = (DetourAttach(&(PVOID&)pfn_ClipCursor, hkClipCursor) == 0);

    if (pfn_mouse_event)
        pfn_mouse_event_hooked = (DetourAttach(&(PVOID&)pfn_mouse_event, hkmouse_event) == 0);

    if (pfn_SendInput)
        pfn_SendInput_hooked = (DetourAttach(&(PVOID&)pfn_SendInput, hkSendInput) == 0);

    if (pfn_SendMessageW)
        pfn_SendMessageW_hooked = (DetourAttach(&(PVOID&)pfn_SendMessageW, hkSendMessageW) == 0);

    DetourTransactionCommit();
}

void MenuCommon::DetachHooks()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    if (pfn_SetPhysicalCursorPos_hooked)
        DetourDetach(&(PVOID&)pfn_SetPhysicalCursorPos, hkSetPhysicalCursorPos);

    if (pfn_SetCursorPos_hooked)
        DetourDetach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos);

    if (pfn_ClipCursor_hooked)
        DetourDetach(&(PVOID&)pfn_ClipCursor, hkClipCursor);

    if (pfn_mouse_event_hooked)
        DetourDetach(&(PVOID&)pfn_mouse_event, hkmouse_event);

    if (pfn_SendInput_hooked)
        DetourDetach(&(PVOID&)pfn_SendInput, hkSendInput);

    if (pfn_SendMessageW_hooked)
        DetourDetach(&(PVOID&)pfn_SendMessageW, hkSendMessageW);

    pfn_SetPhysicalCursorPos_hooked = false;
    pfn_SetCursorPos_hooked = false;
    pfn_mouse_event_hooked = false;
    pfn_SendInput_hooked = false;
    pfn_SendMessageW_hooked = false;

    pfn_SetPhysicalCursorPos = nullptr;
    pfn_SetCursorPos = nullptr;
    pfn_mouse_event = nullptr;
    pfn_SendInput = nullptr;
    pfn_SendMessageW = nullptr;

    DetourTransactionCommit();
}

ImGuiKey MenuCommon::ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
{
    switch (wParam)
    {
        case VK_TAB: return ImGuiKey_Tab;
        case VK_LEFT: return ImGuiKey_LeftArrow;
        case VK_RIGHT: return ImGuiKey_RightArrow;
        case VK_UP: return ImGuiKey_UpArrow;
        case VK_DOWN: return ImGuiKey_DownArrow;
        case VK_PRIOR: return ImGuiKey_PageUp;
        case VK_NEXT: return ImGuiKey_PageDown;
        case VK_HOME: return ImGuiKey_Home;
        case VK_END: return ImGuiKey_End;
        case VK_INSERT: return ImGuiKey_Insert;
        case VK_DELETE: return ImGuiKey_Delete;
        case VK_BACK: return ImGuiKey_Backspace;
        case VK_SPACE: return ImGuiKey_Space;
        case VK_RETURN: return ImGuiKey_Enter;
        case VK_ESCAPE: return ImGuiKey_Escape;
        case VK_OEM_7: return ImGuiKey_Apostrophe;
        case VK_OEM_COMMA: return ImGuiKey_Comma;
        case VK_OEM_MINUS: return ImGuiKey_Minus;
        case VK_OEM_PERIOD: return ImGuiKey_Period;
        case VK_OEM_2: return ImGuiKey_Slash;
        case VK_OEM_1: return ImGuiKey_Semicolon;
        case VK_OEM_PLUS: return ImGuiKey_Equal;
        case VK_OEM_4: return ImGuiKey_LeftBracket;
        case VK_OEM_5: return ImGuiKey_Backslash;
        case VK_OEM_6: return ImGuiKey_RightBracket;
        case VK_OEM_3: return ImGuiKey_GraveAccent;
        case VK_CAPITAL: return ImGuiKey_CapsLock;
        case VK_SCROLL: return ImGuiKey_ScrollLock;
        case VK_NUMLOCK: return ImGuiKey_NumLock;
        case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
        case VK_PAUSE: return ImGuiKey_Pause;
        case VK_NUMPAD0: return ImGuiKey_Keypad0;
        case VK_NUMPAD1: return ImGuiKey_Keypad1;
        case VK_NUMPAD2: return ImGuiKey_Keypad2;
        case VK_NUMPAD3: return ImGuiKey_Keypad3;
        case VK_NUMPAD4: return ImGuiKey_Keypad4;
        case VK_NUMPAD5: return ImGuiKey_Keypad5;
        case VK_NUMPAD6: return ImGuiKey_Keypad6;
        case VK_NUMPAD7: return ImGuiKey_Keypad7;
        case VK_NUMPAD8: return ImGuiKey_Keypad8;
        case VK_NUMPAD9: return ImGuiKey_Keypad9;
        case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
        case VK_DIVIDE: return ImGuiKey_KeypadDivide;
        case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case VK_ADD: return ImGuiKey_KeypadAdd;
        case VK_LSHIFT: return ImGuiKey_LeftShift;
        case VK_LCONTROL: return ImGuiKey_LeftCtrl;
        case VK_LMENU: return ImGuiKey_LeftAlt;
        case VK_LWIN: return ImGuiKey_LeftSuper;
        case VK_RSHIFT: return ImGuiKey_RightShift;
        case VK_RCONTROL: return ImGuiKey_RightCtrl;
        case VK_RMENU: return ImGuiKey_RightAlt;
        case VK_RWIN: return ImGuiKey_RightSuper;
        case VK_APPS: return ImGuiKey_Menu;
        case '0': return ImGuiKey_0;
        case '1': return ImGuiKey_1;
        case '2': return ImGuiKey_2;
        case '3': return ImGuiKey_3;
        case '4': return ImGuiKey_4;
        case '5': return ImGuiKey_5;
        case '6': return ImGuiKey_6;
        case '7': return ImGuiKey_7;
        case '8': return ImGuiKey_8;
        case '9': return ImGuiKey_9;
        case 'A': return ImGuiKey_A;
        case 'B': return ImGuiKey_B;
        case 'C': return ImGuiKey_C;
        case 'D': return ImGuiKey_D;
        case 'E': return ImGuiKey_E;
        case 'F': return ImGuiKey_F;
        case 'G': return ImGuiKey_G;
        case 'H': return ImGuiKey_H;
        case 'I': return ImGuiKey_I;
        case 'J': return ImGuiKey_J;
        case 'K': return ImGuiKey_K;
        case 'L': return ImGuiKey_L;
        case 'M': return ImGuiKey_M;
        case 'N': return ImGuiKey_N;
        case 'O': return ImGuiKey_O;
        case 'P': return ImGuiKey_P;
        case 'Q': return ImGuiKey_Q;
        case 'R': return ImGuiKey_R;
        case 'S': return ImGuiKey_S;
        case 'T': return ImGuiKey_T;
        case 'U': return ImGuiKey_U;
        case 'V': return ImGuiKey_V;
        case 'W': return ImGuiKey_W;
        case 'X': return ImGuiKey_X;
        case 'Y': return ImGuiKey_Y;
        case 'Z': return ImGuiKey_Z;
        case VK_F1: return ImGuiKey_F1;
        case VK_F2: return ImGuiKey_F2;
        case VK_F3: return ImGuiKey_F3;
        case VK_F4: return ImGuiKey_F4;
        case VK_F5: return ImGuiKey_F5;
        case VK_F6: return ImGuiKey_F6;
        case VK_F7: return ImGuiKey_F7;
        case VK_F8: return ImGuiKey_F8;
        case VK_F9: return ImGuiKey_F9;
        case VK_F10: return ImGuiKey_F10;
        case VK_F11: return ImGuiKey_F11;
        case VK_F12: return ImGuiKey_F12;
        case VK_F13: return ImGuiKey_F13;
        case VK_F14: return ImGuiKey_F14;
        case VK_F15: return ImGuiKey_F15;
        case VK_F16: return ImGuiKey_F16;
        case VK_F17: return ImGuiKey_F17;
        case VK_F18: return ImGuiKey_F18;
        case VK_F19: return ImGuiKey_F19;
        case VK_F20: return ImGuiKey_F20;
        case VK_F21: return ImGuiKey_F21;
        case VK_F22: return ImGuiKey_F22;
        case VK_F23: return ImGuiKey_F23;
        case VK_F24: return ImGuiKey_F24;
        case VK_BROWSER_BACK: return ImGuiKey_AppBack;
        case VK_BROWSER_FORWARD: return ImGuiKey_AppForward;
        default: return ImGuiKey_None;
    }
}

//Win32 message handler
LRESULT MenuCommon::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    //LOG_TRACE("msg: {:X}, wParam: {:X}, lParam: {:X}", msg, wParam, lParam);

    if (!State::Instance().isShuttingDown &&
        (msg == WM_QUIT || msg == WM_CLOSE || msg == WM_DESTROY || /* classic messages but they are a bit late to capture */
        (msg == WM_SYSCOMMAND && wParam == SC_CLOSE /* window close*/)))
    {
        LOG_WARN("IsShuttingDown = true");
        State::Instance().isShuttingDown = true;
        return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
    }

    if (State::Instance().isShuttingDown)
        return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);

    if (!_dx11Ready && !_dx12Ready && !_vulkanReady)
    {
        if (_isVisible)
        {
            LOG_INFO("No active features, closing ImGui");

            if (pfn_ClipCursor_hooked)
                pfn_ClipCursor(&_cursorLimit);

            _isVisible = false;
            _showMipmapCalcWindow = false;

            io.MouseDrawCursor = false;
            io.WantCaptureKeyboard = false;
            io.WantCaptureMouse = false;
        }

        return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
    }

    bool rawRead = false;
    ImGuiKey imguiKey;
    RAWINPUT rawData{};
    UINT rawDataSize = sizeof(rawData);

    if (msg == WM_INPUT && GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &rawData, &rawDataSize, sizeof(rawData.data)) != (UINT)-1)
    {
        auto rawCode = GET_RAWINPUT_CODE_WPARAM(wParam);
        rawRead = true;
        receivingWmInputs = true;
        bool isKeyUp = (rawData.data.keyboard.Flags & RI_KEY_BREAK) != 0;
        if (isKeyUp && rawData.header.dwType == RIM_TYPEKEYBOARD && rawData.data.keyboard.VKey != 0)
        {
            if (!inputMenu)
                inputMenu = rawData.data.keyboard.VKey == Config::Instance()->ShortcutKey.value_or_default();

            if (!inputFps)
                inputFps = rawData.data.keyboard.VKey == Config::Instance()->FpsShortcutKey.value_or_default();

            if (!inputFpsCycle)
                inputFpsCycle = rawData.data.keyboard.VKey == Config::Instance()->FpsCycleShortcutKey.value_or_default();
        }
    }

    if (!inputMenu)
        inputMenu = msg == WM_KEYUP && wParam == Config::Instance()->ShortcutKey.value_or_default();

    if (!inputFps)
        inputFps = msg == WM_KEYUP && wParam == Config::Instance()->FpsShortcutKey.value_or_default();

    if (!inputFpsCycle)
        inputFpsCycle = msg == WM_KEYUP && wParam == Config::Instance()->FpsCycleShortcutKey.value_or_default();

    // SHIFT + DEL - Debug dump
    if (msg == WM_KEYUP && wParam == VK_DELETE && (GetKeyState(VK_SHIFT) & 0x8000))
    {
        State::Instance().xessDebug = true;
        return TRUE;
    }

    // ImGui
    if (_isVisible)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        {
            LOG_TRACE("ImGui handled, hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);
            return TRUE;
        }

        switch (msg)
        {
            case WM_KEYUP:
                if (wParam != Config::Instance()->ShortcutKey.value_or_default())
                    return TRUE;

                imguiKey = ImGui_ImplWin32_VirtualKeyToImGuiKey(wParam);
                io.AddKeyEvent(imguiKey, false);

                break;

            case WM_LBUTTONDOWN:
                io.AddMouseButtonEvent(0, true);
                return TRUE;

            case WM_LBUTTONUP:
                io.AddMouseButtonEvent(0, false);
                return TRUE;

            case WM_RBUTTONDOWN:
                io.AddMouseButtonEvent(1, true);
                return TRUE;

            case WM_RBUTTONUP:
                io.AddMouseButtonEvent(1, false);
                return TRUE;

            case WM_MBUTTONDOWN:
                io.AddMouseButtonEvent(2, true);
                return TRUE;

            case WM_MBUTTONUP:
                io.AddMouseButtonEvent(2, false);
                return TRUE;

            case WM_LBUTTONDBLCLK:
                io.AddMouseButtonEvent(0, true);
                return TRUE;

            case WM_RBUTTONDBLCLK:
                io.AddMouseButtonEvent(1, true);
                return TRUE;

            case WM_MBUTTONDBLCLK:
                io.AddMouseButtonEvent(2, true);
                return TRUE;

            case WM_KEYDOWN:
                imguiKey = ImGui_ImplWin32_VirtualKeyToImGuiKey(wParam);
                io.AddKeyEvent(imguiKey, true);

                break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_MOUSEMOVE:
            case WM_SETCURSOR:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_XBUTTONDBLCLK:
                LOG_TRACE("switch handled, hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);
                return TRUE;

            case WM_INPUT:
                if (!rawRead)
                    return TRUE;

                if (rawData.header.dwType == RIM_TYPEMOUSE)
                {
                    if (rawData.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
                        io.AddMouseButtonEvent(0, true);
                    else if (rawData.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
                        io.AddMouseButtonEvent(0, false);
                    if (rawData.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                        io.AddMouseButtonEvent(1, true);
                    else if (rawData.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                        io.AddMouseButtonEvent(1, false);
                    if (rawData.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                        io.AddMouseButtonEvent(2, true);
                    else if (rawData.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
                        io.AddMouseButtonEvent(2, false);

                    if (rawData.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
                        io.AddMouseWheelEvent(0, static_cast<short>(rawData.data.mouse.usButtonData) / WHEEL_DELTA);
                }

                else
                    LOG_TRACE("WM_INPUT hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);

                return TRUE;

            default:
                break;
        }
    }

    return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
}

std::string MenuCommon::GetBackendName(std::string* code)
{
    if (*code == "fsr21")
        return "FSR 2.1.2";

    if (*code == "fsr22")
        return "FSR 2.2.1";

    if (*code == "fsr31")
        return "FSR 3.X";

    if (*code == "fsr21_12")
        return "FSR 2.1.2 w/Dx12";

    if (*code == "fsr22_12")
        return "FSR 2.2.1 w/Dx12";

    if (*code == "fsr31_12")
        return "FSR 3.X w/Dx12";

    if (*code == "xess")
        return "XeSS";

    if (*code == "dlss")
        return "DLSS";

    return "????";
}

std::string MenuCommon::GetBackendCode(const API api)
{
    std::string code;

    if (api == DX11)
        code = Config::Instance()->Dx11Upscaler.value_or_default();
    else if (api == DX12)
        code = Config::Instance()->Dx12Upscaler.value_or_default();
    else
        code = Config::Instance()->VulkanUpscaler.value_or_default();

    return code;
}

void MenuCommon::GetCurrentBackendInfo(const API api, std::string* code, std::string* name)
{
    *code = GetBackendCode(api);
    *name = GetBackendName(code);
}

void MenuCommon::AddDx11Backends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (State::Instance().newBackend == "fsr22" || (State::Instance().newBackend == "" && *code == "fsr22"))
        selectedUpscalerName = "FSR 2.2.1";
    else if (State::Instance().newBackend == "fsr22_12" || (State::Instance().newBackend == "" && *code == "fsr22_12"))
        selectedUpscalerName = "FSR 2.2.1 w/Dx12";
    else if (State::Instance().newBackend == "fsr21_12" || (State::Instance().newBackend == "" && *code == "fsr21_12"))
        selectedUpscalerName = "FSR 2.1.2 w/Dx12";
    else if (State::Instance().newBackend == "fsr31" || (State::Instance().newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (State::Instance().newBackend == "fsr31_12" || (State::Instance().newBackend == "" && *code == "fsr31_12"))
        selectedUpscalerName = "FSR 3.X w/Dx12";
    //else if (State::Instance().newBackend == "fsr304" || (State::Instance().newBackend == "" && *code == "fsr304"))
    //    selectedUpscalerName = "FSR 3.0.4";
    else if (Config::Instance()->DLSSEnabled.value_or_default() && (State::Instance().newBackend == "dlss" || (State::Instance().newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "XeSS w/Dx12";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            State::Instance().newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            State::Instance().newBackend = "fsr31";

        if (ImGui::Selectable("XeSS w/Dx12", *code == "xess"))
            State::Instance().newBackend = "xess";

        if (ImGui::Selectable("FSR 2.1.2 w/Dx12", *code == "fsr21_12"))
            State::Instance().newBackend = "fsr21_12";

        if (ImGui::Selectable("FSR 2.2.1 w/Dx12", *code == "fsr22_12"))
            State::Instance().newBackend = "fsr22_12";

        if (ImGui::Selectable("FSR 3.X w/Dx12", *code == "fsr31_12"))
            State::Instance().newBackend = "fsr31_12";

        if (Config::Instance()->DLSSEnabled.value_or_default() && ImGui::Selectable("DLSS", *code == "dlss"))
            State::Instance().newBackend = "dlss";

        ImGui::EndCombo();
    }
}

void MenuCommon::AddDx12Backends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (State::Instance().newBackend == "fsr21" || (State::Instance().newBackend == "" && *code == "fsr21"))
        selectedUpscalerName = "FSR 2.1.2";
    else if (State::Instance().newBackend == "fsr22" || (State::Instance().newBackend == "" && *code == "fsr22"))
        selectedUpscalerName = "FSR 2.2.1";
    else if (State::Instance().newBackend == "fsr31" || (State::Instance().newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (Config::Instance()->DLSSEnabled.value_or_default() && (State::Instance().newBackend == "dlss" || (State::Instance().newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "XeSS";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("XeSS", *code == "xess"))
            State::Instance().newBackend = "xess";

        if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
            State::Instance().newBackend = "fsr21";

        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            State::Instance().newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            State::Instance().newBackend = "fsr31";

        if (Config::Instance()->DLSSEnabled.value_or_default() && ImGui::Selectable("DLSS", *code == "dlss"))
            State::Instance().newBackend = "dlss";

        ImGui::EndCombo();
    }
}

void MenuCommon::AddVulkanBackends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (State::Instance().newBackend == "fsr21" || (State::Instance().newBackend == "" && *code == "fsr21"))
        selectedUpscalerName = "FSR 2.1.2";
    else if (State::Instance().newBackend == "fsr31" || (State::Instance().newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (Config::Instance()->DLSSEnabled.value_or_default() && (State::Instance().newBackend == "dlss" || (State::Instance().newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "FSR 2.2.1";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
            State::Instance().newBackend = "fsr21";

        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            State::Instance().newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            State::Instance().newBackend = "fsr31";

        if (Config::Instance()->DLSSEnabled.value_or_default() && ImGui::Selectable("DLSS", *code == "dlss"))
            State::Instance().newBackend = "dlss";

        ImGui::EndCombo();
    }
}

template <HasDefaultValue B>
void MenuCommon::AddResourceBarrier(std::string name, CustomOptional<int32_t, B>* value)
{
    const char* states[] = { "AUTO", "COMMON", "VERTEX_AND_CONSTANT_BUFFER", "INDEX_BUFFER", "RENDER_TARGET", "UNORDERED_ACCESS", "DEPTH_WRITE",
        "DEPTH_READ", "NON_PIXEL_SHADER_RESOURCE", "PIXEL_SHADER_RESOURCE", "STREAM_OUT", "INDIRECT_ARGUMENT", "COPY_DEST", "COPY_SOURCE",
        "RESOLVE_DEST", "RESOLVE_SOURCE", "RAYTRACING_ACCELERATION_STRUCTURE", "SHADING_RATE_SOURCE", "GENERIC_READ", "ALL_SHADER_RESOURCE",
        "PRESENT", "PREDICATION", "VIDEO_DECODE_READ", "VIDEO_DECODE_WRITE", "VIDEO_PROCESS_READ", "VIDEO_PROCESS_WRITE", "VIDEO_ENCODE_READ",
        "VIDEO_ENCODE_WRITE" };
    const int values[] = { -1, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 4194304, 16777216, 2755, 192, 0, 310, 65536,
        131072, 262144, 524288, 2097152, 8388608 };


    int selected = value->value_or(-1);

    const char* selectedName = "";

    for (int n = 0; n < 28; n++)
    {
        if (values[n] == selected)
        {
            selectedName = states[n];
            break;
        }
    }

    if (ImGui::BeginCombo(name.c_str(), selectedName))
    {
        if (ImGui::Selectable(states[0], !value->has_value()))
            value->reset();

        for (int n = 1; n < 28; n++)
        {
            if (ImGui::Selectable(states[n], selected == values[n]))
                *value = values[n];
        }

        ImGui::EndCombo();
    }
}

template <HasDefaultValue B>
void MenuCommon::AddDLSSRenderPreset(std::string name, CustomOptional<uint32_t, B>* value)
{
    const char* presets[] = { "DEFAULT", "PRESET A", "PRESET B", "PRESET C", "PRESET D", "PRESET E", "PRESET F", "PRESET G",
                             "PRESET H", "PRESET I", "PRESET J", "PRESET K", "PRESET L", "PRESET M", "PRESET N", "PRESET O",
                             "Latest" };
    const std::string presetsDesc[] = { "Whatever the game uses",
        "Intended for Performance/Balanced/Quality modes.\nAn older variant best suited to combat ghosting for elements with missing inputs, such as motion vectors.",
        "Intended for Ultra Performance mode.\nSimilar to Preset A but for Ultra Performance mode.",
        "Intended for Performance/Balanced/Quality modes.\nGenerally favors current frame information;\nwell suited for fast-paced game content.",
        "Default preset for Performance/Balanced/Quality modes;\ngenerally favors image stability.",
        "DLSS 3.7+, a better D preset",
        "Default preset for Ultra Performance and DLAA modes",
        "Unused",

        "Unused",
        "Unused",
        "Transformers",
        "Transformers 2",
        "Unused",
        "Unused",
        "Unused",
        "Unused",

        "Latest supported by the dll"
    };

    if (value->value_or_default() == 0x00FFFFFF)
        *value = 16;

    PopulateCombo(name, value, presets, presetsDesc, std::size(presets));

    // Value for latest preset
    if (value->value_or_default() == 16)
        *value = 0x00FFFFFF;
}

template <HasDefaultValue B>
void MenuCommon::AddDLSSDRenderPreset(std::string name, CustomOptional<uint32_t, B>* value)
{
    const char* presets[] = { "DEFAULT", "PRESET A", "PRESET B", "PRESET C", "PRESET D", "PRESET E", "PRESET F", "PRESET G",
                             "PRESET H", "PRESET I", "PRESET J", "PRESET K", "PRESET L", "PRESET M", "PRESET N", "PRESET O",
                             "Latest" };
    const std::string presetsDesc[] = { "Whatever the game uses",
        "CNN 1",
        "CNN 2",
        "CNN 3",
        "Transformers",
        "Transformers 2",
        "Unused",
        "Unused",

        "Unused",
        "Unused",
        "Unused",
        "Unused",
        "Unused",
        "Unused",
        "Unused",
        "Unused",

        "Latest supported by the dll"
    };

    if (value->value_or_default() == 0x00FFFFFF)
        *value = 16;

    PopulateCombo(name, value, presets, presetsDesc, std::size(presets));

    // Value for latest preset
    if (value->value_or_default() == 16)
        *value = 0x00FFFFFF;
}

template <HasDefaultValue B>
void MenuCommon::PopulateCombo(std::string name, CustomOptional<uint32_t, B>* value, const char* names[], const std::string desc[], int length, const uint8_t disabledMask[], bool firstAsDefault) {
    int selected = value->value_or(0);

    const char* selectedName = "";

    for (int n = 0; n < length; n++)
    {
        if (n == selected)
        {
            selectedName = names[n];
            break;
        }
    }

    if (ImGui::BeginCombo(name.c_str(), selectedName))
    {
        if (ImGui::Selectable(names[0], !value->has_value()))
        {
            if (firstAsDefault)
                value->reset();
            else
                *value = 0;
        }

        if (!desc[0].empty())
            ShowTooltip(desc[0].c_str());

        for (int n = 1; n < length; n++)
        {
            if (disabledMask && disabledMask[n])
                ImGui::BeginDisabled();

            if (ImGui::Selectable(names[n], selected == n))
            {
                if (n != selected)
                    *value = n;
            }

            if (!desc[n].empty())
                ShowTooltip(desc[n].c_str());

            if (disabledMask && disabledMask[n])
                ImGui::EndDisabled();
        }

        ImGui::EndCombo();
    }
}

static ImVec4 toneMapColor(const ImVec4& color)
{
    // Apply tone mapping (e.g., Reinhard tone mapping)
    float luminance = 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
    float mappedLuminance = luminance / (1.0f + luminance);
    float scale = mappedLuminance / luminance;

    return ImVec4(color.x * scale, color.y * scale, color.z * scale, color.w);
}

static void MenuHdrCheck(ImGuiIO io)
{
    // If game is using HDR, apply tone mapping to the ImGui style
    if (State::Instance().isHdrActive ||
        (!Config::Instance()->OverlayMenu.value_or_default() && (State::Instance().currentFeature->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_IsHDR) > 0))
    {
        if (!_hdrTonemapApplied)
        {
            ImGuiStyle& style = ImGui::GetStyle();

            CopyMemory(SdrColors, style.Colors, sizeof(style.Colors));

            // Apply tone mapping to the ImGui style
            for (int i = 0; i < ImGuiCol_COUNT; ++i)
            {
                ImVec4 color = style.Colors[i];
                style.Colors[i] = toneMapColor(color);
            }

            _hdrTonemapApplied = true;
        }
    }
    else
    {
        if (_hdrTonemapApplied)
        {
            ImGuiStyle& style = ImGui::GetStyle();
            CopyMemory(style.Colors, SdrColors, sizeof(style.Colors));
            _hdrTonemapApplied = false;
        }
    }
}

static void MenuSizeCheck(ImGuiIO io)
{
    // Calculate menu scale according to display resolution
    {
        if (!Config::Instance()->MenuScale.has_value())
        {
            // 1000p is minimum for 1.0 menu ratio
            Config::Instance()->MenuScale = (float)((int)((float)io.DisplaySize.y / 100.0f)) / 10.0f;

            if (Config::Instance()->MenuScale.value() > 1.0f)
                Config::Instance()->MenuScale.value() = 1.0f;

            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(Config::Instance()->MenuScale.value());

            if (Config::Instance()->MenuScale.value() < 1.0f)
                style.MouseCursorScale = 1.0f;
        }

        if (Config::Instance()->MenuScale.value() < 0.5f)
            Config::Instance()->MenuScale = 0.5f;

        if (Config::Instance()->MenuScale.value() > 2.0f)
            Config::Instance()->MenuScale = 2.0f;
    }
}

bool MenuCommon::RenderMenu()
{
    if (!_isInited)
        return false;

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Handle Inputs
    {
        if (inputFps)
        {
            Config::Instance()->ShowFps = !Config::Instance()->ShowFps.value_or_default();
            inputFps = false;
            return false;
        }

        if (inputFpsCycle && Config::Instance()->ShowFps.value_or_default())
            Config::Instance()->FpsOverlayType = (Config::Instance()->FpsOverlayType.value_or_default() + 1) % 5;

        if (inputMenu)
        {
            inputMenu = false;
            _isVisible = !_isVisible;

            LOG_DEBUG("Menu key pressed, {0}", _isVisible ? "opening ImGui" : "closing ImGui");

            if (_isVisible)
            {
                Config::Instance()->ReloadFakenvapi();
                auto dllPath = Util::DllPath().parent_path() / "dlssg_to_fsr3_amd_is_better.dll";
                State::Instance().NukemsFilesAvailable = std::filesystem::exists(dllPath);

                io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

                if (pfn_ClipCursor_hooked)
                {
                    _ssRatio = 0;

                    if (GetClipCursor(&_cursorLimit))
                        pfn_ClipCursor(nullptr);

                    GetCursorPos(&_lastPoint);
                }
            }
            else
            {
                io.ConfigFlags = ImGuiConfigFlags_NavNoCaptureKeyboard | ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoKeyboard;

                if (pfn_ClipCursor_hooked)
                    pfn_ClipCursor(&_cursorLimit);

                _showMipmapCalcWindow = false;
            }

            io.MouseDrawCursor = _isVisible;
            io.WantCaptureKeyboard = _isVisible;
            io.WantCaptureMouse = _isVisible;

            return false;
        }

        inputFpsCycle = false;
    }

    // If Fps overlay is visible
    if (Config::Instance()->ShowFps.value_or_default())
    {
        ImGui_ImplWin32_NewFrame();
        MenuHdrCheck(io);
        MenuSizeCheck(io);
        ImGui::NewFrame();

        State::Instance().frameTimes.pop_front();
        State::Instance().frameTimes.push_back(1000.0 / io.Framerate);

        std::vector<float> frameTimeArray(State::Instance().frameTimes.begin(), State::Instance().frameTimes.end());
        std::vector<float> upscalerFrameTimeArray(State::Instance().upscaleTimes.begin(), State::Instance().upscaleTimes.end());
        float averageFrameTime = 0.0f;
        float averageUpscalerFT = 0.0f;

        for (size_t i = 0; i < frameTimeArray.size(); i++)
        {
            averageFrameTime += frameTimeArray[i];
            averageUpscalerFT += upscalerFrameTimeArray[i];
        }
        averageFrameTime /= frameTimeArray.size();
        averageUpscalerFT /= frameTimeArray.size();

        // Set overlay position
        ImGui::SetNextWindowPos(overlayPosition, ImGuiCond_Always);

        // Set overlay window properties
        ImGui::SetNextWindowBgAlpha(Config::Instance()->FpsOverlayAlpha.value_or_default()); // Transparent background
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0)); // Transparent border
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0)); // Transparent frame background

        ImVec4 green(0.0f, 1.0f, 0.0f, 1.0f);
        if (State::Instance().isHdrActive)
            ImGui::PushStyleColor(ImGuiCol_PlotLines, toneMapColor(green)); // Tone Map plot line color
        else
            ImGui::PushStyleColor(ImGuiCol_PlotLines, green);

        auto size = ImVec2{ 0.0f, 0.0f };
        ImGui::SetNextWindowSize(size);

        if (ImGui::Begin("Performance Overlay", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
        {
            std::string api;
            if (State::Instance().isRunningOnDXVK || State::Instance().isRunningOnLinux)
            {
                api = "VKD3D";
            }
            else
            {
                switch (State::Instance().swapchainApi)
                {
                    case Vulkan:
                        api = "VLK";
                        break;

                    case DX11:
                        api = "D3D11";
                        break;

                    case DX12:
                        api = "D3D12";
                        break;

                    default:
                        switch (State::Instance().api)
                        {
                            case Vulkan:
                                api = "VLK";
                                break;

                            case DX11:
                                api = "D3D11";
                                break;

                            case DX12:
                                api = "D3D12";
                                break;

                            default:
                                api = "???";
                                break;
                        }

                        break;
                }
            }

            if (Config::Instance()->FpsOverlayType.value_or_default() == 0)
                ImGui::Text("%s %5.1f fps %5.2f ms", api.c_str(), io.Framerate, 1000.0f / io.Framerate);
            else
                ImGui::Text("%s Fps: %5.1f, Avg: %5.1f", api.c_str(), io.Framerate, 1000.0f / averageFrameTime);

            if (Config::Instance()->FpsOverlayType.value_or_default() > 0)
            {
                if (Config::Instance()->FpsOverlayHorizontal.value_or_default())
                {
                    ImGui::SameLine(0.0f, 8.0f);
                    ImGui::Text(" | ");
                    ImGui::SameLine(0.0f, 8.0f);
                }
                else
                {
                    ImGui::Spacing();
                }

                ImGui::Text("Frame Time: %5.2f ms, Avg: %5.2f ms", State::Instance().frameTimes.back(), averageFrameTime);
            }

            if (Config::Instance()->FpsOverlayType.value_or_default() > 1)
            {
                ImVec2 plotSize(Config::Instance()->MenuScale.value() * 300, Config::Instance()->MenuScale.value() * 30);

                if (Config::Instance()->FpsOverlayHorizontal.value_or_default())
                {
                    ImGui::SameLine(0.0f, 8.0f);
                    plotSize.y = Config::Instance()->MenuScale.value() * 16;
                }

                // Graph of frame times
                ImGui::PlotLines("##FrameTimeGraph", frameTimeArray.data(), static_cast<int>(frameTimeArray.size()), 0, nullptr,
                                 *std::min_element(frameTimeArray.begin(), frameTimeArray.end()) * 0.9f, *std::max_element(frameTimeArray.begin(), frameTimeArray.end()) * 1.1f, plotSize);
            }

            if (Config::Instance()->FpsOverlayType.value_or_default() > 2)
            {
                if (Config::Instance()->FpsOverlayHorizontal.value_or_default())
                {
                    ImGui::SameLine(0.0f, 8.0f);
                    ImGui::Text(" | ");
                    ImGui::SameLine(0.0f, 8.0f);
                }
                else
                {
                    ImGui::Spacing();
                }

                ImGui::Text("Upscaler Time: %5.2f ms, Avg: %5.2f ms", State::Instance().upscaleTimes.back(), averageUpscalerFT);
            }

            if (Config::Instance()->FpsOverlayType.value_or_default() > 3)
            {
                ImVec2 plotSize(Config::Instance()->MenuScale.value() * 300, Config::Instance()->MenuScale.value() * 30);

                if (Config::Instance()->FpsOverlayHorizontal.value_or_default())
                {
                    ImGui::SameLine(0.0f, 8.0f);
                    plotSize.y = Config::Instance()->MenuScale.value() * 16;
                }

                // Graph of upscaler times
                ImGui::PlotLines("##UpscalerFrameTimeGraph", upscalerFrameTimeArray.data(), static_cast<int>(upscalerFrameTimeArray.size()), 0, nullptr,
                                 *std::min_element(upscalerFrameTimeArray.begin(), upscalerFrameTimeArray.end()) * 0.9f, *std::max_element(upscalerFrameTimeArray.begin(), upscalerFrameTimeArray.end()) * 1.1f, plotSize);
            }

            ImGui::PopStyleColor(3); // Restore the style
        }

        // Get size for postioning
        auto winSize = ImGui::GetWindowSize();

        ImGui::End();

        // Top left / Bottom left
        if (Config::Instance()->FpsOverlayPos.value_or_default() == 0 || Config::Instance()->FpsOverlayPos.value_or_default() == 2)
            overlayPosition.x = 0;
        else
            overlayPosition.x = io.DisplaySize.x - winSize.x;

        // Top Left / Top Right
        if (Config::Instance()->FpsOverlayPos.value_or_default() < 2)
            overlayPosition.y = 0;
        else
            overlayPosition.y = io.DisplaySize.y - winSize.y;

        if (!_isVisible)
        {
            ImGui::EndFrame();
            return true;
        }
    }

    if (!_isVisible)
        return false;

    {
        // If overlay is not visible frame needs to be inited
        if (!Config::Instance()->ShowFps.value_or_default())
        {
            ImGui_ImplWin32_NewFrame();
            MenuHdrCheck(io);
            MenuSizeCheck(io);
            ImGui::NewFrame();
        }

        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoSavedSettings;
        flags |= ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_AlwaysAutoResize;

        // if UI scale is changed rescale the style
        if (_imguiSizeUpdate)
        {
            _imguiSizeUpdate = false;

            ImGuiStyle& style = ImGui::GetStyle();
            ImGuiStyle styleold = style; // Backup colors
            style = ImGuiStyle(); // IMPORTANT: ScaleAllSizes will change the original size, so we should reset all style config
            style.WindowBorderSize = 1.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 1.0f;
            style.FrameBorderSize = 1.0f;
            style.TabBorderSize = 1.0f;
            style.WindowRounding = 0.0f;
            style.ChildRounding = 0.0f;
            style.PopupRounding = 0.0f;
            style.FrameRounding = 0.0f;
            style.ScrollbarRounding = 0.0f;
            style.GrabRounding = 0.0f;
            style.TabRounding = 0.0f;
            style.ScaleAllSizes(Config::Instance()->MenuScale.value_or_default());
            style.MouseCursorScale = 1.0f;
            CopyMemory(style.Colors, styleold.Colors, sizeof(style.Colors)); // Restore colors		
        }

        auto currentFeature = State::Instance().currentFeature;

        auto size = ImVec2{ 0.0f, 0.0f };
        ImGui::SetNextWindowSize(size);

        // Main menu window
        if (ImGui::Begin(VER_PRODUCT_NAME, NULL, flags))
        {
            bool rcasEnabled = false;

            if (!_showMipmapCalcWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                ImGui::SetWindowFocus();

            _selectedScale = ((int)(Config::Instance()->MenuScale.value() * 10.0f)) - 5;

            std::string selectedUpscalerName = "";
            std::string currentBackend = "";
            std::string currentBackendName = "";

            // No active upscaler message
            if (currentFeature == nullptr || !currentFeature->IsInited())
            {
                ImGui::Spacing();

                if (Config::Instance()->UseHQFont.value_or_default())
                    ImGui::PushFont(MenuBase::scaledFont);
                else
                    ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0) * 3.0);

                if (State::Instance().nvngxExists || State::Instance().libxessExists)
                {
                    ImGui::Spacing();

                    ImGui::Text("Please select %s%s%s%s%s as upscaler\nfrom game options and enter the game\nto enable upscaler settings.\n",
                                State::Instance().fsrHooks ? "FSR" : "",
                                State::Instance().fsrHooks && (State::Instance().nvngxExists || State::Instance().isRunningOnNvidia) ? " or " : "",
                                (State::Instance().nvngxExists || State::Instance().isRunningOnNvidia) ? "DLSS" : "",
                                ((State::Instance().nvngxExists || State::Instance().isRunningOnNvidia) || State::Instance().fsrHooks) && State::Instance().libxessExists ? " or " : "",
                                State::Instance().libxessExists ? "XeSS" : "");


                    if (Config::Instance()->UseHQFont.value_or_default())
                        ImGui::PopFont();
                    else
                        ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));

                    ImGui::Spacing();
                    ImGui::Text("nvngx.dll: %sExist", State::Instance().nvngxExists || State::Instance().isRunningOnNvidia ? "" : "Not ");
                    ImGui::Text("libxess.dll: %sExist", State::Instance().libxessExists ? "" : "Not ");
                    ImGui::Text("fsr: %sExist", State::Instance().fsrHooks ? "" : "Not ");
                    ImGui::Spacing();
                }
                else
                {
                    ImGui::Spacing();
                    ImGui::Text("Can't find nvngx.dll and libxess.dll and FSR inputs\nUpscaling support will NOT work.");
                    ImGui::Spacing();

                    if (Config::Instance()->UseHQFont.value_or_default())
                        ImGui::PopFont();
                    else
                        ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));
                }

            }

            if (ImGui::BeginTable("main", 2, ImGuiTableFlags_SizingStretchSame))
            {
                ImGui::TableNextColumn();

                if (currentFeature != nullptr)
                {
                    // UPSCALERS -----------------------------
                    ImGui::SeparatorText("Upscalers");
                    ShowTooltip("Which copium you choose?");

                    GetCurrentBackendInfo(State::Instance().api, &currentBackend, &currentBackendName);

                    switch (State::Instance().api)
                    {
                        case DX11:
                            ImGui::Text("DirectX 11 %s- %s (%d.%d.%d)", State::Instance().isRunningOnDXVK ? "(DXVK) " : "", State::Instance().currentFeature->Name(), State::Instance().currentFeature->Version().major, State::Instance().currentFeature->Version().minor, State::Instance().currentFeature->Version().patch);

                            if (State::Instance().currentFeature->Name() != "DLSSD")
                                AddDx11Backends(&currentBackend, &currentBackendName);

                            break;

                        case DX12:
                            ImGui::Text("DirectX 12 %s- %s (%d.%d.%d)", State::Instance().isRunningOnDXVK ? "(DXVK) " : "", State::Instance().currentFeature->Name(), State::Instance().currentFeature->Version().major, State::Instance().currentFeature->Version().minor, State::Instance().currentFeature->Version().patch);

                            ImGui::SameLine(0.0f, 6.0f);
                            ImGui::Text("Source Api: %s", State::Instance().currentInputApiName.c_str());

                            if (State::Instance().currentFeature->Name() != "DLSSD")
                                AddDx12Backends(&currentBackend, &currentBackendName);

                            break;

                        default:
                            ImGui::Text("Vulkan %s- %s (%d.%d.%d)", State::Instance().isRunningOnDXVK ? "(DXVK) " : "", State::Instance().currentFeature->Name(), State::Instance().currentFeature->Version().major, State::Instance().currentFeature->Version().minor, State::Instance().currentFeature->Version().patch);

                            ImGui::SameLine(0.0f, 6.0f);
                            ImGui::Text("Source Api: %s", State::Instance().currentInputApiName.c_str());

                            if (State::Instance().currentFeature->Name() != "DLSSD")
                                AddVulkanBackends(&currentBackend, &currentBackendName);
                    }

                    if (State::Instance().currentFeature->Name() != "DLSSD")
                    {
                        if (ImGui::Button("Apply") && State::Instance().newBackend != "" && State::Instance().newBackend != currentBackend)
                        {
                            if (State::Instance().newBackend == "xess")
                            {
                                // Reseting them for xess 
                                Config::Instance()->DisableReactiveMask.reset();
                                Config::Instance()->DlssReactiveMaskBias.reset();
                            }

                            State::Instance().changeBackend = true;
                        }

                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::Button("Revert"))
                            State::Instance().newBackend = "";
                    }
                }

                const char* fgOptions[] = { "No Frame Generation", "OptiFG", "FSR-FG via Nukem's DLSSG" };
                std::vector<std::string> fgDesc = { "", "", "Select DLSS FG in-game" };
                std::vector<uint8_t> disabledMask = { false, false, false };

                // OptiFG requirements
                if (!Config::Instance()->OverlayMenu.value_or_default())
                {
                    disabledMask[1] = true;
                    fgDesc[1] = "Old overlay menu is unsupported";
                }
                else if (State::Instance().api != DX12)
                {
                    disabledMask[1] = true;
                    fgDesc[1] = "Unsupported API";
                }
                else if (State::Instance().isWorkingAsNvngx)
                {
                    disabledMask[1] = true;
                    fgDesc[1] = "Unsupported Opti working mode";
                }
                else if (!FfxApiProxy::InitFfxDx12())
                {
                    disabledMask[1] = true;
                    fgDesc[1] = "amd_fidelityfx_dx12.dll is missing";
                }

                // Nukem's FG mod requirements
                if (State::Instance().api == DX11)
                {
                    disabledMask[2] = true;
                    fgDesc[2] = "Unsupported API";
                }
                else if (State::Instance().isWorkingAsNvngx)
                {
                    disabledMask[2] = true;
                    fgDesc[2] = "Unsupported Opti working mode";
                }
                else if (!State::Instance().NukemsFilesAvailable)
                {
                    disabledMask[2] = true;
                    fgDesc[2] = "Missing the dlssg_to_fsr3_amd_is_better.dll file";
                }

                auto disabledCount = std::ranges::count(disabledMask, 1);
                constexpr auto fgOptionsCount = sizeof(fgOptions) / sizeof(char*);

                if (!Config::Instance()->FGType.has_value())
                    Config::Instance()->FGType = Config::Instance()->FGType.value_or_default(); // need to have a value before combo

                if (disabledCount < fgOptionsCount - 1) // maybe always show it anyway?
                {
                    ImGui::SeparatorText("Frame Generation");
                    PopulateCombo("FG Type", reinterpret_cast<CustomOptional<uint32_t>*>(&Config::Instance()->FGType), fgOptions, fgDesc.data(), fgOptionsCount, disabledMask.data(), false);

                    if (State::Instance().showRestartWarning)
                    {
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(1.f, 0.f, 0.0f, 1.f), "Save INI and restart to apply the changes");
                        ImGui::Spacing();
                    }
                }

                State::Instance().showRestartWarning = State::Instance().activeFgType != Config::Instance()->FGType.value_or_default();

                // OptiFG
                if (Config::Instance()->OverlayMenu.value_or_default() && State::Instance().api == DX12 && !State::Instance().isWorkingAsNvngx && State::Instance().activeFgType == FGType::OptiFG)
                {
                    ImGui::SeparatorText("Frame Generation (OptiFG)");

                    if (currentFeature != nullptr && FfxApiProxy::InitFfxDx12())
                    {
                        bool fgActive = Config::Instance()->FGEnabled.value_or_default();
                        if (ImGui::Checkbox("FG Active", &fgActive))
                        {
                            Config::Instance()->FGEnabled = fgActive;
                            LOG_DEBUG("Enabled set FGEnabled: {}", fgActive);

                            if (Config::Instance()->FGEnabled.value_or_default())
                                State::Instance().FGchanged = true;
                        }

                        ShowHelpMarker("Enable frame generation (OptiFG)");

                        bool fgHudfix = Config::Instance()->FGHUDFix.value_or_default();
                        if (ImGui::Checkbox("FG HUD Fix", &fgHudfix))
                        {
                            Config::Instance()->FGHUDFix = fgHudfix;
                            LOG_DEBUG("Enabled set FGHUDFix: {}", fgHudfix);
                            State::Instance().FGchanged = true;
                        }
                        ShowHelpMarker("Enable HUD stability fix, might cause crashes!");

                        ImGui::BeginDisabled(!Config::Instance()->FGHUDFix.value_or_default());

                        ImGui::SameLine(0.0f, 16.0f);
                        ImGui::PushItemWidth(95.0 * Config::Instance()->MenuScale.value_or_default());
                        int hudFixLimit = Config::Instance()->FGHUDLimit.value_or_default();
                        if (ImGui::InputInt("Limit", &hudFixLimit))
                        {
                            if (hudFixLimit < 1)
                                hudFixLimit = 1;
                            else if (hudFixLimit > 999)
                                hudFixLimit = 999;

                            Config::Instance()->FGHUDLimit = hudFixLimit;
                            LOG_DEBUG("Enabled set FGHUDLimit: {}", hudFixLimit);
                        }
                        ShowHelpMarker("Delay HUDless capture, high values might cause crash!");

                        auto immediate = Config::Instance()->FGImmediateCapture.value_or_default();
                        if (ImGui::Checkbox("FG Immediate Capture", &immediate))
                        {
                            LOG_DEBUG("Enabled set FGImmediateCapture: {}", immediate);
                            Config::Instance()->FGImmediateCapture = immediate;
                        }
                        ShowHelpMarker("Enables capturing of resources before shader execution.\nIncrease hudless capture chances but might cause capturing of unnecessary resources.");

                        ImGui::SameLine(0.0f, 16.0f);
                        auto hudExtended = Config::Instance()->FGHUDFixExtended.value_or_default();
                        if (ImGui::Checkbox("FG Extended", &hudExtended))
                        {
                            LOG_DEBUG("Enabled set FGHUDFixExtended: {}", hudExtended);
                            Config::Instance()->FGHUDFixExtended = hudExtended;
                        }
                        ShowHelpMarker("Extended format checks for possible hudless\nMight cause crash and slowdowns!");

                        ImGui::PopItemWidth();

                        ImGui::EndDisabled();

                        bool fgAsync = Config::Instance()->FGAsync.value_or_default();

                        if (ImGui::Checkbox("FG Allow Async", &fgAsync))
                        {
                            Config::Instance()->FGAsync = fgAsync;

                            if (Config::Instance()->FGEnabled.value_or_default())
                            {
                                State::Instance().FGchanged = true;
                                LOG_DEBUG_ONLY("Async set FGChanged");
                            }
                        }
                        ShowHelpMarker("Enable Async for better FG performance\nMight cause crashes, especially with HUD Fix!");

                        ImGui::SameLine(0.0f, 16.0f);

                        bool fgDV = Config::Instance()->FGDebugView.value_or_default();
                        if (ImGui::Checkbox("FG Debug View", &fgDV))
                        {
                            Config::Instance()->FGDebugView = fgDV;

                            if (Config::Instance()->FGEnabled.value_or_default())
                            {
                                State::Instance().FGchanged = true;
                                LOG_DEBUG_ONLY("DebugView set FGChanged");
                            }
                        }
                        ShowHelpMarker("Enable FSR 3.1 frame generation debug view");

                        bool depthScale = Config::Instance()->FGEnableDepthScale.value_or_default();
                        if (ImGui::Checkbox("FG Scale Depth to fix DLSS RR", &depthScale))
                            Config::Instance()->FGEnableDepthScale = depthScale;
                        ShowHelpMarker("Fix for DLSS-D wrong depth inputs");

                        ImGui::Spacing();
                        if (ImGui::CollapsingHeader("Advanced OptiFG Settings"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            ImGui::Checkbox("FG Only Generated", &State::Instance().FGonlyGenerated);
                            ShowHelpMarker("Display only FSR 3.1 generated frames");

                            ImGui::BeginDisabled(State::Instance().FGresetCapturedResources);
                            ImGui::PushItemWidth(95.0 * Config::Instance()->MenuScale.value_or_default());
                            if (ImGui::Checkbox("FG Create List", &State::Instance().FGcaptureResources))
                            {
                                if (!State::Instance().FGcaptureResources)
                                    Config::Instance()->FGHUDLimit = 1;
                                else
                                    State::Instance().FGonlyUseCapturedResources = false;
                            }

                            ImGui::SameLine(0.0f, 16.0f);
                            if (ImGui::Checkbox("FG Use List", &State::Instance().FGonlyUseCapturedResources))
                            {
                                if (State::Instance().FGcaptureResources)
                                {
                                    State::Instance().FGcaptureResources = false;
                                    Config::Instance()->FGHUDLimit = 1;
                                }
                            }

                            ImGui::SameLine(0.0f, 8.0f);
                            ImGui::Text("(%d)", State::Instance().FGcapturedResourceCount);

                            ImGui::PopItemWidth();

                            ImGui::SameLine(0.0f, 16.0f);

                            if (ImGui::Button("Reset List"))
                            {
                                State::Instance().FGresetCapturedResources = true;
                                State::Instance().FGonlyUseCapturedResources = false;
                                State::Instance().FGonlyUseCapturedResources = false;
                            }

                            ImGui::EndDisabled();

                            ImGui::Spacing();
                            ImGui::Spacing();
                            if (ImGui::TreeNode("Tracking Settings"))
                            {
                                auto ath = Config::Instance()->FGAlwaysTrackHeaps.value_or_default();
                                if (ImGui::Checkbox("Always Track Heaps", &ath))
                                {
                                    Config::Instance()->FGAlwaysTrackHeaps = ath;
                                    LOG_DEBUG("Enabled set FGAlwaysTrackHeaps: {}", ath);
                                }
                                ShowHelpMarker("Always track resources, might cause performace issues\nbut also might fix HudFix related crashes!");

                                ImGui::SameLine(0.0f, 16.0f);
                                if (ImGui::Checkbox("Async Heap Tracking", &State::Instance().useThreadingForHeaps))
                                    LOG_DEBUG("Enabled set UseThreadingForHeaps: {}", State::Instance().useThreadingForHeaps);

                                ShowHelpMarker("Use async while tracking descriptor heap copy operations.\nMight cause locks or crashes!");

                                ImGui::TreePop();
                            }

                            ImGui::Spacing();
                            if (ImGui::TreeNode("Resource Settings"))
                            {
                                bool makeMVCopies = Config::Instance()->FGMakeMVCopy.value_or_default();
                                if (ImGui::Checkbox("FG Make MV Copies", &makeMVCopies))
                                    Config::Instance()->FGMakeMVCopy = makeMVCopies;
                                ShowHelpMarker("Make a copy of motion vectors to use with OptiFG\n"
                                               "For preventing corruptions that might happen");

                                bool makeDepthCopies = Config::Instance()->FGMakeDepthCopy.value_or_default();
                                if (ImGui::Checkbox("FG Make Depth Copies", &makeDepthCopies))
                                    Config::Instance()->FGMakeDepthCopy = makeDepthCopies;
                                ShowHelpMarker("Make a copy of depth to use with OptiFG\n"
                                               "For preventing corruptions that might happen");

                                ImGui::PushItemWidth(115.0 * Config::Instance()->MenuScale.value_or_default());
                                float depthScaleMax = Config::Instance()->FGDepthScaleMax.value_or_default();
                                if (ImGui::InputFloat("FG Scale Depth Max", &depthScaleMax, 10.0f, 100.0f, "%.1f"))
                                    Config::Instance()->FGDepthScaleMax = depthScaleMax;
                                ShowHelpMarker("Depth values will be divided to this value");
                                ImGui::PopItemWidth();

                                ImGui::TreePop();
                            }

                            ImGui::Spacing();
                            if (ImGui::TreeNode("Syncing Settings"))
                            {
                                bool useMutexForPresent = Config::Instance()->FGUseMutexForSwaphain.value_or_default();
                                if (ImGui::Checkbox("FG Use Mutex for Present", &useMutexForPresent))
                                    Config::Instance()->FGUseMutexForSwaphain = useMutexForPresent;
                                ShowHelpMarker("Use mutex to prevent desync of FG and crashes\n"
                                               "Disabling might improve the perf but decrase stability");

                                bool closeAfterCallback = Config::Instance()->FGHudFixCloseAfterCallback.value_or_default();
                                if (ImGui::Checkbox("FG Close CmdList After Callback", &closeAfterCallback))
                                    Config::Instance()->FGHudFixCloseAfterCallback = closeAfterCallback;
                                ShowHelpMarker("Close OptiFG CmdList after swapchain callback\n"
                                               "Mostly for debug purposes");

                                ImGui::TreePop();
                            }

                            ImGui::Spacing();
                            if (ImGui::TreeNode("FG Rectangle Settings"))
                            {
                                ImGui::PushItemWidth(95.0 * Config::Instance()->MenuScale.value_or_default());
                                int rectLeft = Config::Instance()->FGRectLeft.value_or(0);
                                if (ImGui::InputInt("Rect Left", &rectLeft))
                                    Config::Instance()->FGRectLeft = rectLeft;

                                ImGui::SameLine(0.0f, 16.0f);
                                int rectTop = Config::Instance()->FGRectTop.value_or(0);
                                if (ImGui::InputInt("Rect Top", &rectTop))
                                    Config::Instance()->FGRectTop = rectTop;

                                int rectWidth = Config::Instance()->FGRectWidth.value_or(0);
                                if (ImGui::InputInt("Rect Width", &rectWidth))
                                    Config::Instance()->FGRectWidth = rectWidth;

                                ImGui::SameLine(0.0f, 16.0f);
                                int rectHeight = Config::Instance()->FGRectHeight.value_or(0);
                                if (ImGui::InputInt("Rect Height", &rectHeight))
                                    Config::Instance()->FGRectHeight = rectHeight;

                                ImGui::PopItemWidth();
                                ShowHelpMarker("Frame generation rectangle, adjust for letterboxed content");

                                ImGui::BeginDisabled(!Config::Instance()->FGRectLeft.has_value() && !Config::Instance()->FGRectTop.has_value() &&
                                                     !Config::Instance()->FGRectWidth.has_value() && !Config::Instance()->FGRectHeight.has_value());

                                if (ImGui::Button("Reset FG Rect"))
                                {
                                    Config::Instance()->FGRectLeft.reset();
                                    Config::Instance()->FGRectTop.reset();
                                    Config::Instance()->FGRectWidth.reset();
                                    Config::Instance()->FGRectHeight.reset();
                                }

                                ShowHelpMarker("Resets frame generation rectangle");

                                ImGui::EndDisabled();
                                ImGui::TreePop();
                            }

                            if (isVersionOrBetter(FfxApiProxy::VersionDx12(), { 3, 1, 3 }))
                            {
                                ImGui::Spacing();
                                if (ImGui::TreeNode("Frame Pacing Tuning"))
                                {
                                    auto fptEnabled = Config::Instance()->FGFramePacingTuning.value_or_default();
                                    if (ImGui::Checkbox("Enable Tuning", &fptEnabled))
                                    {
                                        Config::Instance()->FGFramePacingTuning = fptEnabled;
                                        FrameGen_Dx12::ConfigureFramePaceTuning();
                                    }

                                    ImGui::BeginDisabled(!Config::Instance()->FGFramePacingTuning.value_or_default());

                                    ImGui::PushItemWidth(115.0 * Config::Instance()->MenuScale.value_or_default());
                                    auto fptSafetyMargin = Config::Instance()->FGFPTSafetyMarginInMs.value_or_default();
                                    if (ImGui::InputFloat("Safety Margins in ms", &fptSafetyMargin, 0.01, 0.1, "%.2f"))
                                        Config::Instance()->FGFPTSafetyMarginInMs = fptSafetyMargin;
                                    ShowHelpMarker("Safety margins in millisecons\n"
                                                   "FSR default value: 0.1ms\n"
                                                   "Opti default value: 0.01ms");

                                    auto fptVarianceFactor = Config::Instance()->FGFPTVarianceFactor.value_or_default();
                                    if (ImGui::SliderFloat("Variance Factor", &fptVarianceFactor, 0.0f, 1.0f, "%.2f"))
                                        Config::Instance()->FGFPTVarianceFactor = fptVarianceFactor;
                                    ShowHelpMarker("Variance factor\n"
                                                   "FSR default value: 0.1\n"
                                                   "Opti default value: 0.3");
                                    ImGui::PopItemWidth();

                                    auto fpHybridSpin = Config::Instance()->FGFPTAllowHybridSpin.value_or_default();
                                    if (ImGui::Checkbox("Enable Hybrid Spin", &fpHybridSpin))
                                        Config::Instance()->FGFPTAllowHybridSpin = fpHybridSpin;
                                    ShowHelpMarker("Allows pacing spinlock to sleep, should reduce CPU usage\n"
                                                   "Might cause slow ramp up of FPS");

                                    ImGui::PushItemWidth(115.0 * Config::Instance()->MenuScale.value_or_default());
                                    auto fptHybridSpinTime = Config::Instance()->FGFPTHybridSpinTime.value_or_default();
                                    if (ImGui::SliderInt("Hybrid Spin Time", &fptHybridSpinTime, 0, 100))
                                        Config::Instance()->FGFPTHybridSpinTime = fptHybridSpinTime;
                                    ShowHelpMarker("How long to spin if FPTHybridSpin is true. Measured in timer resolution units.\n"
                                                   "Not recommended to go below 2. Will result in frequent overshoots");
                                    ImGui::PopItemWidth();

                                    auto fpWaitForSingleObjectOnFence = Config::Instance()->FGFPTAllowWaitForSingleObjectOnFence.value_or_default();
                                    if (ImGui::Checkbox("Enable WaitForSingleObjectOnFence", &fpWaitForSingleObjectOnFence))
                                        Config::Instance()->FGFPTAllowWaitForSingleObjectOnFence = fpWaitForSingleObjectOnFence;
                                    ShowHelpMarker("Allows WaitForSingleObject instead of spinning for fence value");

                                    if (ImGui::Button("Apply Timing Changes"))
                                        FrameGen_Dx12::ConfigureFramePaceTuning();

                                    ImGui::EndDisabled();
                                    ImGui::TreePop();
                                }
                            }
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }
                    else if (currentFeature == nullptr)
                    {
                        ImGui::Text("Upscaler is not active"); // Probably never will be visible
                    }
                    else if (!FfxApiProxy::InitFfxDx12())
                    {
                        ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "amd_fidelityfx_dx12.dll is missing!"); // Probably never will be visible
                    }
                }

                // DLSSG Mod
                if (State::Instance().api != DX11 && !State::Instance().isWorkingAsNvngx && State::Instance().activeFgType == FGType::Nukems)
                {
                    SeparatorWithHelpMarker("Frame Generation (FSR-FG via Nukem's DLSSG)", "Requires Nukem's dlssg_to_fsr3 dll\nSelect DLSS FG in-game");

                    if (!State::Instance().NukemsFilesAvailable)
                        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Please put dlssg_to_fsr3_amd_is_better.dll next to OptiScaler");

                    if (!ReflexHooks::isReflexHooked()) {
                        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Reflex not hooked");
                        ImGui::Text("If you are using an AMD/Intel GPU then make sure you have fakenvapi");
                    }
                    else if (!ReflexHooks::isDlssgDetected()) {
                        ImGui::Text("Please select DLSS Frame Generation in the game options\nYou might need to select DLSS first");
                    }

                    if (State::Instance().api == DX12) {
                        ImGui::Text("Current DLSSG state:");
                        ImGui::SameLine();
                        if (ReflexHooks::isDlssgDetected())
                            ImGui::TextColored(ImVec4(0.f, 1.f, 0.25f, 1.f), "ON");
                        else
                            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "OFF");

                        if (bool makeDepthCopy = Config::Instance()->MakeDepthCopy.value_or_default(); ImGui::Checkbox("Fix broken visuals", &makeDepthCopy))
                            Config::Instance()->MakeDepthCopy = makeDepthCopy;
                        ShowHelpMarker("Makes a copy of the depth buffer\nCan fix broken visuals in some games on AMD GPUs under Windows\nCan cause stutters so best to use only when necessary");

                        if (DLSSGMod::isLoaded())
                        {
                            if (DLSSGMod::is120orNewer()) {
                                if (ImGui::Checkbox("Enable Debug View", &State::Instance().DLSSGDebugView)) {
                                    DLSSGMod::setDebugView(State::Instance().DLSSGDebugView);
                                }
                                if (ImGui::Checkbox("Interpolated frames only", &State::Instance().DLSSGInterpolatedOnly)) {
                                    DLSSGMod::setInterpolatedOnly(State::Instance().DLSSGInterpolatedOnly);
                                }
                            }
                            else if (DLSSGMod::FSRDebugView() != nullptr)
                            {
                                if (ImGui::Checkbox("Enable Debug View", &State::Instance().DLSSGDebugView)) {
                                    DLSSGMod::FSRDebugView()(State::Instance().DLSSGDebugView);
                                }
                            }
                        }

                    }
                }

                if (currentFeature != nullptr)
                {
                    // Dx11 with Dx12
                    if (State::Instance().api == DX11 &&
                        Config::Instance()->Dx11Upscaler.value_or_default() != "fsr22" && Config::Instance()->Dx11Upscaler.value_or_default() != "dlss" &&
                        Config::Instance()->Dx11Upscaler.value_or_default() != "fsr31")
                    {
                        ImGui::Spacing();
                        if (ImGui::CollapsingHeader("Dx11 with Dx12 Settings"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            const char* sync[] = { "No Syncing", "Fence", "Fence + Event", "Fence + Flush", "Fence + Flush + Event", "Only Query" };

                            const char* syncDesc[] = {
                                "Self explanatory",
                                "Sync using shared Fences(Signal& Wait). These should happen on GPU which is pretty fast.",
                                "Sync using shared Fences(Signal& Event). Events are waited on CPU which is slower.",
                                "Like Fence but flushes Dx11 DeviceContext after Signal shared Fence.",
                                "Like Fence + Event but flushes Dx11 DeviceContext after Signal shared Fence.",
                                "Uses Dx11 Query to sync, in general faster than Events but slower than Fences."
                            };

                            const char* selectedSync = sync[Config::Instance()->TextureSyncMethod.value_or_default()];

                            if (ImGui::BeginCombo("Input Sync", selectedSync))
                            {
                                for (int n = 0; n < 6; n++)
                                {
                                    if (ImGui::Selectable(sync[n], (Config::Instance()->TextureSyncMethod.value_or_default() == n)))
                                        Config::Instance()->TextureSyncMethod = n;
                                    ShowTooltip(syncDesc[n]);
                                }

                                ImGui::EndCombo();
                            }

                            const char* sync2[] = { "No Syncing", "Fence", "Fence + Event", "Fence + Flush", "Fence + Flush + Event", "Only Query" };

                            selectedSync = sync2[Config::Instance()->CopyBackSyncMethod.value_or_default()];

                            if (ImGui::BeginCombo("Output Sync", selectedSync))
                            {
                                for (int n = 0; n < 6; n++)
                                {
                                    if (ImGui::Selectable(sync2[n], (Config::Instance()->CopyBackSyncMethod.value_or_default() == n)))
                                        Config::Instance()->CopyBackSyncMethod = n;
                                    ShowTooltip(syncDesc[n]);
                                }

                                ImGui::EndCombo();
                            }

                            if (bool afterDx12 = Config::Instance()->SyncAfterDx12.value_or_default(); ImGui::Checkbox("Sync After Dx12", &afterDx12))
                                Config::Instance()->SyncAfterDx12 = afterDx12;
                            ShowHelpMarker("When using Events for syncing output SyncAfterDx12=false is usually more performant.");

                            ImGui::SameLine(0.0f, 6.0f);

                            if (bool dontUseNTShared = Config::Instance()->DontUseNTShared.value_or_default(); ImGui::Checkbox("Don't Use NTShared", &dontUseNTShared))
                                Config::Instance()->DontUseNTShared = dontUseNTShared;
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }


                    // UPSCALER SPECIFIC -----------------------------

                    // XeSS -----------------------------
                    if (currentBackend == "xess" && State::Instance().currentFeature->Name() != "DLSSD")
                    {
                        ImGui::Spacing();
                        if (ImGui::CollapsingHeader("XeSS Settings"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            const char* models[] = { "KPSS", "SPLAT", "MODEL_3", "MODEL_4", "MODEL_5", "MODEL_6" };
                            auto configModes = Config::Instance()->NetworkModel.value_or_default();

                            if (configModes < 0 || configModes > 5)
                                configModes = 0;

                            const char* selectedModel = models[configModes];

                            if (ImGui::BeginCombo("Network Models", selectedModel))
                            {
                                for (int n = 0; n < 6; n++)
                                {
                                    if (ImGui::Selectable(models[n], (Config::Instance()->NetworkModel.value_or_default() == n)))
                                    {
                                        Config::Instance()->NetworkModel = n;
                                        State::Instance().newBackend = currentBackend;
                                        State::Instance().changeBackend = true;
                                    }
                                }

                                ImGui::EndCombo();
                            }
                            ShowHelpMarker("Likely don't do much");

                            if (bool dbg = State::Instance().xessDebug; ImGui::Checkbox("Dump (Shift+Del)", &dbg))
                                State::Instance().xessDebug = dbg;

                            ImGui::SameLine(0.0f, 6.0f);
                            int dbgCount = State::Instance().xessDebugFrames;

                            ImGui::PushItemWidth(95.0 * Config::Instance()->MenuScale.value_or_default());
                            if (ImGui::InputInt("frames", &dbgCount))
                            {
                                if (dbgCount < 4)
                                    dbgCount = 4;
                                else if (dbgCount > 999)
                                    dbgCount = 999;

                                State::Instance().xessDebugFrames = dbgCount;
                            }

                            ImGui::PopItemWidth();
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }

                    // FSR -----------------
                    if (currentBackend.rfind("fsr", 0) == 0 && State::Instance().currentFeature->Name() != "DLSSD" &&
                        (currentBackend == "fsr31" || currentBackend == "fsr31_12" || State::Instance().currentFeature->AccessToReactiveMask()))
                    {
                        ImGui::SeparatorText("FSR Settings");

                        if (_fsr3xIndex < 0)
                            _fsr3xIndex = Config::Instance()->Fsr3xIndex.value_or_default();

                        if (currentBackend == "fsr31" || currentBackend == "fsr31_12" && State::Instance().fsr3xVersionNames.size() > 0)
                        {
                            auto currentName = std::format("FSR {}", State::Instance().fsr3xVersionNames[_fsr3xIndex]);
                            if (ImGui::BeginCombo("Upscaler", currentName.c_str()))
                            {
                                for (int n = 0; n < State::Instance().fsr3xVersionIds.size(); n++)
                                {
                                    auto name = std::format("FSR {}", State::Instance().fsr3xVersionNames[n]);
                                    if (ImGui::Selectable(name.c_str(), Config::Instance()->Fsr3xIndex.value_or_default() == n))
                                        _fsr3xIndex = n;
                                }

                                ImGui::EndCombo();
                            }
                            ShowHelpMarker("List of upscalers reported by FFX SDK");

                            if (ImGui::Button("Change Upscaler") && _fsr3xIndex != Config::Instance()->Fsr3xIndex.value_or_default())
                            {
                                Config::Instance()->Fsr3xIndex = _fsr3xIndex;
                                State::Instance().newBackend = currentBackend;
                                State::Instance().changeBackend = true;
                            }

                            if (currentFeature->Version().patch > 0)
                            {
                                float velocity = Config::Instance()->FsrVelocity.value_or_default();
                                if (ImGui::SliderFloat("Velocity Factor", &velocity, 0.00f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                                    Config::Instance()->FsrVelocity = velocity;

                                ShowHelpMarker("Lower values are more stable with ghosting\n"
                                               "Higher values are more pixelly but less ghosting.");
                            }
                        }

                        if (currentBackend == "fsr31" || currentBackend == "fsr31_12")
                        {
                            if (bool dView = Config::Instance()->FsrDebugView.value_or_default(); ImGui::Checkbox("FSR 3.X Debug View", &dView))
                                Config::Instance()->FsrDebugView = dView;
                            ShowHelpMarker("Top left: Dilated Motion Vectors\n"
                                           "Top middle: Protected Areas\n"
                                           "Top right: Dilated Depth\n"
                                           "Middle: Upscaled frame\n"
                                           "Bottom left: Disocclusion mask\n"
                                           "Bottom middle: Reactiveness\n"
                                           "Bottom right: Detail Protection Takedown");
                        }

                        if (State::Instance().currentFeature->AccessToReactiveMask())
                        {
                            ImGui::BeginDisabled(Config::Instance()->DisableReactiveMask.value_or(false));

                            auto useAsTransparency = Config::Instance()->FsrUseMaskForTransparency.value_or_default();
                            if (ImGui::Checkbox("Use Reactive Mask as Transparency Mask", &useAsTransparency))
                                Config::Instance()->FsrUseMaskForTransparency = useAsTransparency;

                            ImGui::EndDisabled();
                        }
                    }

                    // FSR Common -----------------
                    if (State::Instance().activeFgType == FGType::OptiFG || currentBackend.rfind("fsr", 0) == 0)
                    {
                        SeparatorWithHelpMarker("FSR Common Settings", "Affects both FSR-FG & Upscalers");

                        bool useFsrVales = Config::Instance()->FsrUseFsrInputValues.value_or_default();
                        if (ImGui::Checkbox("Use FSR Input Values", &useFsrVales))
                            Config::Instance()->FsrUseFsrInputValues = useFsrVales;

                        bool useVFov = Config::Instance()->FsrVerticalFov.has_value() || !Config::Instance()->FsrHorizontalFov.has_value();

                        float vfov = Config::Instance()->FsrVerticalFov.value_or_default();
                        float hfov = Config::Instance()->FsrHorizontalFov.value_or(90.0f);

                        if (useVFov && !Config::Instance()->FsrVerticalFov.has_value())
                            Config::Instance()->FsrVerticalFov = vfov;
                        else if (!useVFov && !Config::Instance()->FsrHorizontalFov.has_value())
                            Config::Instance()->FsrHorizontalFov = hfov;

                        if (ImGui::RadioButton("Use Vert. Fov", useVFov))
                        {
                            Config::Instance()->FsrHorizontalFov.reset();
                            Config::Instance()->FsrVerticalFov = vfov;
                            useVFov = true;
                        }

                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::RadioButton("Use Horz. Fov", !useVFov))
                        {
                            Config::Instance()->FsrVerticalFov.reset();
                            Config::Instance()->FsrHorizontalFov = hfov;
                            useVFov = false;
                        }

                        if (useVFov)
                        {
                            if (ImGui::SliderFloat("Vert. FOV", &vfov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                                Config::Instance()->FsrVerticalFov = vfov;

                            ShowHelpMarker("Might help achieve better image quality");
                        }
                        else
                        {
                            if (ImGui::SliderFloat("Horz. FOV", &hfov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                                Config::Instance()->FsrHorizontalFov = hfov;

                            ShowHelpMarker("Might help achieve better image quality");
                        }

                        float cameraNear;
                        float cameraFar;

                        cameraNear = Config::Instance()->FsrCameraNear.value_or_default();
                        cameraFar = Config::Instance()->FsrCameraFar.value_or_default();

                        if (ImGui::SliderFloat("Camera Near", &cameraNear, 0.1f, 500000.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrCameraNear = cameraNear;
                        ShowHelpMarker("Might help achieve better image quality\n"
                                       "And potentially less ghosting");

                        if (ImGui::SliderFloat("Camera Far", &cameraFar, 0.1f, 500000.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrCameraFar = cameraFar;
                        ShowHelpMarker("Might help achieve better image quality\n"
                                       "And potentially less ghosting");

                        if (ImGui::Button("Reset Camera Values"))
                        {
                            Config::Instance()->FsrVerticalFov.reset();
                            Config::Instance()->FsrHorizontalFov.reset();
                            Config::Instance()->FsrCameraNear.reset();
                            Config::Instance()->FsrCameraFar.reset();
                        }

                        ImGui::SameLine(0.0f, 6.0f);
                        ImGui::Text("Near: %.1f Far: %.1f",
                                    State::Instance().lastFsrCameraNear < 500000.0f ? State::Instance().lastFsrCameraNear : 500000.0f,
                                    State::Instance().lastFsrCameraFar < 500000.0f ? State::Instance().lastFsrCameraFar : 500000.0f);
                    }

                    // DLSS -----------------
                    if ((Config::Instance()->DLSSEnabled.value_or_default() && currentBackend == "dlss" && State::Instance().currentFeature->Version().major > 2) ||
                        State::Instance().currentFeature->Name() == "DLSSD")
                    {
                        const bool usesDlssd = State::Instance().currentFeature->Name() == "DLSSD";

                        if (usesDlssd)
                            ImGui::SeparatorText("DLSSD Settings");
                        else
                            ImGui::SeparatorText("DLSS Settings");

                        auto overridden = usesDlssd ? State::Instance().dlssdPresetsOverriddenExternally : State::Instance().dlssPresetsOverriddenExternally;

                        if (overridden) {
                            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Presets are overridden externally");
                            ShowHelpMarker("This usually happens due to using tools\n"
                                           "such as Nvidia App or Nvidia Inspector");
                            ImGui::Text("Selecting setting below will disable that external override\n"
                                        "but you need to Save INI and restart the game");

                            ImGui::Spacing();
                        }

                        if (bool pOverride = Config::Instance()->RenderPresetOverride.value_or_default(); ImGui::Checkbox("Render Presets Override", &pOverride))
                            Config::Instance()->RenderPresetOverride = pOverride;
                        ShowHelpMarker("Each render preset has it strengths and weaknesses\n"
                                       "Override to potentially improve image quality");

                        ImGui::BeginDisabled(!Config::Instance()->RenderPresetOverride.value_or_default() || overridden);

                        ImGui::PushItemWidth(135.0 * Config::Instance()->MenuScale.value_or_default());
                        if (usesDlssd)
                            AddDLSSDRenderPreset("Override Preset", &Config::Instance()->RenderPresetForAll);
                        else
                            AddDLSSRenderPreset("Override Preset", &Config::Instance()->RenderPresetForAll);

                        ImGui::PopItemWidth();

                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::Button("Apply Changes"))
                        {
                            if (usesDlssd)
                                State::Instance().newBackend = "dlssd";
                            else
                                State::Instance().newBackend = currentBackend;

                            State::Instance().changeBackend = true;
                        }

                        ImGui::EndDisabled();

                        ImGui::Spacing();

                        if (ImGui::CollapsingHeader(usesDlssd ? "Advanced DLSSD Settings" : "Advanced DLSS Settings"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            bool appIdOverride = Config::Instance()->UseGenericAppIdWithDlss.value_or_default();
                            if (ImGui::Checkbox("Use Generic App Id with DLSS", &appIdOverride))
                                Config::Instance()->UseGenericAppIdWithDlss = appIdOverride;

                            ShowHelpMarker("Use generic appid with NGX\n"
                                           "Fixes OptiScaler preset override not working with certain games\n"
                                           "Requires a game restart.");

                            ImGui::BeginDisabled(!Config::Instance()->RenderPresetOverride.value_or_default() || overridden);
                            ImGui::Spacing();
                            ImGui::PushItemWidth(135.0 * Config::Instance()->MenuScale.value_or_default());

                            if (usesDlssd)
                            {
                                AddDLSSDRenderPreset("DLAA Preset", &Config::Instance()->RenderPresetDLAA);
                                AddDLSSDRenderPreset("UltraQ Preset", &Config::Instance()->RenderPresetUltraQuality);
                                AddDLSSDRenderPreset("Quality Preset", &Config::Instance()->RenderPresetQuality);
                                AddDLSSDRenderPreset("Balanced Preset", &Config::Instance()->RenderPresetBalanced);
                                AddDLSSDRenderPreset("Perf Preset", &Config::Instance()->RenderPresetPerformance);
                                AddDLSSDRenderPreset("UltraP Preset", &Config::Instance()->RenderPresetUltraPerformance);
                            }
                            else
                            {
                                AddDLSSRenderPreset("DLAA Preset", &Config::Instance()->RenderPresetDLAA);
                                AddDLSSRenderPreset("UltraQ Preset", &Config::Instance()->RenderPresetUltraQuality);
                                AddDLSSRenderPreset("Quality Preset", &Config::Instance()->RenderPresetQuality);
                                AddDLSSRenderPreset("Balanced Preset", &Config::Instance()->RenderPresetBalanced);
                                AddDLSSRenderPreset("Perf Preset", &Config::Instance()->RenderPresetPerformance);
                                AddDLSSRenderPreset("UltraP Preset", &Config::Instance()->RenderPresetUltraPerformance);
                            }
                            ImGui::PopItemWidth();
                            ImGui::EndDisabled();
                        }

                        ImGui::Spacing();
                        ImGui::Spacing();
                    }

                    // RCAS -----------------
                    if (State::Instance().api == DX12 || State::Instance().api == DX11)
                    {
                        ImGui::SeparatorText("RCAS Settings");

                        // xess or dlss version >= 2.5.1
                        constexpr feature_version requiredDlssVersion = { 2, 5, 1 };
                        rcasEnabled = (currentBackend == "xess" || (currentBackend == "dlss" && isVersionOrBetter(State::Instance().currentFeature->Version(), requiredDlssVersion)));

                        if (bool rcas = Config::Instance()->RcasEnabled.value_or(rcasEnabled); ImGui::Checkbox("Enable RCAS", &rcas))
                            Config::Instance()->RcasEnabled = rcas;
                        ShowHelpMarker("A sharpening filter\n"
                                       "By default uses a sharpening value provided by the game\n"
                                       "Select 'Override' under 'Sharpness' and adjust the slider to change it\n\n"
                                       "Some upscalers have it's own sharpness filter so RCAS is not always needed");

                        ImGui::BeginDisabled(!Config::Instance()->RcasEnabled.value_or(rcasEnabled));

                        ImGui::Spacing();
                        if (ImGui::CollapsingHeader("Motion Adaptive Sharpness##2"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            if (bool overrideMotionSharpness = Config::Instance()->MotionSharpnessEnabled.value_or_default(); ImGui::Checkbox("Motion Adaptive Sharpness", &overrideMotionSharpness))
                                Config::Instance()->MotionSharpnessEnabled = overrideMotionSharpness;
                            ShowHelpMarker("Applies more sharpness to things in motion");

                            ImGui::BeginDisabled(!Config::Instance()->MotionSharpnessEnabled.value_or_default());

                            ImGui::SameLine(0.0f, 6.0f);

                            if (bool overrideMSDebug = Config::Instance()->MotionSharpnessDebug.value_or_default(); ImGui::Checkbox("MAS Debug", &overrideMSDebug))
                                Config::Instance()->MotionSharpnessDebug = overrideMSDebug;
                            ShowHelpMarker("Areas that are more red will have more sharpness applied\n"
                                           "Green areas will get reduced sharpness");

                            float motionSharpness = Config::Instance()->MotionSharpness.value_or_default();
                            ImGui::SliderFloat("MotionSharpness", &motionSharpness, -1.3f, 1.3f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                            Config::Instance()->MotionSharpness = motionSharpness;

                            float motionThreshod = Config::Instance()->MotionThreshold.value_or_default();
                            ImGui::SliderFloat("MotionThreshod", &motionThreshod, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                            Config::Instance()->MotionThreshold = motionThreshod;

                            float motionScale = Config::Instance()->MotionScaleLimit.value_or_default();
                            ImGui::SliderFloat("MotionRange", &motionScale, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                            Config::Instance()->MotionScaleLimit = motionScale;

                            ImGui::EndDisabled();
                        }
                        ImGui::Spacing();
                        ImGui::Spacing();
                        ImGui::EndDisabled();
                    }

                    // DLSS Enabler -----------------
                    if (State::Instance().enablerAvailable)
                    {
                        ImGui::SeparatorText("DLSS Enabler");

                        ImGui::BeginDisabled(Config::Instance()->DE_FramerateLimitVsync.value_or(false));

                        // set inital value
                        if (Config::Instance()->DE_FramerateLimit.has_value() && _deLimitFps > 200)
                            _deLimitFps = Config::Instance()->DE_FramerateLimit.value();

                        ImGui::SliderInt("FPS Limit", &_deLimitFps, 0, 200);

                        if (ImGui::Button("Apply Limit"))
                            Config::Instance()->DE_FramerateLimit = _deLimitFps;

                        ImGui::EndDisabled();

                        ImGui::SameLine(0.0f, 6.0f);

                        bool fpsLimitVsync = Config::Instance()->DE_FramerateLimitVsync.value_or(false);
                        if (ImGui::Checkbox("VSync", &fpsLimitVsync))
                            Config::Instance()->DE_FramerateLimitVsync = fpsLimitVsync;
                        ShowHelpMarker("Limit FPS to your monitor's refresh rate");

                        if (Config::Instance()->DE_DynamicLimitAvailable.has_value() && Config::Instance()->DE_DynamicLimitAvailable.value() > 0)
                        {
                            ImGui::SameLine(0.0f, 6.0f);

                            bool dfgEnabled = Config::Instance()->DE_DynamicLimitEnabled.value_or(false);
                            if (ImGui::Checkbox("Dynamic Frame Gen.", &dfgEnabled))
                                Config::Instance()->DE_DynamicLimitEnabled = dfgEnabled;
                            ShowHelpMarker("Enables FG only when FPS would go below FPS Limit.\n"
                                           "This will result in higher input latency in situation of low fps\n"
                                           "but motion smoothness will be preserved.");
                        }

                        if (ImGui::CollapsingHeader("Advanced Enabler Settings"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            std::string selected;

                            if (Config::Instance()->DE_Generator.value_or("auto") == "auto")
                                selected = "Auto";
                            else if (Config::Instance()->DE_Generator.value_or("auto") == "fsr30")
                                selected = "FSR3.0";
                            else if (Config::Instance()->DE_Generator.value_or("auto") == "fsr31")
                                selected = "FSR3.1";
                            else if (Config::Instance()->DE_Generator.value_or("auto") == "dlssg")
                                selected = "DLSS-G";

                            if (ImGui::BeginCombo("Generator", selected.c_str()))
                            {
                                if (ImGui::Selectable("Auto", Config::Instance()->DE_Generator.value_or("auto") == "auto"))
                                    Config::Instance()->DE_Generator = "auto";

                                if (ImGui::Selectable("FSR3.0", Config::Instance()->DE_Generator.value_or("auto") == "fsr30"))
                                    Config::Instance()->DE_Generator = "fsr30";

                                if (ImGui::Selectable("FSR3.1", Config::Instance()->DE_Generator.value_or("auto") == "fsr31"))
                                    Config::Instance()->DE_Generator = "fsr31";

                                if (ImGui::Selectable("DLSS-G", Config::Instance()->DE_Generator.value_or("auto") == "dlssg"))
                                    Config::Instance()->DE_Generator = "dlssg";

                                ImGui::EndCombo();
                            }
                            ShowHelpMarker("Frame Generation algorithm to be used");

                            if (Config::Instance()->DE_Reflex.value_or("on") == "on")
                                selected = "On";
                            else if (Config::Instance()->DE_Reflex.value_or("on") == "boost")
                                selected = "Boost";
                            else if (Config::Instance()->DE_Reflex.value_or("on") == "off")
                                selected = "Off";

                            if (ImGui::BeginCombo("Reflex", selected.c_str()))
                            {
                                if (ImGui::Selectable("On", Config::Instance()->DE_Reflex.value_or("on") == "on"))
                                    Config::Instance()->DE_Reflex = "on";

                                if (ImGui::Selectable("Boost", Config::Instance()->DE_Reflex.value_or("on") == "boost"))
                                    Config::Instance()->DE_Reflex = "boost";

                                if (ImGui::Selectable("Off", Config::Instance()->DE_Reflex.value_or("on") == "off"))
                                    Config::Instance()->DE_Reflex = "off";

                                ImGui::EndCombo();
                            }

                            if (Config::Instance()->DE_ReflexEmu.value_or("auto") == "auto")
                                selected = "Auto";
                            else if (Config::Instance()->DE_ReflexEmu.value_or("auto") == "on")
                                selected = "On";
                            else if (Config::Instance()->DE_ReflexEmu.value_or("auto") == "off")
                                selected = "Off";

                            if (ImGui::BeginCombo("Reflex Emu", selected.c_str()))
                            {
                                if (ImGui::Selectable("Auto", Config::Instance()->DE_ReflexEmu.value_or("auto") == "auto"))
                                    Config::Instance()->DE_ReflexEmu = "auto";

                                if (ImGui::Selectable("On", Config::Instance()->DE_ReflexEmu.value_or("auto") == "on"))
                                    Config::Instance()->DE_ReflexEmu = "on";

                                if (ImGui::Selectable("Off", Config::Instance()->DE_ReflexEmu.value_or("auto") == "off"))
                                    Config::Instance()->DE_ReflexEmu = "off";

                                ImGui::EndCombo();
                            }
                            ShowHelpMarker("Doesn't reduce the input latency\nbut may stabilize the frame rate");
                        }
                    }
                }

                // Framerate ---------------------
                if (!State::Instance().enablerAvailable && (State::Instance().reflexLimitsFps || Config::Instance()->OverlayMenu))
                {
                    SeparatorWithHelpMarker("Framerate", "Uses Reflex when possible\non AMD/Intel cards you can use fakenvapi to substitute Reflex");

                    ImGui::Text(std::format("Current method: {}", State::Instance().reflexLimitsFps ? "Reflex" : "Fallback").c_str());

                    if (State::Instance().reflexShowWarning)
                    {
                        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Using Reflex's limit with OptiFG has performance overhead");

                        ImGui::Spacing();
                    }

                    // set initial value
                    if (_limitFps == INFINITY)
                        _limitFps = Config::Instance()->FramerateLimit.value_or_default();

                    ImGui::SliderFloat("FPS Limit", &_limitFps, 0, 200, "%.0f");

                    if (ImGui::Button("Apply Limit")) {
                        Config::Instance()->FramerateLimit = _limitFps;
                    }
                }

                if (currentFeature != nullptr) {
                    // OUTPUT SCALING -----------------------------
                    if (State::Instance().api == DX12 || State::Instance().api == DX11)
                    {
                        // if motion vectors are not display size
                        ImGui::BeginDisabled((Config::Instance()->DisplayResolution.has_value() && Config::Instance()->DisplayResolution.value()) ||
                                             (!Config::Instance()->DisplayResolution.has_value() && !(State::Instance().currentFeature->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes)));

                        ImGui::SeparatorText("Output Scaling");

                        float defaultRatio = 1.5f;

                        if (_ssRatio == 0.0f)
                        {
                            _ssRatio = Config::Instance()->OutputScalingMultiplier.value_or(defaultRatio);
                            _ssEnabled = Config::Instance()->OutputScalingEnabled.value_or_default();
                            _ssUseFsr = Config::Instance()->OutputScalingUseFsr.value_or_default();
                            _ssDownsampler = Config::Instance()->OutputScalingDownscaler.value_or_default();
                        }

                        ImGui::BeginDisabled((currentBackend == "xess" || currentBackend == "dlss") &&
                                             State::Instance().currentFeature->RenderWidth() > State::Instance().currentFeature->DisplayWidth());
                        ImGui::Checkbox("Enable", &_ssEnabled);
                        ImGui::EndDisabled();

                        ShowHelpMarker("Upscales the image internally to a selected resolution\n"
                                       "Then scales it to your resolution\n\n"
                                       "Values <1.0 might make upscaler less expensive\n"
                                       "Values >1.0 might make image sharper at the cost of performance\n\n"
                                       "You can see each step at the bottom of this menu");

                        ImGui::SameLine(0.0f, 6.0f);

                        ImGui::BeginDisabled(!_ssEnabled);
                        {
                            ImGui::Checkbox("Use FSR 1", &_ssUseFsr);
                            ShowHelpMarker("Use FSR 1 for scaling");

                            ImGui::SameLine(0.0f, 6.0f);

                            ImGui::BeginDisabled(_ssUseFsr || _ssRatio < 1.0f);
                            {
                                const char* ds_modes[] = { "Bicubic", "Lanczos", "Catmull-Rom", "MAGC" };
                                const std::string ds_modesDesc[] = { "", "", "", "" };

                                ImGui::PushItemWidth(75.0f * Config::Instance()->MenuScale.value());
                                PopulateCombo("Downscaler", &Config::Instance()->OutputScalingDownscaler, ds_modes, ds_modesDesc, 4);
                                ImGui::PopItemWidth();
                            }
                            ImGui::EndDisabled();
                        }
                        ImGui::EndDisabled();

                        bool applyEnabled = _ssEnabled != Config::Instance()->OutputScalingEnabled.value_or_default() ||
                            _ssRatio != Config::Instance()->OutputScalingMultiplier.value_or(defaultRatio) ||
                            _ssUseFsr != Config::Instance()->OutputScalingUseFsr.value_or_default() ||
                            (_ssRatio > 1.0f && _ssDownsampler != Config::Instance()->OutputScalingDownscaler.value_or_default());

                        ImGui::BeginDisabled(!applyEnabled);
                        if (ImGui::Button("Apply Change"))
                        {
                            Config::Instance()->OutputScalingEnabled = _ssEnabled;
                            Config::Instance()->OutputScalingMultiplier = _ssRatio;
                            Config::Instance()->OutputScalingUseFsr = _ssUseFsr;
                            _ssDownsampler = Config::Instance()->OutputScalingDownscaler.value_or_default();

                            if (State::Instance().currentFeature->Name() == "DLSSD")
                                State::Instance().newBackend = "dlssd";
                            else
                                State::Instance().newBackend = currentBackend;


                            State::Instance().changeBackend = true;
                        }
                        ImGui::EndDisabled();

                        ImGui::BeginDisabled(!_ssEnabled || State::Instance().currentFeature->RenderWidth() > State::Instance().currentFeature->DisplayWidth());
                        ImGui::SliderFloat("Ratio", &_ssRatio, 0.5f, 3.0f, "%.2f");
                        ImGui::EndDisabled();

                        if (currentFeature != nullptr)
                        {
                            ImGui::Text("Output Scaling is %s, target res: %dx%d", Config::Instance()->OutputScalingEnabled.value_or_default() ? "ENABLED" : "DISABLED",
                                        (uint32_t)(currentFeature->DisplayWidth() * _ssRatio), (uint32_t)(currentFeature->DisplayHeight() * _ssRatio));
                        }

                        ImGui::EndDisabled();
                    }
                }

                // NEXT COLUMN -----------------
                ImGui::TableNextColumn();

                if (currentFeature != nullptr)
                {
                    // SHARPNESS -----------------------------
                    ImGui::SeparatorText("Sharpness");

                    if (bool overrideSharpness = Config::Instance()->OverrideSharpness.value_or_default(); ImGui::Checkbox("Override", &overrideSharpness))
                    {
                        Config::Instance()->OverrideSharpness = overrideSharpness;

                        if (currentBackend == "dlss" && State::Instance().currentFeature->Version().major < 3)
                        {
                            State::Instance().newBackend = currentBackend;
                            State::Instance().changeBackend = true;
                        }
                    }
                    ShowHelpMarker("Ignores the value sent by the game\n"
                                   "and uses the value set below");

                    ImGui::BeginDisabled(!Config::Instance()->OverrideSharpness.value_or_default());

                    float sharpness = Config::Instance()->Sharpness.value_or_default();
                    ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, Config::Instance()->RcasEnabled.value_or(rcasEnabled) ? 1.3f : 1.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                    Config::Instance()->Sharpness = sharpness;

                    ImGui::EndDisabled();

                    // UPSCALE RATIO OVERRIDE -----------------

                    auto minSliderLimit = Config::Instance()->ExtendedLimits.value_or_default() ? 0.1f : 1.0f;
                    auto maxSliderLimit = Config::Instance()->ExtendedLimits.value_or_default() ? 6.0f : 3.0f;

                    ImGui::SeparatorText("Upscale Ratio");
                    if (bool upOverride = Config::Instance()->UpscaleRatioOverrideEnabled.value_or_default(); ImGui::Checkbox("Ratio Override", &upOverride))
                        Config::Instance()->UpscaleRatioOverrideEnabled = upOverride;
                    ShowHelpMarker("Let's you override every upscaler preset\n"
                                   "with a value set below\n\n"
                                   "1.5x on a 1080p screen means internal resolution of 720p\n"
                                   "1080 / 1.5 = 720");

                    ImGui::BeginDisabled(!Config::Instance()->UpscaleRatioOverrideEnabled.value_or_default());

                    float urOverride = Config::Instance()->UpscaleRatioOverrideValue.value_or_default();
                    ImGui::SliderFloat("Override Ratio", &urOverride, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                    Config::Instance()->UpscaleRatioOverrideValue = urOverride;

                    ImGui::EndDisabled();

                    // QUALITY OVERRIDES -----------------------------
                    ImGui::SeparatorText("Quality Overrides");

                    if (bool qOverride = Config::Instance()->QualityRatioOverrideEnabled.value_or_default(); ImGui::Checkbox("Quality Override", &qOverride))
                        Config::Instance()->QualityRatioOverrideEnabled = qOverride;
                    ShowHelpMarker("Let's you override each preset's ratio individually\n"
                                   "Note that not every game supports every quality preset\n\n"
                                   "1.5x on a 1080p screen means internal resolution of 720p\n"
                                   "1080 / 1.5 = 720");

                    ImGui::BeginDisabled(!Config::Instance()->QualityRatioOverrideEnabled.value_or_default());

                    float qDlaa = Config::Instance()->QualityRatio_DLAA.value_or_default();
                    if (ImGui::SliderFloat("DLAA", &qDlaa, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_DLAA = qDlaa;

                    float qUq = Config::Instance()->QualityRatio_UltraQuality.value_or_default();
                    if (ImGui::SliderFloat("Ultra Quality", &qUq, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_UltraQuality = qUq;

                    float qQ = Config::Instance()->QualityRatio_Quality.value_or_default();
                    if (ImGui::SliderFloat("Quality", &qQ, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Quality = qQ;

                    float qB = Config::Instance()->QualityRatio_Balanced.value_or_default();
                    if (ImGui::SliderFloat("Balanced", &qB, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Balanced = qB;

                    float qP = Config::Instance()->QualityRatio_Performance.value_or_default();
                    if (ImGui::SliderFloat("Performance", &qP, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Performance = qP;

                    float qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or_default();
                    if (ImGui::SliderFloat("Ultra Performance", &qUp, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_UltraPerformance = qUp;

                    ImGui::EndDisabled();

                    // DRS -----------------------------
                    ImGui::SeparatorText("DRS (Dynamic Resolution Scaling)");
                    if (ImGui::BeginTable("drs", 2, ImGuiTableFlags_SizingStretchSame))
                    {
                        ImGui::TableNextColumn();
                        if (bool drsMin = Config::Instance()->DrsMinOverrideEnabled.value_or_default(); ImGui::Checkbox("Override Minimum", &drsMin))
                            Config::Instance()->DrsMinOverrideEnabled = drsMin;
                        ShowHelpMarker("Fix for games ignoring official DRS limits");

                        ImGui::TableNextColumn();
                        if (bool drsMax = Config::Instance()->DrsMaxOverrideEnabled.value_or_default(); ImGui::Checkbox("Override Maximum", &drsMax))
                            Config::Instance()->DrsMaxOverrideEnabled = drsMax;
                        ShowHelpMarker("Fix for games ignoring official DRS limits");

                        ImGui::EndTable();
                    }

                    // INIT -----------------------------
                    ImGui::SeparatorText("Init Flags");
                    if (ImGui::BeginTable("init", 2, ImGuiTableFlags_SizingStretchSame))
                    {
                        ImGui::TableNextColumn();
                        if (bool autoExposure = Config::Instance()->AutoExposure.value_or(false); ImGui::Checkbox("Auto Exposure", &autoExposure))
                        {
                            Config::Instance()->AutoExposure = autoExposure;

                            if (State::Instance().currentFeature->Name() == "DLSSD")
                                State::Instance().newBackend = "dlssd";
                            else
                                State::Instance().newBackend = currentBackend;

                            State::Instance().changeBackend = true;
                        }
                        ShowHelpMarker("Some Unreal Engine games need this\n"
                                       "Might fix colors, especially in dark areas");

                        ImGui::TableNextColumn();
                        if (bool hdr = Config::Instance()->HDR.value_or(false); ImGui::Checkbox("HDR", &hdr))
                        {
                            Config::Instance()->HDR = hdr;

                            if (State::Instance().currentFeature->Name() == "DLSSD")
                                State::Instance().newBackend = "dlssd";
                            else
                                State::Instance().newBackend = currentBackend;

                            State::Instance().changeBackend = true;
                        }
                        ShowHelpMarker("Might help with purple hue in some games");

                        ImGui::EndTable();

                        ImGui::Spacing();
                        if (ImGui::CollapsingHeader("Advanced Init Flags"))
                        {
                            ScopedIndent indent{};
                            ImGui::Spacing();
                            if (ImGui::BeginTable("init2", 2, ImGuiTableFlags_SizingStretchSame))
                            {
                                ImGui::TableNextColumn();
                                if (bool depth = Config::Instance()->DepthInverted.value_or(false); ImGui::Checkbox("Depth Inverted", &depth))
                                {
                                    Config::Instance()->DepthInverted = depth;

                                    if (State::Instance().currentFeature->Name() == "DLSSD")
                                        State::Instance().newBackend = "dlssd";
                                    else
                                        State::Instance().newBackend = currentBackend;

                                    State::Instance().changeBackend = true;
                                }
                                ShowHelpMarker("You shouldn't need to change it");

                                ImGui::TableNextColumn();
                                if (bool jitter = Config::Instance()->JitterCancellation.value_or(false); ImGui::Checkbox("Jitter Cancellation", &jitter))
                                {
                                    Config::Instance()->JitterCancellation = jitter;

                                    if (State::Instance().currentFeature->Name() == "DLSSD")
                                        State::Instance().newBackend = "dlssd";
                                    else
                                        State::Instance().newBackend = currentBackend;

                                    State::Instance().changeBackend = true;
                                }
                                ShowHelpMarker("Fix for games that send motion data with preapplied jitter");

                                ImGui::TableNextColumn();
                                if (bool mv = Config::Instance()->DisplayResolution.value_or(false); ImGui::Checkbox("Display Res. MV", &mv))
                                {
                                    Config::Instance()->DisplayResolution = mv;

                                    if (mv)
                                    {
                                        Config::Instance()->OutputScalingEnabled = false;
                                        _ssEnabled = false;
                                    }

                                    if (State::Instance().currentFeature->Name() == "DLSSD")
                                        State::Instance().newBackend = "dlssd";
                                    else
                                        State::Instance().newBackend = currentBackend;

                                    State::Instance().changeBackend = true;
                                }
                                ShowHelpMarker("Mostly a fix for Unreal Engine games\n"
                                               "Top left part of the screen will be blurry");

                                ImGui::TableNextColumn();
                                auto accessToReactiveMask = State::Instance().currentFeature->AccessToReactiveMask();
                                ImGui::BeginDisabled(!accessToReactiveMask);

                                bool rm = Config::Instance()->DisableReactiveMask.value_or(!accessToReactiveMask || currentBackend == "dlss" || currentBackend == "xess");
                                if (ImGui::Checkbox("Disable Reactive Mask", &rm))
                                {
                                    Config::Instance()->DisableReactiveMask = rm;

                                    if (currentBackend == "xess")
                                    {
                                        State::Instance().newBackend = currentBackend;
                                        State::Instance().changeBackend = true;
                                    }
                                }

                                ImGui::EndDisabled();

                                if (accessToReactiveMask)
                                    ShowHelpMarker("Allows the use of a reactive mask\n"
                                                   "Keep in mind that a reactive mask sent to DLSS\n"
                                                   "will not produce a good image in combination with FSR/XeSS");
                                else
                                    ShowHelpMarker("Option disabled because tha game doesn't provide a reactive mask");

                                ImGui::TableNextColumn();
                                ImGui::EndTable();
                            }

                            if (State::Instance().currentFeature->AccessToReactiveMask() && currentBackend != "dlss")
                            {
                                ImGui::BeginDisabled(Config::Instance()->DisableReactiveMask.value_or(currentBackend == "xess"));

                                bool binaryMask = State::Instance().api == Vulkan || currentBackend == "xess";
                                auto bias = Config::Instance()->DlssReactiveMaskBias.value_or(binaryMask ? 0.0f : 0.45f);

                                if (!binaryMask)
                                {
                                    if (ImGui::SliderFloat("React. Mask Bias", &bias, 0.0f, 0.9f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                                        Config::Instance()->DlssReactiveMaskBias = bias;

                                    ShowHelpMarker("Values above 0 activates usage of reactive mask");
                                }
                                else
                                {
                                    bool useRM = bias > 0.0f;
                                    if (ImGui::Checkbox("Use Binary Reactive Mask", &useRM))
                                    {
                                        if (useRM)
                                            Config::Instance()->DlssReactiveMaskBias = 0.45f;
                                        else
                                            Config::Instance()->DlssReactiveMaskBias.reset();
                                    }
                                }

                                ImGui::EndDisabled();
                            }
                        }
                    }
                }

                // FAKENVAPI ---------------------------
                if (fakenvapi::isUsingFakenvapi())
                {
                    ImGui::SeparatorText("fakenvapi");

                    if (bool logs = Config::Instance()->FN_EnableLogs.value_or_default(); ImGui::Checkbox("Enable Logs", &logs))
                        Config::Instance()->FN_EnableLogs = logs;

                    ImGui::SameLine(0.0f, 6.0f);
                    if (bool traceLogs = Config::Instance()->FN_EnableTraceLogs.value_or_default(); ImGui::Checkbox("Enable Trace Logs", &traceLogs))
                        Config::Instance()->FN_EnableTraceLogs = traceLogs;

                    if (bool forceLFX = Config::Instance()->FN_ForceLatencyFlex.value_or_default(); ImGui::Checkbox("Force LatencyFlex", &forceLFX))
                        Config::Instance()->FN_ForceLatencyFlex = forceLFX;
                    ShowHelpMarker("When possible AntiLag 2 is used, this setting let's you force LatencyFlex instead");

                    const char* lfx_modes[] = { "Conservative", "Aggressive", "Reflex ID" };
                    const std::string lfx_modesDesc[] = { "The safest but might not reduce latency well",
                        "Improves latency but in some cases will lower fps more than expected",
                        "Best when can be used, some games are not compatible (i.e. cyberpunk) and will fallback to aggressive"
                    };

                    PopulateCombo("LatencyFlex mode", &Config::Instance()->FN_LatencyFlexMode, lfx_modes, lfx_modesDesc, 3);

                    const char* rfx_modes[] = { "Follow in-game", "Force Disable", "Force Enable" };
                    const std::string rfx_modesDesc[] = { "", "", "" };

                    PopulateCombo("Force Reflex", &Config::Instance()->FN_ForceReflex, rfx_modes, rfx_modesDesc, 3);

                    if (ImGui::Button("Apply##2"))
                    {
                        Config::Instance()->SaveFakenvapiIni();
                    }
                }

                ImGui::Spacing();
                if (ImGui::CollapsingHeader("Advanced Settings"))
                {
                    ScopedIndent indent{};
                    ImGui::Spacing();
                    if (currentFeature != nullptr)
                    {
                        bool extendedLimits = Config::Instance()->ExtendedLimits.value_or_default();
                        if (ImGui::Checkbox("Enable Extended Limits", &extendedLimits))
                            Config::Instance()->ExtendedLimits = extendedLimits;

                        ShowHelpMarker("Extended sliders limit for quality presets\n\n"
                                       "Using this option changes resolution detection logic\n"
                                       "and might cause issues and crashes!");
                    }

                    bool pcShaders = Config::Instance()->UsePrecompiledShaders.value_or_default();
                    if (ImGui::Checkbox("Use Precompiled Shaders", &pcShaders))
                    {
                        Config::Instance()->UsePrecompiledShaders = pcShaders;
                        State::Instance().newBackend = currentBackend;
                        State::Instance().changeBackend = true;
                    }
                }

                // LOGGING -----------------------------
                ImGui::Spacing();
                if (ImGui::CollapsingHeader("Logging"))
                {
                    ScopedIndent indent{};
                    ImGui::Spacing();
                    if (Config::Instance()->LogToConsole.value_or_default() || Config::Instance()->LogToFile.value_or_default() || Config::Instance()->LogToNGX.value_or_default())
                        spdlog::default_logger()->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or_default());
                    else
                        spdlog::default_logger()->set_level(spdlog::level::off);

                    if (bool toFile = Config::Instance()->LogToFile.value_or_default(); ImGui::Checkbox("To File", &toFile))
                    {
                        Config::Instance()->LogToFile = toFile;
                        PrepareLogger();
                    }

                    ImGui::SameLine(0.0f, 6.0f);
                    if (bool toConsole = Config::Instance()->LogToConsole.value_or_default(); ImGui::Checkbox("To Console", &toConsole))
                    {
                        Config::Instance()->LogToConsole = toConsole;
                        PrepareLogger();
                    }

                    const char* logLevels[] = { "Trace", "Debug", "Information", "Warning", "Error" };
                    const char* selectedLevel = logLevels[Config::Instance()->LogLevel.value_or_default()];

                    if (ImGui::BeginCombo("Log Level", selectedLevel))
                    {
                        for (int n = 0; n < 5; n++)
                        {
                            if (ImGui::Selectable(logLevels[n], (Config::Instance()->LogLevel.value_or_default() == n)))
                            {
                                Config::Instance()->LogLevel = n;
                                spdlog::default_logger()->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or_default());
                            }
                        }

                        ImGui::EndCombo();
                    }
                }

                // FPS OVERLAY -----------------------------
                ImGui::Spacing();
                if (ImGui::CollapsingHeader("FPS Overlay"))
                {
                    ScopedIndent indent{};
                    ImGui::Spacing();
                    bool fpsEnabled = Config::Instance()->ShowFps.value_or_default();
                    if (ImGui::Checkbox("FPS Overlay Enabled", &fpsEnabled))
                        Config::Instance()->ShowFps = fpsEnabled;

                    ImGui::SameLine(0.0f, 6.0f);

                    bool fpsHorizontal = Config::Instance()->FpsOverlayHorizontal.value_or_default();
                    if (ImGui::Checkbox("Horizontal", &fpsHorizontal))
                        Config::Instance()->FpsOverlayHorizontal = fpsHorizontal;

                    const char* fpsPosition[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
                    const char* selectedPosition = fpsPosition[Config::Instance()->FpsOverlayPos.value_or_default()];

                    if (ImGui::BeginCombo("Overlay Position", selectedPosition))
                    {
                        for (int n = 0; n < 4; n++)
                        {
                            if (ImGui::Selectable(fpsPosition[n], (Config::Instance()->FpsOverlayPos.value_or_default() == n)))
                                Config::Instance()->FpsOverlayPos = n;
                        }

                        ImGui::EndCombo();
                    }

                    const char* fpsType[] = { "Simple", "Detailed", "Detailed + Graph", "Full", "Full + Graph" };
                    const char* selectedType = fpsType[Config::Instance()->FpsOverlayType.value_or_default()];

                    if (ImGui::BeginCombo("Overlay Type", selectedType))
                    {
                        for (int n = 0; n < 5; n++)
                        {
                            if (ImGui::Selectable(fpsType[n], (Config::Instance()->FpsOverlayType.value_or_default() == n)))
                                Config::Instance()->FpsOverlayType = n;
                        }

                        ImGui::EndCombo();
                    }

                    float fpsAlpha = Config::Instance()->FpsOverlayAlpha.value_or_default();
                    if (ImGui::SliderFloat("Background Alpha", &fpsAlpha, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->FpsOverlayAlpha = fpsAlpha;
                }

                // ADVANCED SETTINGS -----------------------------
                ImGui::Spacing();
                if (ImGui::CollapsingHeader("Upscaler Inputs", currentFeature == nullptr ? ImGuiTreeNodeFlags_DefaultOpen : 0))
                {
                    ScopedIndent indent{};
                    ImGui::Spacing();
                    bool fsr2Inputs = Config::Instance()->Fsr2Inputs.value_or_default();
                    bool fsr2Pattern = Config::Instance()->Fsr2Pattern.value_or_default();
                    bool fsr3Inputs = Config::Instance()->Fsr3Inputs.value_or_default();
                    bool fsr3Pattern = Config::Instance()->Fsr3Pattern.value_or_default();
                    bool ffxInputs = Config::Instance()->FfxInputs.value_or_default();

                    if (ImGui::Checkbox("Use Fsr2 Inputs", &fsr2Inputs))
                        Config::Instance()->Fsr2Inputs = fsr2Inputs;

                    if (ImGui::Checkbox("Use Fsr2 Pattern Matching", &fsr2Pattern))
                        Config::Instance()->Fsr2Pattern = fsr2Pattern;
                    ShowTooltip("This setting will become active on next boot!");

                    if (ImGui::Checkbox("Use Fsr3 Inputs", &fsr3Inputs))
                        Config::Instance()->Fsr3Inputs = fsr3Inputs;

                    if (ImGui::Checkbox("Use Fsr3 Pattern Matching", &fsr3Pattern))
                        Config::Instance()->Fsr3Pattern = fsr3Pattern;
                    ShowTooltip("This setting will become active on next boot!");

                    if (ImGui::Checkbox("Use Ffx Inputs", &ffxInputs))
                        Config::Instance()->FfxInputs = ffxInputs;
                }

                // DX11 & DX12 -----------------------------
                if (State::Instance().api != Vulkan)
                {
                    // MIPMAP BIAS & Anisotropy -----------------------------
                    ImGui::Spacing();
                    if (ImGui::CollapsingHeader("Mipmap Bias", currentFeature == nullptr ? ImGuiTreeNodeFlags_DefaultOpen : 0))
                    {
                        ScopedIndent indent{};
                        ImGui::Spacing();
                        if (Config::Instance()->MipmapBiasOverride.has_value() && _mipBias == 0.0f)
                            _mipBias = Config::Instance()->MipmapBiasOverride.value();

                        ImGui::SliderFloat("Mipmap Bias##2", &_mipBias, -15.0f, 15.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);
                        ShowHelpMarker("Can help with blurry textures in broken games\n"
                                       "Negative values will make textures sharper\n"
                                       "Positive values will make textures more blurry\n\n"
                                       "Has a small performance impact");

                        ImGui::BeginDisabled(!Config::Instance()->MipmapBiasOverride.has_value());
                        {
                            ImGui::BeginDisabled(Config::Instance()->MipmapBiasScaleOverride.has_value() && Config::Instance()->MipmapBiasScaleOverride.value());
                            {
                                bool mbFixed = Config::Instance()->MipmapBiasFixedOverride.value_or_default();
                                if (ImGui::Checkbox("MB Fixed Override", &mbFixed))
                                {
                                    Config::Instance()->MipmapBiasScaleOverride.reset();
                                    Config::Instance()->MipmapBiasFixedOverride = mbFixed;
                                }

                                ShowHelpMarker("Apply same override value to all textures");
                            }
                            ImGui::EndDisabled();

                            ImGui::SameLine(0.0f, 6.0f);

                            ImGui::BeginDisabled(Config::Instance()->MipmapBiasFixedOverride.has_value() && Config::Instance()->MipmapBiasFixedOverride.value());
                            {
                                bool mbScale = Config::Instance()->MipmapBiasScaleOverride.value_or_default();
                                if (ImGui::Checkbox("MB Scale Override", &mbScale))
                                {
                                    Config::Instance()->MipmapBiasFixedOverride.reset();
                                    Config::Instance()->MipmapBiasScaleOverride = mbScale;
                                }

                                ShowHelpMarker("Apply override value as scale multiplier\n"
                                               "When using scale mode please use positive\n"
                                               "override values to increase sharpness!");
                            }
                            ImGui::EndDisabled();

                            bool mbAll = Config::Instance()->MipmapBiasOverrideAll.value_or_default();
                            if (ImGui::Checkbox("MB Override All Textures", &mbAll))
                                Config::Instance()->MipmapBiasOverrideAll = mbAll;

                            ShowHelpMarker("Override all textures mipmap values\n"
                                           "Normally OptiScaler only overrides\n"
                                           "below zero mipmap values!");
                        }
                        ImGui::EndDisabled();

                        ImGui::BeginDisabled(Config::Instance()->MipmapBiasOverride.has_value() && Config::Instance()->MipmapBiasOverride.value() == _mipBias);
                        {
                            if (ImGui::Button("Set"))
                            {
                                Config::Instance()->MipmapBiasOverride = _mipBias;
                                State::Instance().lastMipBias = 100.0f;
                                State::Instance().lastMipBiasMax = -100.0f;
                            }
                        }
                        ImGui::EndDisabled();

                        ImGui::SameLine(0.0f, 6.0f);

                        ImGui::BeginDisabled(!Config::Instance()->MipmapBiasOverride.has_value());
                        {
                            if (ImGui::Button("Reset"))
                            {
                                Config::Instance()->MipmapBiasOverride.reset();
                                _mipBias = 0.0f;
                                State::Instance().lastMipBias = 100.0f;
                                State::Instance().lastMipBiasMax = -100.0f;
                            }
                        }
                        ImGui::EndDisabled();

                        if (currentFeature != nullptr)
                        {
                            ImGui::SameLine(0.0f, 6.0f);

                            if (ImGui::Button("Calculate Mipmap Bias"))
                                _showMipmapCalcWindow = true;
                        }

                        if (Config::Instance()->MipmapBiasOverride.has_value())
                        {
                            if (Config::Instance()->MipmapBiasFixedOverride.value_or_default())
                            {
                                ImGui::Text("Current : %.3f / %.3f, Target: %.3f",
                                            State::Instance().lastMipBias, State::Instance().lastMipBiasMax, Config::Instance()->MipmapBiasOverride.value());
                            }
                            else if (Config::Instance()->MipmapBiasScaleOverride.value_or_default())
                            {
                                ImGui::Text("Current : %.3f / %.3f, Target: Base * %.3f",
                                            State::Instance().lastMipBias, State::Instance().lastMipBiasMax, Config::Instance()->MipmapBiasOverride.value());
                            }
                            else
                            {
                                ImGui::Text("Current : %.3f / %.3f, Target: Base + %.3f",
                                            State::Instance().lastMipBias, State::Instance().lastMipBiasMax, Config::Instance()->MipmapBiasOverride.value());
                            }
                        }
                        else
                        {
                            ImGui::Text("Current : %.3f / %.3f", State::Instance().lastMipBias, State::Instance().lastMipBiasMax);
                        }

                        ImGui::Text("Will be applied after RESOLUTION/PRESET change !!!");
                    }

                    ImGui::Spacing();
                    if (ImGui::CollapsingHeader("Anisotropic Filtering", currentFeature == nullptr ? ImGuiTreeNodeFlags_DefaultOpen : 0))
                    {
                        ScopedIndent indent{};
                        ImGui::Spacing();
                        ImGui::PushItemWidth(65.0f * Config::Instance()->MenuScale.value());

                        auto selectedAF = Config::Instance()->AnisotropyOverride.has_value() ? std::to_string(Config::Instance()->AnisotropyOverride.value()) : "Auto";
                        if (ImGui::BeginCombo("Force Anisotropic Filtering", selectedAF.c_str()))
                        {
                            if (ImGui::Selectable("Auto", !Config::Instance()->AnisotropyOverride.has_value()))
                                Config::Instance()->AnisotropyOverride.reset();

                            if (ImGui::Selectable("1", Config::Instance()->AnisotropyOverride.value_or(0) == 1))
                                Config::Instance()->AnisotropyOverride = 1;

                            if (ImGui::Selectable("2", Config::Instance()->AnisotropyOverride.value_or(0) == 2))
                                Config::Instance()->AnisotropyOverride = 2;

                            if (ImGui::Selectable("4", Config::Instance()->AnisotropyOverride.value_or(0) == 4))
                                Config::Instance()->AnisotropyOverride = 4;

                            if (ImGui::Selectable("8", Config::Instance()->AnisotropyOverride.value_or(0) == 8))
                                Config::Instance()->AnisotropyOverride = 8;

                            if (ImGui::Selectable("16", Config::Instance()->AnisotropyOverride.value_or(0) == 16))
                                Config::Instance()->AnisotropyOverride = 16;

                            ImGui::EndCombo();
                        }

                        ImGui::PopItemWidth();

                        ImGui::Text("Will be applied after RESOLUTION/PRESET change !!!");
                    }

                    if (currentFeature != nullptr)
                    {
                        // Non-DLSS hotfixes -----------------------------
                        if (currentBackend != "dlss")
                        {
                            // BARRIERS -----------------------------
                            ImGui::Spacing();
                            if (ImGui::CollapsingHeader("Resource Barriers"))
                            {
                                ScopedIndent indent{};
                                ImGui::Spacing();
                                AddResourceBarrier("Color", &Config::Instance()->ColorResourceBarrier);
                                AddResourceBarrier("Depth", &Config::Instance()->DepthResourceBarrier);
                                AddResourceBarrier("Motion", &Config::Instance()->MVResourceBarrier);
                                AddResourceBarrier("Exposure", &Config::Instance()->ExposureResourceBarrier);
                                AddResourceBarrier("Mask", &Config::Instance()->MaskResourceBarrier);
                                AddResourceBarrier("Output", &Config::Instance()->OutputResourceBarrier);
                            }

                            // HOTFIXES -----------------------------
                            if (State::Instance().api == DX12)
                            {
                                ImGui::Spacing();
                                if (ImGui::CollapsingHeader("Root Signatures"))
                                {
                                    ScopedIndent indent{};
                                    ImGui::Spacing();
                                    if (bool crs = Config::Instance()->RestoreComputeSignature.value_or_default(); ImGui::Checkbox("Restore Compute Root Signature", &crs))
                                        Config::Instance()->RestoreComputeSignature = crs;

                                    if (bool grs = Config::Instance()->RestoreGraphicSignature.value_or_default(); ImGui::Checkbox("Restore Graphic Root Signature", &grs))
                                        Config::Instance()->RestoreGraphicSignature = grs;
                                }
                            }
                        }
                    }
                }

                ImGui::EndTable();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                State::Instance().frameTimes.pop_front();
                State::Instance().frameTimes.push_back(1000.0 / io.Framerate);

                if (ImGui::BeginTable("plots", 2, ImGuiTableFlags_SizingStretchSame))
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("FrameTime");
                    auto ft = std::format("{:5.2f} ms / {:5.1f} fps", State::Instance().frameTimes.back(), io.Framerate);
                    std::vector<float> frameTimeArray(State::Instance().frameTimes.begin(), State::Instance().frameTimes.end());
                    ImGui::PlotLines(ft.c_str(), frameTimeArray.data(), frameTimeArray.size());


                    if (currentFeature != nullptr)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("Upscaler");
                        auto ups = std::format("{:7.4f} ms", State::Instance().upscaleTimes.back());
                        std::vector<float> upscaleTimeArray(State::Instance().upscaleTimes.begin(), State::Instance().upscaleTimes.end());
                        ImGui::PlotLines(ups.c_str(), upscaleTimeArray.data(), upscaleTimeArray.size());
                    }

                    ImGui::EndTable();
                }

                // BOTTOM LINE ---------------
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (currentFeature != nullptr)
                {
                    ImGui::Text("%dx%d -> %dx%d (%.1f) [%dx%d (%.1f)]",
                                currentFeature->RenderWidth(), currentFeature->RenderHeight(), currentFeature->TargetWidth(), currentFeature->TargetHeight(), (float)currentFeature->TargetWidth() / (float)currentFeature->RenderWidth(),
                                currentFeature->DisplayWidth(), currentFeature->DisplayHeight(), (float)currentFeature->DisplayWidth() / (float)currentFeature->RenderWidth());

                    ImGui::SameLine(0.0f, 4.0f);

                    ImGui::Text("%d", State::Instance().currentFeature->FrameCount());

                    ImGui::SameLine(0.0f, 10.0f);
                }

                ImGui::PushItemWidth(55.0f * Config::Instance()->MenuScale.value());

                const char* uiScales[] = { "0.5", "0.6", "0.7", "0.8", "0.9", "1.0", "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8", "1.9", "2.0" };
                const char* selectedScaleName = uiScales[_selectedScale];

                if (ImGui::BeginCombo("UI Scale", selectedScaleName))
                {
                    for (int n = 0; n < 16; n++)
                    {
                        if (ImGui::Selectable(uiScales[n], (_selectedScale == n)))
                        {
                            _selectedScale = n;
                            Config::Instance()->MenuScale = 0.5f + (float)n / 10.0f;

                            ImGuiStyle& style = ImGui::GetStyle();
                            style.ScaleAllSizes(Config::Instance()->MenuScale.value());

                            if (Config::Instance()->MenuScale.value() < 1.0f)
                                style.MouseCursorScale = 1.0f;

                            _imguiSizeUpdate = true;
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopItemWidth();


                ImGui::SameLine(0.0f, 15.0f);

                if (ImGui::Button("Save INI"))
                    Config::Instance()->SaveIni();

                ImGui::SameLine(0.0f, 6.0f);

                if (ImGui::Button("Close"))
                {
                    _showMipmapCalcWindow = false;
                    _isVisible = false;
                }

                ImGui::Spacing();
                ImGui::Separator();

                if (State::Instance().nvngxIniDetected)
                {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "nvngx.ini detected, please move over to using OptiScaler.ini and delete the old config");
                    ImGui::Spacing();
                }

                auto winSize = ImGui::GetWindowSize();
                auto winPos = ImGui::GetWindowPos();

                if (winPos.x == 60.0 && winSize.x > 100)
                {
                    float posX;
                    float posY;

                    if (currentFeature != nullptr)
                    {
                        posX = ((float)State::Instance().currentFeature->DisplayWidth() - winSize.x) / 2.0f;
                        posY = ((float)State::Instance().currentFeature->DisplayHeight() - winSize.y) / 2.0f;
                    }
                    else
                    {
                        posX = ((float)State::Instance().screenWidth - winSize.x) / 2.0f;
                        posY = ((float)State::Instance().screenHeight - winSize.x) / 2.0f;
                    }

                    // don't position menu outside of screen
                    if (posX < 0.0 || posY < 0.0)
                    {
                        posX = 50;
                        posY = 50;
                    }

                    ImGui::SetWindowPos(ImVec2{ posX, posY });
                }


                ImGui::End();
            }

            // Mipmap calculation window
            if (_showMipmapCalcWindow && currentFeature != nullptr && currentFeature->IsInited())
            {
                auto posX = (State::Instance().currentFeature->DisplayWidth() - 450.0f) / 2.0f;
                auto posY = (State::Instance().currentFeature->DisplayHeight() - 200.0f) / 2.0f;

                ImGui::SetNextWindowPos(ImVec2{ posX , posY }, ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2{ 450.0f, 200.0f }, ImGuiCond_FirstUseEver);

                if (_displayWidth == 0)
                {
                    if (Config::Instance()->OutputScalingEnabled.value_or_default())
                        _displayWidth = State::Instance().currentFeature->DisplayWidth() * Config::Instance()->OutputScalingMultiplier.value_or_default();
                    else
                        _displayWidth = State::Instance().currentFeature->DisplayWidth();

                    _renderWidth = _displayWidth / 3.0f;
                    _mipmapUpscalerQuality = 0;
                    _mipmapUpscalerRatio = 3.0f;
                    _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
                }

                if (ImGui::Begin("Mipmap Bias", nullptr, flags))
                {
                    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                        ImGui::SetWindowFocus();

                    if (ImGui::InputScalar("Display Width", ImGuiDataType_U32, &_displayWidth, NULL, NULL, "%u"))
                    {
                        if (_displayWidth <= 0)
                        {
                            if (Config::Instance()->OutputScalingEnabled.value_or_default())
                                _displayWidth = State::Instance().currentFeature->DisplayWidth() * Config::Instance()->OutputScalingMultiplier.value_or_default();
                            else
                                _displayWidth = State::Instance().currentFeature->DisplayWidth();

                        }

                        _renderWidth = _displayWidth / _mipmapUpscalerRatio;
                        _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
                    }

                    const char* q[] = { "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality", "DLAA" };
                    float fr[] = { 3.0f, 2.0f, 1.7f, 1.5f, 1.3f, 1.0f };
                    auto configQ = _mipmapUpscalerQuality;

                    const char* selectedQ = q[configQ];

                    ImGui::BeginDisabled(Config::Instance()->UpscaleRatioOverrideEnabled.value_or_default());

                    if (ImGui::BeginCombo("Upscaler Quality", selectedQ))
                    {
                        for (int n = 0; n < 6; n++)
                        {
                            if (ImGui::Selectable(q[n], (_mipmapUpscalerQuality == n)))
                            {
                                _mipmapUpscalerQuality = n;

                                float ov = -1.0f;

                                if (Config::Instance()->QualityRatioOverrideEnabled.value_or_default())
                                {
                                    switch (n)
                                    {
                                        case 0:
                                            ov = Config::Instance()->QualityRatio_UltraPerformance.value_or(-1.0f);
                                            break;

                                        case 1:
                                            ov = Config::Instance()->QualityRatio_Performance.value_or(-1.0f);
                                            break;

                                        case 2:
                                            ov = Config::Instance()->QualityRatio_Balanced.value_or(-1.0f);
                                            break;

                                        case 3:
                                            ov = Config::Instance()->QualityRatio_Quality.value_or(-1.0f);
                                            break;

                                        case 4:
                                            ov = Config::Instance()->QualityRatio_UltraQuality.value_or(-1.0f);
                                            break;
                                    }
                                }

                                if (ov > 0.0f)
                                    _mipmapUpscalerRatio = ov;
                                else
                                    _mipmapUpscalerRatio = fr[n];

                                _renderWidth = _displayWidth / _mipmapUpscalerRatio;
                                _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
                            }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::EndDisabled();

                    if (ImGui::SliderFloat("Upscaler Ratio", &_mipmapUpscalerRatio,
                        Config::Instance()->ExtendedLimits.value_or_default() ? 0.1f : 1.0f,
                        Config::Instance()->ExtendedLimits.value_or_default() ? 6.0f : 3.0f,
                        "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                    {
                        _renderWidth = _displayWidth / _mipmapUpscalerRatio;
                        _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
                    }

                    if (ImGui::InputScalar("Render Width", ImGuiDataType_U32, &_renderWidth, NULL, NULL, "%u"))
                        _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);

                    ImGui::SliderFloat("Mipmap Bias", &_mipBiasCalculated, -15.0f, 0.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);

                    // BOTTOM LINE
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);

                    if (ImGui::Button("Use Value"))
                    {
                        _mipBias = _mipBiasCalculated;
                        _showMipmapCalcWindow = false;
                    }

                    ImGui::SameLine(0.0f, 6.0f);

                    if (ImGui::Button("Close"))
                        _showMipmapCalcWindow = false;

                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::End();
                }
            }
        }

        return true;
    }
}

void MenuCommon::Init(HWND InHwnd)
{
    _handle = InHwnd;
    _isVisible = false;

    LOG_DEBUG("Handle: {0:X}", (size_t)_handle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;

    io.MouseDrawCursor = _isVisible;
    io.WantCaptureKeyboard = _isVisible;
    io.WantCaptureMouse = _isVisible;
    io.WantSetMousePos = _isVisible;

    io.IniFilename = io.LogFilename = nullptr;

    bool initResult = ImGui_ImplWin32_Init(InHwnd);
    LOG_DEBUG("ImGui_ImplWin32_Init result: {0}", initResult);

    MenuBase::UpdateFonts(io, Config::Instance()->MenuScale.value_or_default());

    if (!Config::Instance()->OverlayMenu.value_or_default()) {
        _imguiSizeUpdate = true;
        _hdrTonemapApplied = false;
    }

    if (_oWndProc == nullptr)
        _oWndProc = (WNDPROC)SetWindowLongPtr(InHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    LOG_DEBUG("_oWndProc: {0:X}", (ULONG64)_oWndProc);

    if (!pfn_SetCursorPos_hooked)
        AttachHooks();

    _isInited = true;
}

void MenuCommon::Shutdown()
{
    if (!MenuCommon::_isInited)
        return;


    if (_oWndProc != nullptr)
    {
        SetWindowLongPtr((HWND)ImGui::GetMainViewport()->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)_oWndProc);
        _oWndProc = nullptr;
    }

    if (pfn_SetCursorPos_hooked)
        DetachHooks();

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    _isInited = false;
    _isVisible = false;
}

void MenuCommon::HideMenu()
{
    if (!_isVisible)
        return;

    _isVisible = false;

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (pfn_ClipCursor_hooked)
        pfn_ClipCursor(&_cursorLimit);

    _showMipmapCalcWindow = false;

    RECT windowRect = {};

    if (GetWindowRect(_handle, &windowRect))
    {
        auto x = windowRect.left + (windowRect.right - windowRect.left) / 2;
        auto y = windowRect.top + (windowRect.bottom - windowRect.top) / 2;

        if (pfn_SetCursorPos != nullptr)
            pfn_SetCursorPos(x, y);
        else
            SetCursorPos(x, y);
    }

    io.MouseDrawCursor = _isVisible;
    io.WantCaptureKeyboard = _isVisible;
    io.WantCaptureMouse = _isVisible;
}
