#include "imgui_common.h"

void ImGuiCommon::ShowTooltip(const char* tip) {
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0)); // Scale the tooltip text
        ImGui::Text(tip);
        ImGui::EndTooltip();
    }
}

void ImGuiCommon::ShowHelpMarker(const char* tip)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    ShowTooltip(tip);
}

LRESULT __stdcall ImGuiCommon::hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (_isVisible && Msg == 0x0020)
        return TRUE;
    else
        return pfn_SendMessageW(hWnd, Msg, wParam, lParam);
}

BOOL __stdcall ImGuiCommon::hkSetPhysicalCursorPos(int x, int y)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SetPhysicalCursorPos(x, y);
}

BOOL __stdcall ImGuiCommon::hkSetCursorPos(int x, int y)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SetCursorPos(x, y);
}

BOOL __stdcall ImGuiCommon::hkClipCursor(RECT* lpRect)
{
    if (_isVisible)
        return TRUE;
    else
    {
        _lastCursorLimit = lpRect;
        return pfn_ClipCursor(lpRect);
    }
}

void __stdcall ImGuiCommon::hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
{
    if (_isVisible)
        return;
    else
        pfn_mouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
}

UINT __stdcall ImGuiCommon::hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
    if (_isVisible)
        return TRUE;
    else
        return pfn_SendInput(cInputs, pInputs, cbSize);
}

void ImGuiCommon::AttachHooks()
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

void ImGuiCommon::DetachHooks()
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

ImGuiKey ImGuiCommon::ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
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

LRESULT __stdcall ImGuiCommon::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (_isResetRequested || (!_dx11Ready && !_dx12Ready && !_vulkanReady))
    {
        if (_isVisible)
        {
            LOG_INFO("No active features, closing ImGui");

            if (pfn_ClipCursor_hooked && _lastCursorLimit != nullptr)
                pfn_ClipCursor(_lastCursorLimit);

            _isVisible = false;
            _showMipmapCalcWindow = false;

            io.MouseDrawCursor = false;
            io.WantCaptureKeyboard = false;
            io.WantCaptureMouse = false;
        }

        return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
    }

    bool rawRead = false;
    bool vmInputMenu = false;
    bool vmInputReset = false;
    auto rawCode = GET_RAWINPUT_CODE_WPARAM(wParam);
    RAWINPUT rawData;
    UINT rawDataSize = sizeof(rawData);
    ImGuiKey imguiKey;

    if (msg == WM_INPUT &&
        GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &rawData, &rawDataSize, sizeof(rawData.data)) != (UINT)-1)
    {
        rawRead = true;
        if (rawData.header.dwType == RIM_TYPEKEYBOARD && rawData.data.keyboard.VKey != 0)
        {
            vmInputMenu = rawData.data.keyboard.VKey == Config::Instance()->ShortcutKey.value_or(VK_INSERT);
            //vmInputReset = rawData.data.keyboard.VKey == Config::Instance()->ResetKey.value_or(VK_END);
        }
    }

    // END - REINIT MENU
    //if ((msg == WM_KEYDOWN && wParam == Config::Instance()->ResetKey.value_or(VK_END)) || vmInputReset)
    //{
    //    _isResetRequested = true;
    //    return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
    //}

    // INSERT - OPEN MENU
    if (((msg == WM_KEYDOWN && wParam == Config::Instance()->ShortcutKey.value_or(VK_INSERT)) || vmInputMenu))
    {
        _isVisible = !_isVisible;

        if (_isVisible)
            Config::Instance()->ReloadFakenvapi();

        if (!_isVisible)
        {
            if (pfn_ClipCursor_hooked && _lastCursorLimit != nullptr)
                pfn_ClipCursor(_lastCursorLimit);

            _showMipmapCalcWindow = false;
        }
        else if (pfn_ClipCursor_hooked)
        {
            _ssRatio = 0;

            if (GetWindowRect(_handle, &_cursorLimit))
                pfn_ClipCursor(&_cursorLimit);
            else
                pfn_ClipCursor(nullptr);
        }

        //RECT windowRect = {};

        //if (GetWindowRect(_handle, &windowRect))
        //{
        //    auto x = windowRect.left + (windowRect.right - windowRect.left) / 2;
        //    auto y = windowRect.top + (windowRect.bottom - windowRect.top) / 2;

        //    if (pfn_SetCursorPos != nullptr)
        //        pfn_SetCursorPos(x, y);
        //    else
        //        SetCursorPos(x, y);
        //}

        io.MouseDrawCursor = _isVisible;
        io.WantCaptureKeyboard = _isVisible;
        io.WantCaptureMouse = _isVisible;

        LOG_TRACE("Menu key pressed, {0}", _isVisible ? "opening ImGui" : "closing ImGui");

        return TRUE;
    }

    // SHUFT + DEL - Debug dump
    if (msg == WM_KEYDOWN && wParam == VK_DELETE && (GetKeyState(VK_SHIFT) & 0x8000))
    {
        Config::Instance()->xessDebug = true;
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
                if (wParam != Config::Instance()->ShortcutKey.value_or(VK_INSERT))
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

std::string ImGuiCommon::GetBackendName(std::string* code)
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

std::string ImGuiCommon::GetBackendCode(const NVNGX_Api api)
{
    std::string code;

    if (api == NVNGX_DX11)
        code = Config::Instance()->Dx11Upscaler.value_or("fsr22");
    else if (api == NVNGX_DX12)
        code = Config::Instance()->Dx12Upscaler.value_or("xess");
    else
        code = Config::Instance()->VulkanUpscaler.value_or("fsr21");

    return code;
}

void ImGuiCommon::GetCurrentBackendInfo(const NVNGX_Api api, std::string* code, std::string* name)
{
    *code = GetBackendCode(api);
    *name = GetBackendName(code);
}

void ImGuiCommon::AddDx11Backends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
        selectedUpscalerName = "FSR 2.2.1";
    else if (Config::Instance()->newBackend == "fsr22_12" || (Config::Instance()->newBackend == "" && *code == "fsr22_12"))
        selectedUpscalerName = "FSR 2.2.1 w/Dx12";
    else if (Config::Instance()->newBackend == "fsr21_12" || (Config::Instance()->newBackend == "" && *code == "fsr21_12"))
        selectedUpscalerName = "FSR 2.1.2 w/Dx12";
    else if (Config::Instance()->newBackend == "fsr31" || (Config::Instance()->newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (Config::Instance()->newBackend == "fsr31_12" || (Config::Instance()->newBackend == "" && *code == "fsr31_12"))
        selectedUpscalerName = "FSR 3.X w/Dx12";
    //else if (Config::Instance()->newBackend == "fsr304" || (Config::Instance()->newBackend == "" && *code == "fsr304"))
    //    selectedUpscalerName = "FSR 3.0.4";
    else if (Config::Instance()->DLSSEnabled.value_or(true) && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "XeSS w/Dx12";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            Config::Instance()->newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            Config::Instance()->newBackend = "fsr31";

        if (ImGui::Selectable("XeSS w/Dx12", *code == "xess"))
            Config::Instance()->newBackend = "xess";

        if (ImGui::Selectable("FSR 2.1.2 w/Dx12", *code == "fsr21_12"))
            Config::Instance()->newBackend = "fsr21_12";

        if (ImGui::Selectable("FSR 2.2.1 w/Dx12", *code == "fsr22_12"))
            Config::Instance()->newBackend = "fsr22_12";

        if (ImGui::Selectable("FSR 3.X w/Dx12", *code == "fsr31_12"))
            Config::Instance()->newBackend = "fsr31_12";

        if (Config::Instance()->DLSSEnabled.value_or(true) && ImGui::Selectable("DLSS", *code == "dlss"))
            Config::Instance()->newBackend = "dlss";

        ImGui::EndCombo();
    }
}

void ImGuiCommon::AddDx12Backends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
        selectedUpscalerName = "FSR 2.1.2";
    else if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
        selectedUpscalerName = "FSR 2.2.1";
    else if (Config::Instance()->newBackend == "fsr31" || (Config::Instance()->newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (Config::Instance()->DLSSEnabled.value_or(true) && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "XeSS";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("XeSS", *code == "xess"))
            Config::Instance()->newBackend = "xess";

        if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
            Config::Instance()->newBackend = "fsr21";

        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            Config::Instance()->newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            Config::Instance()->newBackend = "fsr31";

        if (Config::Instance()->DLSSEnabled.value_or(true) && ImGui::Selectable("DLSS", *code == "dlss"))
            Config::Instance()->newBackend = "dlss";

        ImGui::EndCombo();
    }
}

void ImGuiCommon::AddVulkanBackends(std::string* code, std::string* name)
{
    std::string selectedUpscalerName = "";

    if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
        selectedUpscalerName = "FSR 2.1.2";
    else if (Config::Instance()->newBackend == "fsr31" || (Config::Instance()->newBackend == "" && *code == "fsr31"))
        selectedUpscalerName = "FSR 3.X";
    else if (Config::Instance()->DLSSEnabled.value_or(true) && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
        selectedUpscalerName = "DLSS";
    else
        selectedUpscalerName = "FSR 2.2.1";

    if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
    {
        if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
            Config::Instance()->newBackend = "fsr21";

        if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
            Config::Instance()->newBackend = "fsr22";

        if (ImGui::Selectable("FSR 3.X", *code == "fsr31"))
            Config::Instance()->newBackend = "fsr31";

        if (Config::Instance()->DLSSEnabled.value_or(true) && ImGui::Selectable("DLSS", *code == "dlss"))
            Config::Instance()->newBackend = "dlss";

        ImGui::EndCombo();
    }
}

void ImGuiCommon::AddResourceBarrier(std::string name, std::optional<int>* value)
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

void ImGuiCommon::AddRenderPreset(std::string name, std::optional<uint32_t>* value)
{
    const char* presets[] = { "DEFAULT", "PRESET A", "PRESET B", "PRESET C", "PRESET D", "PRESET E", "PRESET F", "PRESET G" };
    const char* presetsDesc[] = { "Whatever the game uses",
        "Intended for Performance/Balanced/Quality modes.\nAn older variant best suited to combat ghosting for elements with missing inputs, such as motion vectors.",
        "Intended for Ultra Performance mode.\nSimilar to Preset A but for Ultra Performance mode.",
        "Intended for Performance/Balanced/Quality modes.\nGenerally favors current frame information;\nwell suited for fast-paced game content.",
        "Default preset for Performance/Balanced/Quality modes;\ngenerally favors image stability.",
        "DLSS 3.7+, a better D preset",
        "Default preset for Ultra Performance and DLAA modes",
        "Unused"
    };

    PopulateCombo(name, value, presets, presetsDesc, 8);
}

void ImGuiCommon::PopulateCombo(std::string name, std::optional<uint32_t>* value, const char* names[], const char* desc[], int length) {
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
            value->reset();
        ShowTooltip(desc[0]);

        for (int n = 1; n < length; n++)
        {
            if (ImGui::Selectable(names[n], selected == n))
            {
                if (n != selected)
                    *value = n;
            }
            ShowTooltip(desc[n]);
        }

        ImGui::EndCombo();
    }
}

void ImGuiCommon::RenderMenu()
{
    if (!_isInited || !_isVisible)
        return;

    ImGuiIO const& io = ImGui::GetIO(); (void)io;

    {
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoSavedSettings;
        flags |= ImGuiWindowFlags_NoCollapse;

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
            style.ScaleAllSizes(Config::Instance()->MenuScale.value_or(1.0));
            CopyMemory(style.Colors, styleold.Colors, sizeof(style.Colors)); // Restore colors		
        }

        ImGui::NewFrame();

        auto cf = Config::Instance()->CurrentFeature;

        auto size = ImVec2{ 0.0f, 0.0f };
        ImGui::SetNextWindowSize(size);

        float posX;
        float posY;

        if (cf != nullptr)
        {
            posX = ((float)Config::Instance()->CurrentFeature->DisplayWidth() - 770.0f) / 2.0f;
            posY = ((float)Config::Instance()->CurrentFeature->DisplayHeight() - 685.0f) / 2.0f;
        }
        else
        {
            posX = ((float)Config::Instance()->ScreenWidth - 770.0f) / 2.0f;
            posY = ((float)Config::Instance()->ScreenHeight - 685.0f) / 2.0f;
        }

        // don't position menu outside of screen
        if (posX < 0.0 || posY < 0.0)
        {
            posX = 50;
            posY = 50;
        }

        ImGui::SetNextWindowPos(ImVec2{ posX, posY }, ImGuiCond_FirstUseEver);

        if (ImGui::Begin(VER_PRODUCT_NAME, NULL, flags))
        {
            bool rcasEnabled = false;

            if (!_showMipmapCalcWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                ImGui::SetWindowFocus();

            ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));

            std::string selectedUpscalerName = "";
            std::string currentBackend = "";
            std::string currentBackendName = "";

            if (cf == nullptr)
            {
                ImGui::Spacing();
                ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0) * 2);
                ImGui::Text("Please select DLSS as upscaler from game options and\nenter the game to enable upscaler settings.");
                ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));
            }

            if (ImGui::BeginTable("main", 2))
            {
                ImGui::TableNextColumn();

                if (cf != nullptr)
                {
                    // UPSCALERS -----------------------------
                    ImGui::SeparatorText("Upscalers");
                    ShowTooltip("Which copium you choose?");

                    GetCurrentBackendInfo(Config::Instance()->Api, &currentBackend, &currentBackendName);

                    switch (Config::Instance()->Api)
                    {
                        case NVNGX_DX11:
                            ImGui::Text("DirectX 11 %s- %s (%d.%d.%d)", Config::Instance()->IsRunningOnDXVK ? "(DXVK) " : "", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

                            if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
                                AddDx11Backends(&currentBackend, &currentBackendName);

                            break;

                        case NVNGX_DX12:
                            ImGui::Text("DirectX 12 %s- %s (%d.%d.%d)", Config::Instance()->IsRunningOnDXVK ? "(DXVK) " : "", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

                            ImGui::SameLine(0.0f, 6.0f);
                            ImGui::Text("Source Api: %s", Config::Instance()->currentInputApiName.c_str());

                            if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
                                AddDx12Backends(&currentBackend, &currentBackendName);

                            break;

                        default:
                            ImGui::Text("Vulkan %s- %s (%d.%d.%d)", Config::Instance()->IsRunningOnDXVK ? "(DXVK) " : "", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

                            if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
                                AddVulkanBackends(&currentBackend, &currentBackendName);
                    }

                    if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
                    {
                        if (ImGui::Button("Apply") && Config::Instance()->newBackend != "" && Config::Instance()->newBackend != currentBackend)
                        {
                            if (Config::Instance()->newBackend == "xess")
                            {
                                // Reseting them for xess 
                                Config::Instance()->DisableReactiveMask.reset();
                                Config::Instance()->DlssReactiveMaskBias.reset();
                            }

                            Config::Instance()->changeBackend = true;
                        }

                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::Button("Revert"))
                            Config::Instance()->newBackend = "";
                    }

                    // DYNAMIC PROPERTIES -----------------------------

                    // Frame Generation
                    if (Config::Instance()->FGUseFGSwapChain.value_or(true) && Config::Instance()->OverlayMenu.value_or(true) &&
                        Config::Instance()->Api == NVNGX_DX12 && !Config::Instance()->WorkingAsNvngx)
                    {
                        ImGui::SeparatorText("Frame Generation (Dx12)");

                        bool fgActive = Config::Instance()->FGEnabled.value_or(false);
                        if (ImGui::Checkbox("Frame Generation", &fgActive))
                        {
                            Config::Instance()->FGEnabled = fgActive;
                            LOG_DEBUG("Enabled set FGEnabled: {}", fgActive);

                            if (Config::Instance()->FGEnabled.value_or(false))
                                Config::Instance()->FGChanged = true;
                        }

                        ShowHelpMarker("Enable FSR frame generation");

                        bool fgHudfix = Config::Instance()->FGHUDFix.value_or(false);
                        if (ImGui::Checkbox("FG HUD Fix", &fgHudfix))
                        {
                            Config::Instance()->FGHUDFix = fgHudfix;
                            LOG_DEBUG("Enabled set FGHUDFix: {}", fgHudfix);
                            Config::Instance()->FGChanged = true;
                        }
                        ShowHelpMarker("Enable HUD stability fix, might cause crashes!");

                        ImGui::BeginDisabled(!Config::Instance()->FGHUDFix.value_or(false));

                        ImGui::SameLine(0.0f, 16.0f);
                        ImGui::PushItemWidth(85.0);
                        int hudFixLimit = Config::Instance()->FGHUDLimit.value_or(1);
                        if (ImGui::InputInt("Limit", &hudFixLimit))
                        {
                            if (hudFixLimit < 1)
                                hudFixLimit = 1;
                            else if (hudFixLimit > 99)
                                hudFixLimit = 99;

                            Config::Instance()->FGHUDLimit = hudFixLimit;
                            LOG_DEBUG("Enabled set FGHUDLimit: {}", hudFixLimit);
                        }
                        ShowHelpMarker("Delay HUDless capture, high values might cause crash!");

                        ImGui::SameLine(0.0f, 16.0f);
                        auto hudExtended = Config::Instance()->FGHUDFixExtended.value_or(false);
                        if (ImGui::Checkbox("Extended", &hudExtended))
                        {
                            LOG_DEBUG("Enabled set FGHUDFixExtended: {}", hudExtended);
                            Config::Instance()->FGHUDFixExtended = hudExtended;
                        }
                        ShowHelpMarker("Extended HUDless checks, might cause crash and slowdowns!");

                        ImGui::PopItemWidth();

                        ImGui::EndDisabled();

                        bool fgAsync = Config::Instance()->FGAsync.value_or(false);

                        if (ImGui::Checkbox("FG Allow Async", &fgAsync))
                        {
                            Config::Instance()->FGAsync = fgAsync;

                            if (Config::Instance()->FGEnabled.value_or(false))
                            {
                                Config::Instance()->FGChanged = true;
                                LOG_DEBUG_ONLY("Async set FGChanged");
                            }
                        }
                        ShowHelpMarker("Enable Async for better FG performance\nMight cause crashes, especially with HUD Fix!");

                        bool fgDV = Config::Instance()->FGDebugView.value_or(false);
                        if (ImGui::Checkbox("FG Debug View", &fgDV))
                        {
                            Config::Instance()->FGDebugView = fgDV;

                            if (Config::Instance()->FGEnabled.value_or(false))
                            {
                                Config::Instance()->FGChanged = true;
                                LOG_DEBUG_ONLY("DebugView set FGChanged");
                            }
                        }

                        ShowHelpMarker("Enable FSR 3.1 frame generation debug view");

                        ImGui::Checkbox("FG Only Generated", &Config::Instance()->FGOnlyGenerated);
                        ShowHelpMarker("Display only FSR 3.1 generated frames");

                        if (Config::Instance()->AdvancedSettings.value_or(false))
                        {
                            ImGui::PushItemWidth(95.0);
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
                        }
                    }

                    // Dx11 with Dx12
                    if (Config::Instance()->AdvancedSettings.value_or(false) && Config::Instance()->Api == NVNGX_DX11 &&
                        Config::Instance()->Dx11Upscaler.value_or("fsr22") != "fsr22" && Config::Instance()->Dx11Upscaler.value_or("fsr22") != "dlss" && Config::Instance()->Dx11Upscaler.value_or("fsr22") != "fsr31")
                    {
                        ImGui::SeparatorText("Dx11 with Dx12 Settings");

                        const char* sync[] = { "No Syncing", "Fence", "Fence + Event", "Fence + Flush", "Fence + Flush + Event", "Only Query" };

                        const char* syncDesc[] = {
                            "Self explanatory",
                            "Sync using shared Fences(Signal& Wait). These should happen on GPU which is pretty fast.",
                            "Sync using shared Fences(Signal& Event). Events are waited on CPU which is slower.",
                            "Like Fence but flushes Dx11 DeviceContext after Signal shared Fence.",
                            "Like Fence + Event but flushes Dx11 DeviceContext after Signal shared Fence.",
                            "Uses Dx11 Query to sync, in general faster than Events but slower than Fences."
                        };

                        const char* selectedSync = sync[Config::Instance()->TextureSyncMethod.value_or(1)];

                        if (ImGui::BeginCombo("Input Sync", selectedSync))
                        {
                            for (int n = 0; n < 6; n++)
                            {
                                if (ImGui::Selectable(sync[n], (Config::Instance()->TextureSyncMethod.value_or(1) == n)))
                                    Config::Instance()->TextureSyncMethod = n;
                                ShowTooltip(syncDesc[n]);
                            }

                            ImGui::EndCombo();
                        }

                        const char* sync2[] = { "No Syncing", "Fence", "Fence + Event", "Fence + Flush", "Fence + Flush + Event", "Only Query" };

                        selectedSync = sync2[Config::Instance()->CopyBackSyncMethod.value_or(5)];

                        if (ImGui::BeginCombo("Output Sync", selectedSync))
                        {
                            for (int n = 0; n < 6; n++)
                            {
                                if (ImGui::Selectable(sync2[n], (Config::Instance()->CopyBackSyncMethod.value_or(5) == n)))
                                    Config::Instance()->CopyBackSyncMethod = n;
                                ShowTooltip(syncDesc[n]);
                            }

                            ImGui::EndCombo();
                        }

                        if (bool afterDx12 = Config::Instance()->SyncAfterDx12.value_or(true); ImGui::Checkbox("Sync After Dx12", &afterDx12))
                            Config::Instance()->SyncAfterDx12 = afterDx12;
                        ShowHelpMarker("When using Events for syncing output SyncAfterDx12=false is usually more performant.");

                        ImGui::SameLine(0.0f, 6.0f);

                        if (bool dontUseNTShared = Config::Instance()->DontUseNTShared.value_or(false); ImGui::Checkbox("Don't Use NTShared", &dontUseNTShared))
                            Config::Instance()->DontUseNTShared = dontUseNTShared;
                    }

                    // UPSCALER SPECIFIC -----------------------------

                    // XeSS -----------------------------
                    if (currentBackend == "xess" && Config::Instance()->CurrentFeature->Name() != "DLSSD")
                    {
                        ImGui::SeparatorText("XeSS Settings");

                        const char* models[] = { "KPSS", "SPLAT", "MODEL_3", "MODEL_4", "MODEL_5", "MODEL_6" };
                        auto configModes = Config::Instance()->NetworkModel.value_or(0);

                        if (configModes < 0 || configModes > 5)
                            configModes = 0;

                        const char* selectedModel = models[configModes];

                        if (ImGui::BeginCombo("Network Models", selectedModel))
                        {
                            for (int n = 0; n < 6; n++)
                            {
                                if (ImGui::Selectable(models[n], (Config::Instance()->NetworkModel.value_or(0) == n)))
                                {
                                    Config::Instance()->NetworkModel = n;
                                    Config::Instance()->newBackend = currentBackend;
                                    Config::Instance()->changeBackend = true;
                                }
                            }

                            ImGui::EndCombo();
                        }
                        ShowHelpMarker("Likely don't do much");

                        if (bool dbg = Config::Instance()->xessDebug; ImGui::Checkbox("Dump (Shift+Del)", &dbg))
                            Config::Instance()->xessDebug = dbg;

                        ImGui::SameLine(0.0f, 6.0f);
                        int dbgCount = Config::Instance()->xessDebugFrames;

                        ImGui::PushItemWidth(85.0);
                        if (ImGui::InputInt("frames", &dbgCount))
                        {
                            if (dbgCount < 4)
                                dbgCount = 4;
                            else if (dbgCount > 999)
                                dbgCount = 999;

                            Config::Instance()->xessDebugFrames = dbgCount;
                        }

                        ImGui::PopItemWidth();
                    }

                    // FSR -----------------
                    if (currentBackend.rfind("fsr", 0) == 0 && Config::Instance()->CurrentFeature->Name() != "DLSSD")
                    {
                        ImGui::SeparatorText("FSR Settings");

                        if (_fsr3xIndex < 0)
                            _fsr3xIndex = Config::Instance()->Fsr3xIndex.value_or(0);

                        if (currentBackend == "fsr31" || currentBackend == "fsr31_12" && Config::Instance()->fsr3xVersionNames.size() > 0)
                        {
                            auto currentName = std::format("FSR {}", Config::Instance()->fsr3xVersionNames[_fsr3xIndex]);
                            if (ImGui::BeginCombo("Upscaler", currentName.c_str()))
                            {
                                for (int n = 0; n < Config::Instance()->fsr3xVersionIds.size(); n++)
                                {
                                    auto name = std::format("FSR {}", Config::Instance()->fsr3xVersionNames[n]);
                                    if (ImGui::Selectable(name.c_str(), Config::Instance()->Fsr3xIndex.value_or(0) == n))
                                        _fsr3xIndex = n;
                                }

                                ImGui::EndCombo();
                            }
                            ShowHelpMarker("List of upscalers reported by FFX SDK");

                            if (ImGui::Button("Change Upscaler") && _fsr3xIndex != Config::Instance()->Fsr3xIndex.value_or(0))
                            {
                                Config::Instance()->Fsr3xIndex = _fsr3xIndex;
                                Config::Instance()->newBackend = currentBackend;
                                Config::Instance()->changeBackend = true;
                            }

                            if (cf->Version().patch > 0)
                            {
                                float velocity = Config::Instance()->FsrVelocity.value_or(1.0);
                                if (ImGui::SliderFloat("Velocity Factor", &velocity, 0.00f, 1.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                                    Config::Instance()->FsrVelocity = velocity;

                                ShowHelpMarker("Lower values are more stable with ghosting\n"
                                               "Higher values are more pixelly but less ghosting.");
                            }
                        }

                        bool useVFov = Config::Instance()->FsrVerticalFov.has_value() || !Config::Instance()->FsrHorizontalFov.has_value();

                        float vfov;
                        float hfov;

                        vfov = Config::Instance()->FsrVerticalFov.value_or(60.0f);
                        hfov = Config::Instance()->FsrHorizontalFov.value_or(90.0f);

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

                        ImGui::BeginDisabled(!useVFov);

                        if (ImGui::SliderFloat("Vert. FOV", &vfov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrVerticalFov = vfov;

                        ImGui::EndDisabled();
                        ShowHelpMarker("Might help achieve better image quality");

                        ImGui::BeginDisabled(useVFov);

                        if (ImGui::SliderFloat("Horz. FOV", &hfov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrHorizontalFov = hfov;

                        ImGui::EndDisabled();
                        ShowHelpMarker("Might help achieve better image quality");

                        float cameraNear;
                        float cameraFar;

                        cameraNear = Config::Instance()->FsrCameraNear.value_or(0.01f);
                        cameraFar = Config::Instance()->FsrCameraFar.value_or(0.99f);

                        if (currentBackend == "fsr31" || currentBackend == "fsr31_12")
                        {
                            if (bool dView = Config::Instance()->FsrDebugView.value_or(false); ImGui::Checkbox("FSR 3.X Debug View", &dView))
                                Config::Instance()->FsrDebugView = dView;
                            ShowHelpMarker("Top left: Dilated Motion Vectors\n"
                                           "Top middle: Protected Areas\n"
                                           "Top right: Dilated Depth\n"
                                           "Middle: Upscaled frame\n"
                                           "Bottom left: Disocclusion mask\n"
                                           "Bottom middle: Reactiveness\n"
                                           "Bottom right: Detail Protection Takedown");
                        }

                        if (ImGui::SliderFloat("Camera Near", &cameraNear, 0.01f, 9.99f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrCameraNear = cameraNear;
                        ShowHelpMarker("Might help achieve better image quality\n"
                                       "And potentially less ghosting");

                        if (ImGui::SliderFloat("Camera Far", &cameraFar, 0.01f, 9.99f, "%.2f", ImGuiSliderFlags_NoRoundToFormat))
                            Config::Instance()->FsrCameraFar = cameraFar;
                        ShowHelpMarker("Might help achieve better image quality\n"
                                       "And potentially less ghosting");

                        if (Config::Instance()->CurrentFeature->AccessToReactiveMask())
                        {
                            ImGui::BeginDisabled(Config::Instance()->DisableReactiveMask.value_or(false));

                            auto useAsTransparency = Config::Instance()->FsrUseMaskForTransparency.value_or(true);
                            if (ImGui::Checkbox("Use Reactive Mask as Transparency Mask", &useAsTransparency))
                                Config::Instance()->FsrUseMaskForTransparency = useAsTransparency;

                            ImGui::EndDisabled();
                        }
                    }

                    // DLSS -----------------
                    if ((Config::Instance()->DLSSEnabled.value_or(true) && currentBackend == "dlss" && Config::Instance()->CurrentFeature->Version().major > 2) ||
                        Config::Instance()->CurrentFeature->Name() == "DLSSD")
                    {
                        ImGui::SeparatorText("DLSS Settings");

                        if (bool pOverride = Config::Instance()->RenderPresetOverride.value_or(false); ImGui::Checkbox("Render Presets Override", &pOverride))
                            Config::Instance()->RenderPresetOverride = pOverride;
                        ShowHelpMarker("Each render preset has it strengths and weaknesses\n"
                                       "Override to potentially improve image quality");

                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::Button("Apply Changes"))
                        {
                            if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                Config::Instance()->newBackend = "dlssd";
                            else
                                Config::Instance()->newBackend = currentBackend;

                            Config::Instance()->changeBackend = true;
                        }

                        AddRenderPreset("DLAA Preset", &Config::Instance()->RenderPresetDLAA);
                        AddRenderPreset("UltraQ Preset", &Config::Instance()->RenderPresetUltraQuality);
                        AddRenderPreset("Quality Preset", &Config::Instance()->RenderPresetQuality);
                        AddRenderPreset("Balanced Preset", &Config::Instance()->RenderPresetBalanced);
                        AddRenderPreset("Perf Preset", &Config::Instance()->RenderPresetPerformance);
                        AddRenderPreset("UltraP Preset", &Config::Instance()->RenderPresetUltraPerformance);
                    }

                    // RCAS -----------------
                    if (Config::Instance()->Api == NVNGX_DX12 || Config::Instance()->Api == NVNGX_DX11)
                    {
                        ImGui::SeparatorText("RCAS Settings");

                        // xess or dlss version >= 2.5.1
                        rcasEnabled = (currentBackend == "xess" || (currentBackend == "dlss" &&
                                       (Config::Instance()->CurrentFeature->Version().major > 2 ||
                                       (Config::Instance()->CurrentFeature->Version().major == 2 &&
                                       Config::Instance()->CurrentFeature->Version().minor >= 5 &&
                                       Config::Instance()->CurrentFeature->Version().patch >= 1))));

                        if (bool rcas = Config::Instance()->RcasEnabled.value_or(rcasEnabled); ImGui::Checkbox("Enable RCAS", &rcas))
                            Config::Instance()->RcasEnabled = rcas;
                        ShowHelpMarker("A sharpening filter\n"
                                       "By default uses a sharpening value provided by the game\n"
                                       "Select 'Override' under 'Sharpness' and adjust the slider to change it\n\n"
                                       "Some upscalers have it's own sharpness filter so RCAS is not always needed");

                        ImGui::BeginDisabled(!Config::Instance()->RcasEnabled.value_or(rcasEnabled));

                        if (bool overrideMotionSharpness = Config::Instance()->MotionSharpnessEnabled.value_or(false); ImGui::Checkbox("Motion Adaptive Sharpness", &overrideMotionSharpness))
                            Config::Instance()->MotionSharpnessEnabled = overrideMotionSharpness;
                        ImGui::EndDisabled();
                        ShowHelpMarker("Applies more sharpness to things in motion");

                        ImGui::BeginDisabled(!Config::Instance()->MotionSharpnessEnabled.value_or(false) || !Config::Instance()->RcasEnabled.value_or(rcasEnabled));

                        ImGui::SameLine(0.0f, 6.0f);

                        if (bool overrideMSDebug = Config::Instance()->MotionSharpnessDebug.value_or(false); ImGui::Checkbox("MAS Debug", &overrideMSDebug))
                            Config::Instance()->MotionSharpnessDebug = overrideMSDebug;
                        ImGui::EndDisabled();
                        ShowHelpMarker("Areas that are more red will have more sharpness applied\n"
                                       "Green areas will get reduced sharpness");
                        ImGui::BeginDisabled(!Config::Instance()->MotionSharpnessEnabled.value_or(false) || !Config::Instance()->RcasEnabled.value_or(rcasEnabled));

                        float motionSharpness = Config::Instance()->MotionSharpness.value_or(0.4f);
                        ImGui::SliderFloat("MotionSharpness", &motionSharpness, -1.3f, 1.3f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                        Config::Instance()->MotionSharpness = motionSharpness;

                        float motionThreshod = Config::Instance()->MotionThreshold.value_or(0.0f);
                        ImGui::SliderFloat("MotionThreshod", &motionThreshod, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                        Config::Instance()->MotionThreshold = motionThreshod;

                        float motionScale = Config::Instance()->MotionScaleLimit.value_or(10.0f);
                        ImGui::SliderFloat("MotionRange", &motionScale, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
                        Config::Instance()->MotionScaleLimit = motionScale;

                        ImGui::EndDisabled();
                    }

                    // DLSS Enabler -----------------
                    if (Config::Instance()->DE_Available)
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
                        ShowHelpMarker("Limit FPS to your monitor's refresh rate\nNot really vsync");

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

                        if (Config::Instance()->AdvancedSettings.value_or(false))
                        {
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

                    // OUTPUT SCALING -----------------------------
                    if (Config::Instance()->Api == NVNGX_DX12 || Config::Instance()->Api == NVNGX_DX11)
                    {
                        // if motion vectors are not display size
                        ImGui::BeginDisabled((Config::Instance()->DisplayResolution.has_value() && Config::Instance()->DisplayResolution.value()) ||
                                             (!Config::Instance()->DisplayResolution.has_value() && !(Config::Instance()->CurrentFeature->GetFeatureFlags() & NVSDK_NGX_DLSS_Feature_Flags_MVLowRes)));

                        ImGui::SeparatorText("Output Scaling");

                        float defaultRatio = 1.5f;

                        if (_ssRatio == 0.0f)
                        {
                            _ssRatio = Config::Instance()->OutputScalingMultiplier.value_or(defaultRatio);
                            _ssEnabled = Config::Instance()->OutputScalingEnabled.value_or(false);
                            _ssUseFsr = Config::Instance()->OutputScalingUseFsr.value_or(true);
                        }



                        ImGui::BeginDisabled((currentBackend == "xess" || currentBackend == "dlss") &&
                                             Config::Instance()->CurrentFeature->RenderWidth() > Config::Instance()->CurrentFeature->DisplayWidth());
                        ImGui::Checkbox("Enable", &_ssEnabled);
                        ImGui::EndDisabled();

                        ShowHelpMarker("Upscales the image internally to a selected resolution\n"
                                       "Then scales it to your resolution\n\n"
                                       "Values <1.0 might make upscaler less expensive\n"
                                       "Values >1.0 might make image sharper at the cost of performance\n\n"
                                       "You can see each step at the bottom of this menu");

                        ImGui::SameLine(0.0f, 6.0f);

                        ImGui::BeginDisabled(!_ssEnabled);
                        ImGui::Checkbox("Use FSR 1", &_ssUseFsr);
                        ImGui::EndDisabled();
                        ShowHelpMarker("Use FSR 1's upscaling pass instead of bicubic");

                        ImGui::SameLine(0.0f, 6.0f);

                        bool applyEnabled = _ssEnabled != Config::Instance()->OutputScalingEnabled.value_or(false) ||
                            _ssRatio != Config::Instance()->OutputScalingMultiplier.value_or(defaultRatio) ||
                            _ssUseFsr != Config::Instance()->OutputScalingUseFsr.value_or(true);

                        ImGui::BeginDisabled(!applyEnabled);
                        if (ImGui::Button("Apply Change"))
                        {
                            Config::Instance()->OutputScalingEnabled = _ssEnabled;
                            Config::Instance()->OutputScalingMultiplier = _ssRatio;
                            Config::Instance()->OutputScalingUseFsr = _ssUseFsr;

                            if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                Config::Instance()->newBackend = "dlssd";
                            else
                                Config::Instance()->newBackend = currentBackend;


                            Config::Instance()->changeBackend = true;
                        }
                        ImGui::EndDisabled();

                        ImGui::BeginDisabled(!_ssEnabled || Config::Instance()->CurrentFeature->RenderWidth() > Config::Instance()->CurrentFeature->DisplayWidth());
                        ImGui::SliderFloat("Ratio", &_ssRatio, 0.5f, 3.0f, "%.2f");
                        ImGui::EndDisabled();

                        if (cf != nullptr)
                        {
                            ImGui::Text("Output Scaling is %s, target res: %dx%d", Config::Instance()->OutputScalingEnabled.value_or(false) ? "ENABLED" : "DISABLED",
                                        (uint32_t)(cf->DisplayWidth() * _ssRatio), (uint32_t)(cf->DisplayHeight() * _ssRatio));
                        }

                        ImGui::EndDisabled();
                    }
                }

                // DX11 & DX12 -----------------------------
                if ((cf == nullptr || Config::Instance()->AdvancedSettings.value_or(false)) && Config::Instance()->Api != NVNGX_VULKAN)
                {
                    // MIPMAP BIAS & Anisotropy -----------------------------
                    ImGui::SeparatorText("Mipmap Bias (DirectX)");

                    if (Config::Instance()->MipmapBiasOverride.has_value() && _mipBias == 0.0f)
                        _mipBias = Config::Instance()->MipmapBiasOverride.value();

                    ImGui::SliderFloat("Mipmap Bias", &_mipBias, -15.0f, 15.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);
                    ShowHelpMarker("Can help with blurry textures in broken games\n"
                                   "Negative values will make textures sharper\n"
                                   "Positive values will make textures more blurry\n\n"
                                   "Has a small performance impact");


                    ImGui::BeginDisabled(Config::Instance()->MipmapBiasOverride.has_value() && Config::Instance()->MipmapBiasOverride.value() == _mipBias);
                    if (ImGui::Button("Set"))
                        Config::Instance()->MipmapBiasOverride = _mipBias;
                    ImGui::EndDisabled();

                    ImGui::SameLine(0.0f, 6.0f);

                    ImGui::BeginDisabled(!Config::Instance()->MipmapBiasOverride.has_value());
                    if (ImGui::Button("Reset"))
                    {
                        Config::Instance()->MipmapBiasOverride.reset();
                        _mipBias = 0.0f;
                    }
                    ImGui::EndDisabled();

                    if (cf != nullptr)
                    {
                        ImGui::SameLine(0.0f, 6.0f);

                        if (ImGui::Button("Calculate Mipmap Bias"))
                            _showMipmapCalcWindow = true;
                    }

                    if (Config::Instance()->MipmapBiasOverride.has_value())
                        ImGui::Text("Current : %.6f, Target: %.6f", Config::Instance()->lastMipBias, Config::Instance()->MipmapBiasOverride.value());
                    else
                        ImGui::Text("Current : %.6f", Config::Instance()->lastMipBias);

                    ImGui::Text("Will be applied after RESOLUTION or PRESENT change !!!");

                    // MIPMAP BIAS & Anisotropy -----------------------------
                    ImGui::SeparatorText("Anisotropic Filtering (DirectX)");

                    ImGui::PushItemWidth(65.0f * Config::Instance()->MenuScale.value_or(1.0));

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

                    ImGui::Text("Will be applied after RESOLUTION or PRESENT change !!!");

                    if (cf != nullptr)
                    {
                        // Non-DLSS hotfixes -----------------------------
                        if (currentBackend != "dlss")
                        {
                            // BARRIERS -----------------------------
                            ImGui::SeparatorText("Resource Barriers (Dx12)");

                            AddResourceBarrier("Color", &Config::Instance()->ColorResourceBarrier);
                            AddResourceBarrier("Depth", &Config::Instance()->DepthResourceBarrier);
                            AddResourceBarrier("Motion", &Config::Instance()->MVResourceBarrier);
                            AddResourceBarrier("Exposure", &Config::Instance()->ExposureResourceBarrier);
                            AddResourceBarrier("Mask", &Config::Instance()->MaskResourceBarrier);
                            AddResourceBarrier("Output", &Config::Instance()->OutputResourceBarrier);

                            // HOTFIXES -----------------------------
                            ImGui::SeparatorText("Root Signatures (Dx12)");

                            if (bool crs = Config::Instance()->RestoreComputeSignature.value_or(false); ImGui::Checkbox("Restore Compute Root Signature", &crs))
                                Config::Instance()->RestoreComputeSignature = crs;

                            if (bool grs = Config::Instance()->RestoreGraphicSignature.value_or(false); ImGui::Checkbox("Restore Graphic Root Signature", &grs))
                                Config::Instance()->RestoreGraphicSignature = grs;
                        }
                    }
                }

                // NEXT COLUMN -----------------
                ImGui::TableNextColumn();

                if (cf != nullptr)
                {
                    // SHARPNESS -----------------------------
                    ImGui::SeparatorText("Sharpness");

                    if (bool overrideSharpness = Config::Instance()->OverrideSharpness.value_or(false); ImGui::Checkbox("Override", &overrideSharpness))
                    {
                        Config::Instance()->OverrideSharpness = overrideSharpness;

                        if (currentBackend == "dlss" && Config::Instance()->CurrentFeature->Version().major < 3)
                        {
                            Config::Instance()->newBackend = currentBackend;
                            Config::Instance()->changeBackend = true;
                        }
                    }
                    ShowHelpMarker("Ignores the value sent by the game\n"
                                   "and uses the value set below");

                    ImGui::BeginDisabled(!Config::Instance()->OverrideSharpness.value_or(false));

                    float sharpness = Config::Instance()->Sharpness.value_or(0.3f);
                    ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, Config::Instance()->RcasEnabled.value_or(rcasEnabled) ? 1.3f : 1.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                    Config::Instance()->Sharpness = sharpness;

                    ImGui::EndDisabled();

                    // UPSCALE RATIO OVERRIDE -----------------

                    auto minSliderLimit = Config::Instance()->ExtendedLimits.value_or(false) ? 0.1f : 1.0f;
                    auto maxSliderLimit = Config::Instance()->ExtendedLimits.value_or(false) ? 6.0f : 3.0f;

                    ImGui::SeparatorText("Upscale Ratio");
                    if (bool upOverride = Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false); ImGui::Checkbox("Ratio Override", &upOverride))
                        Config::Instance()->UpscaleRatioOverrideEnabled = upOverride;
                    ShowHelpMarker("Let's you override every upscaler preset\n"
                                   "with a value set below\n\n"
                                   "1.5x on a 1080p screen means internal resolution of 720p\n"
                                   "1080 / 1.5 = 720");

                    ImGui::BeginDisabled(!Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false));

                    float urOverride = Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f);
                    ImGui::SliderFloat("Override Ratio", &urOverride, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
                    Config::Instance()->UpscaleRatioOverrideValue = urOverride;

                    ImGui::EndDisabled();

                    // QUALITY OVERRIDES -----------------------------
                    ImGui::SeparatorText("Quality Overrides");

                    if (bool qOverride = Config::Instance()->QualityRatioOverrideEnabled.value_or(false); ImGui::Checkbox("Quality Override", &qOverride))
                        Config::Instance()->QualityRatioOverrideEnabled = qOverride;
                    ShowHelpMarker("Let's you override each preset's ratio individually\n"
                                   "Note that not every game supports every quality preset\n\n"
                                   "1.5x on a 1080p screen means internal resolution of 720p\n"
                                   "1080 / 1.5 = 720");

                    ImGui::BeginDisabled(!Config::Instance()->QualityRatioOverrideEnabled.value_or(false));

                    float qDlaa = Config::Instance()->QualityRatio_DLAA.value_or(1.0f);
                    if (ImGui::SliderFloat("DLAA", &qDlaa, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_DLAA = qDlaa;

                    float qUq = Config::Instance()->QualityRatio_UltraQuality.value_or(1.3f);
                    if (ImGui::SliderFloat("Ultra Quality", &qUq, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_UltraQuality = qUq;

                    float qQ = Config::Instance()->QualityRatio_Quality.value_or(1.5f);
                    if (ImGui::SliderFloat("Quality", &qQ, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Quality = qQ;

                    float qB = Config::Instance()->QualityRatio_Balanced.value_or(1.7f);
                    if (ImGui::SliderFloat("Balanced", &qB, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Balanced = qB;

                    float qP = Config::Instance()->QualityRatio_Performance.value_or(2.0f);
                    if (ImGui::SliderFloat("Performance", &qP, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_Performance = qP;

                    float qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0f);
                    if (ImGui::SliderFloat("Ultra Performance", &qUp, minSliderLimit, maxSliderLimit, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
                        Config::Instance()->QualityRatio_UltraPerformance = qUp;

                    ImGui::EndDisabled();

                    // DRS -----------------------------
                    ImGui::SeparatorText("DRS (Dynamic Resolution Scaling)");
                    if (ImGui::BeginTable("drs", 2))
                    {
                        ImGui::TableNextColumn();
                        if (bool drsMin = Config::Instance()->DrsMinOverrideEnabled.value_or(false); ImGui::Checkbox("Override Minimum", &drsMin))
                            Config::Instance()->DrsMinOverrideEnabled = drsMin;
                        ShowHelpMarker("Fix for games ignoring official DRS limits");

                        ImGui::TableNextColumn();
                        if (bool drsMax = Config::Instance()->DrsMaxOverrideEnabled.value_or(false); ImGui::Checkbox("Override Maximum", &drsMax))
                            Config::Instance()->DrsMaxOverrideEnabled = drsMax;
                        ShowHelpMarker("Fix for games ignoring official DRS limits");

                        ImGui::EndTable();
                    }

                    // INIT -----------------------------
                    ImGui::SeparatorText("Init Flags");
                    if (ImGui::BeginTable("init", 2))
                    {
                        ImGui::TableNextColumn();
                        if (bool autoExposure = Config::Instance()->AutoExposure.value_or(false); ImGui::Checkbox("Auto Exposure", &autoExposure))
                        {
                            Config::Instance()->AutoExposure = autoExposure;

                            if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                Config::Instance()->newBackend = "dlssd";
                            else
                                Config::Instance()->newBackend = currentBackend;

                            Config::Instance()->changeBackend = true;
                        }
                        ShowHelpMarker("Some Unreal Engine games need this\n"
                                       "Might fix colors, especially in dark areas");

                        ImGui::TableNextColumn();
                        if (bool hdr = Config::Instance()->HDR.value_or(false); ImGui::Checkbox("HDR", &hdr))
                        {
                            Config::Instance()->HDR = hdr;

                            if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                Config::Instance()->newBackend = "dlssd";
                            else
                                Config::Instance()->newBackend = currentBackend;

                            Config::Instance()->changeBackend = true;
                        }
                        ShowHelpMarker("Might help with purple hue in some games");

                        ImGui::TableNextColumn();

                        if (Config::Instance()->AdvancedSettings.value_or(false))
                        {
                            if (bool depth = Config::Instance()->DepthInverted.value_or(false); ImGui::Checkbox("Depth Inverted", &depth))
                            {
                                Config::Instance()->DepthInverted = depth;

                                if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                    Config::Instance()->newBackend = "dlssd";
                                else
                                    Config::Instance()->newBackend = currentBackend;

                                Config::Instance()->changeBackend = true;
                            }
                            ShowHelpMarker("You shouldn't need to change it");

                            ImGui::TableNextColumn();
                            if (bool jitter = Config::Instance()->JitterCancellation.value_or(false); ImGui::Checkbox("Jitter Cancellation", &jitter))
                            {
                                Config::Instance()->JitterCancellation = jitter;

                                if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                    Config::Instance()->newBackend = "dlssd";
                                else
                                    Config::Instance()->newBackend = currentBackend;

                                Config::Instance()->changeBackend = true;
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

                                if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
                                    Config::Instance()->newBackend = "dlssd";
                                else
                                    Config::Instance()->newBackend = currentBackend;

                                Config::Instance()->changeBackend = true;
                            }
                            ShowHelpMarker("Mostly a fix for Unreal Engine games\n"
                                           "Top left part of the screen will be blurry");

                            ImGui::TableNextColumn();
                            auto accessToReactiveMask = Config::Instance()->CurrentFeature->AccessToReactiveMask();
                            ImGui::BeginDisabled(!accessToReactiveMask);

                            bool rm = Config::Instance()->DisableReactiveMask.value_or(!accessToReactiveMask || currentBackend == "dlss" || currentBackend == "xess");
                            if (ImGui::Checkbox("Disable Reactive Mask", &rm))
                            {
                                Config::Instance()->DisableReactiveMask = rm;

                                if (currentBackend == "xess")
                                {
                                    Config::Instance()->newBackend = currentBackend;
                                    Config::Instance()->changeBackend = true;
                                }
                            }

                            ImGui::EndDisabled();

                            if (accessToReactiveMask)
                                ShowHelpMarker("Allows the use of a reactive mask\n"
                                               "Keep in mind that a reactive mask sent to DLSS\n"
                                               "will not produce a good image in combination with FSR/XeSS");
                            else
                                ShowHelpMarker("Option disabled because tha game doesn't provide a reactive mask");
                        }

                        ImGui::EndTable();

                        if (Config::Instance()->CurrentFeature->AccessToReactiveMask() && currentBackend != "dlss")
                        {
                            ImGui::BeginDisabled(Config::Instance()->DisableReactiveMask.value_or(currentBackend == "xess"));

                            bool binaryMask = Config::Instance()->Api == NVNGX_VULKAN || currentBackend == "xess";
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

                        if (Config::Instance()->AdvancedSettings.value_or(false))
                        {
                            bool pcShaders = Config::Instance()->UsePrecompiledShaders.value_or(true);

                            if (ImGui::Checkbox("Use Precompiled Shaders", &pcShaders))
                            {
                                Config::Instance()->UsePrecompiledShaders = pcShaders;
                                Config::Instance()->newBackend = currentBackend;
                                Config::Instance()->changeBackend = true;
                            }
                        }

                        if (Config::Instance()->AdvancedSettings.value_or(false) && currentBackend == "dlss")
                        {
                            bool appIdOverride = Config::Instance()->UseGenericAppIdWithDlss.value_or(false);

                            if (ImGui::Checkbox("Use Generic App Id with DLSS", &appIdOverride))
                            {
                                Config::Instance()->UseGenericAppIdWithDlss = appIdOverride;
                            }

                            ShowHelpMarker("Use generic appid with NGX\n"
                                           "Fixes OptiScaler preset override not working with certain games\n"
                                           "Requires a game restart.");
                        }
                    }
                }

                // FAKENVAPI ---------------------------
                if (Config::Instance()->FN_Available)
                {
                    ImGui::SeparatorText("fakenvapi");

                    if (bool logs = Config::Instance()->FN_EnableLogs.value_or(true); ImGui::Checkbox("Enable Logs", &logs))
                        Config::Instance()->FN_EnableLogs = logs;

                    ImGui::SameLine(0.0f, 6.0f);
                    if (bool traceLogs = Config::Instance()->FN_EnableTraceLogs.value_or(true); ImGui::Checkbox("Enable Trace Logs", &traceLogs))
                        Config::Instance()->FN_EnableTraceLogs = traceLogs;

                    if (bool forceLFX = Config::Instance()->FN_ForceLatencyFlex.value_or(true); ImGui::Checkbox("Force LatencyFlex", &forceLFX))
                        Config::Instance()->FN_ForceLatencyFlex = forceLFX;
                    ShowHelpMarker("When possible AntiLag 2 is used, this setting let's you force LatencyFlex instead");

                    const char* lfx_modes[] = { "Conservative", "Aggressive", "Reflex ID" };
                    const char* lfx_modesDesc[] = { "The safest but might not reduce latency well",
                        "Improves latency but in some cases will lower fps more than expected",
                        "Best when can be used, some games are not compatible (i.e. cyberpunk) and will fallback to aggressive"
                    };

                    PopulateCombo("LatencyFlex mode", &Config::Instance()->FN_LatencyFlexMode, lfx_modes, lfx_modesDesc, 3);

                    const char* rfx_modes[] = { "Follow in-game", "Force Disable", "Force Enable" };
                    const char* rfx_modesDesc[] = { "", "", "" };

                    PopulateCombo("Force Reflex", &Config::Instance()->FN_ForceReflex, rfx_modes, rfx_modesDesc, 3);

                    if (ImGui::Button("Apply##2"))
                    {
                        Config::Instance()->SaveFakenvapiIni();
                    }
                }


                // LOGGING -----------------------------
                ImGui::SeparatorText("Logging");

                if (Config::Instance()->LogToConsole.value_or(false) || Config::Instance()->LogToFile.value_or(false) || Config::Instance()->LogToNGX.value_or(false))
                    spdlog::default_logger()->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));
                else
                    spdlog::default_logger()->set_level(spdlog::level::off);

                if (bool toFile = Config::Instance()->LogToFile.value_or(false); ImGui::Checkbox("To File", &toFile))
                {
                    Config::Instance()->LogToFile = toFile;
                    PrepareLogger();
                }

                ImGui::SameLine(0.0f, 6.0f);
                if (bool toConsole = Config::Instance()->LogToConsole.value_or(false); ImGui::Checkbox("To Console", &toConsole))
                {
                    Config::Instance()->LogToConsole = toConsole;
                    PrepareLogger();
                }

                const char* logLevels[] = { "Trace", "Debug", "Information", "Warning", "Error" };
                const char* selectedLevel = logLevels[Config::Instance()->LogLevel.value_or(2)];

                if (ImGui::BeginCombo("Log Level", selectedLevel))
                {
                    for (int n = 0; n < 5; n++)
                    {
                        if (ImGui::Selectable(logLevels[n], (Config::Instance()->LogLevel.value_or(2) == n)))
                        {
                            Config::Instance()->LogLevel = n;
                            spdlog::default_logger()->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));
                        }
                    }

                    ImGui::EndCombo();
                }

                if (cf != nullptr)
                {
                    // ADVANCED SETTINGS -----------------------------
                    ImGui::SeparatorText("Advanced Settings");

                    bool advancedSettings = Config::Instance()->AdvancedSettings.value_or(false);
                    if (ImGui::Checkbox("Show Advanced Settings", &advancedSettings))
                        Config::Instance()->AdvancedSettings = advancedSettings;

                    if (advancedSettings)
                    {
                        bool extendedLimits = Config::Instance()->ExtendedLimits.value_or(false);
                        if (ImGui::Checkbox("Enable Extended Limits", &extendedLimits))
                            Config::Instance()->ExtendedLimits = extendedLimits;

                        ShowHelpMarker("Extended sliders limit for quality presets\n\n"
                                       "Using this option changes resolution detection logic\n"
                                       "and might cause issues and crashes!");
                    }
                }

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTable("plots", 2))
            {
                Config::Instance()->frameTimes.pop_front();
                Config::Instance()->frameTimes.push_back(1000.0 / io.Framerate);

                ImGui::TableNextColumn();
                ImGui::Text("FrameTime");
                auto ft = std::format("{:.2f} ms / {:.1f} fps", Config::Instance()->frameTimes.back(), io.Framerate);
                std::vector<float> frameTimeArray(Config::Instance()->frameTimes.begin(), Config::Instance()->frameTimes.end());
                ImGui::PlotLines(ft.c_str(), frameTimeArray.data(), frameTimeArray.size());

                if (cf != nullptr)
                {
                    ImGui::TableNextColumn();
                    ImGui::Text("Upscaler (GPU time)");
                    auto ups = std::format("{:.2f} ms", Config::Instance()->upscaleTimes.back());
                    std::vector<float> upscaleTimeArray(Config::Instance()->upscaleTimes.begin(), Config::Instance()->upscaleTimes.end());
                    ImGui::PlotLines(ups.c_str(), upscaleTimeArray.data(), upscaleTimeArray.size());
                }

                ImGui::EndTable();
            }

            // BOTTOM LINE ---------------
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (cf != nullptr)
            {
                ImGui::Text("%dx%d -> %dx%d (%.1f) [%dx%d (%.1f)]",
                            cf->RenderWidth(), cf->RenderHeight(), cf->TargetWidth(), cf->TargetHeight(), (float)cf->TargetWidth() / (float)cf->RenderWidth(),
                            cf->DisplayWidth(), cf->DisplayHeight(), (float)cf->DisplayWidth() / (float)cf->RenderWidth());

                ImGui::SameLine(0.0f, 4.0f);

                ImGui::Text("%d", Config::Instance()->CurrentFeature->FrameCount());

                ImGui::SameLine(0.0f, 10.0f);
            }

            ImGui::PushItemWidth(45.0f * Config::Instance()->MenuScale.value_or(1.0));

            const char* uiScales[] = { "1.0", "1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7", "1.8", "1.9", "2.0" };
            const char* selectedScaleName = uiScales[_selectedScale];

            if (ImGui::BeginCombo("UI Scale", selectedScaleName))
            {
                for (int n = 0; n < 11; n++)
                {
                    if (ImGui::Selectable(uiScales[n], (_selectedScale == n)))
                    {
                        _selectedScale = n;
                        Config::Instance()->MenuScale = 1.0f + ((float)n / 10.0f);
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

            ImGui::End();
        }

        if (_showMipmapCalcWindow && cf != nullptr)
        {
            auto posX = (Config::Instance()->CurrentFeature->DisplayWidth() - 450.0f) / 2.0f;
            auto posY = (Config::Instance()->CurrentFeature->DisplayHeight() - 200.0f) / 2.0f;

            ImGui::SetNextWindowPos(ImVec2{ posX , posY }, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2{ 450.0f, 200.0f }, ImGuiCond_FirstUseEver);

            if (_displayWidth == 0)
            {
                if (Config::Instance()->OutputScalingEnabled.value_or(false))
                    _displayWidth = Config::Instance()->CurrentFeature->DisplayWidth() * Config::Instance()->OutputScalingMultiplier.value_or(1.5f);
                else
                    _displayWidth = Config::Instance()->CurrentFeature->DisplayWidth();

                _renderWidth = _displayWidth / 3.0f;
                _mipmapUpscalerQuality = 0;
                _mipmapUpscalerRatio = 3.0f;
                _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
            }

            if (ImGui::Begin("Mipmap Bias", nullptr, flags))
            {
                if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
                    ImGui::SetWindowFocus();

                ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));

                if (ImGui::InputScalar("Display Width", ImGuiDataType_U32, &_displayWidth, NULL, NULL, "%u"))
                {
                    if (_displayWidth <= 0)
                    {
                        if (Config::Instance()->OutputScalingEnabled.value_or(false))
                            _displayWidth = Config::Instance()->CurrentFeature->DisplayWidth() * Config::Instance()->OutputScalingMultiplier.value_or(1.5f);
                        else
                            _displayWidth = Config::Instance()->CurrentFeature->DisplayWidth();

                    }

                    _renderWidth = _displayWidth / _mipmapUpscalerRatio;
                    _mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
                }

                const char* q[] = { "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality", "DLAA" };
                float fr[] = { 3.0f, 2.0f, 1.7f, 1.5f, 1.3f, 1.0f };
                auto configQ = _mipmapUpscalerQuality;

                const char* selectedQ = q[configQ];

                ImGui::BeginDisabled(Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false));

                if (ImGui::BeginCombo("Upscaler Quality", selectedQ))
                {
                    for (int n = 0; n < 6; n++)
                    {
                        if (ImGui::Selectable(q[n], (_mipmapUpscalerQuality == n)))
                        {
                            _mipmapUpscalerQuality = n;

                            float ov = -1.0f;

                            if (Config::Instance()->QualityRatioOverrideEnabled.value_or(false))
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
                    Config::Instance()->ExtendedLimits.value_or(false) ? 0.1f : 1.0f,
                    Config::Instance()->ExtendedLimits.value_or(false) ? 6.0f : 3.0f,
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
}

void ImGuiCommon::Init(HWND InHwnd)
{
    _handle = InHwnd;
    _isVisible = false;
    _isResetRequested = false;

    LOG_DEBUG("Handle: {0:X}", (ULONG64)_handle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    io.MouseDrawCursor = _isVisible;
    io.WantCaptureKeyboard = _isVisible;
    io.WantCaptureMouse = _isVisible;
    io.WantSetMousePos = _isVisible;

    io.IniFilename = io.LogFilename = nullptr;

    bool initResult = ImGui_ImplWin32_Init(InHwnd);
    LOG_DEBUG("ImGui_ImplWin32_Init result: {0}", initResult);

    if (_oWndProc == nullptr)
        _oWndProc = (WNDPROC)SetWindowLongPtr(InHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

    LOG_DEBUG("_oWndProc: {0:X}", (ULONG64)_oWndProc);

    if (!pfn_SetCursorPos_hooked)
        AttachHooks();

    if (Config::Instance()->MenuScale.value_or(1.0f) < 1.0f)
        Config::Instance()->MenuScale = 1.0f;

    if (Config::Instance()->MenuScale.value_or(1.0f) > 2.0f)
        Config::Instance()->MenuScale = 2.0f;

    _selectedScale = (int)((Config::Instance()->MenuScale.value_or(1.0f) - 1.0f) / 0.1f);
    _isInited = true;

    if (skHandle == nullptr && Config::Instance()->LoadSpecialK.value_or(false))
    {
        skHandle = LoadLibrary(L"SpecialK64.dll");
        LOG_INFO("Loading SpecialK64.dll, result: {0:X}", (UINT64)skHandle);
    }
}

void ImGuiCommon::Shutdown()
{
    if (!ImGuiCommon::_isInited)
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
    _isResetRequested = false;
}

void ImGuiCommon::HideMenu()
{
    if (!_isVisible)
        return;

    _isVisible = false;

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (pfn_ClipCursor_hooked && _lastCursorLimit != nullptr)
        pfn_ClipCursor(_lastCursorLimit);

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
