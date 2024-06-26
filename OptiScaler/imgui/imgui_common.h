#pragma once

#include "../pch.h"
#include "../Config.h"
#include "../Resource.h"
#include "../Logger.h"
#include "../NVNGX_Proxy.h"

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
	inline static bool _isResetRequested = false;

	// mipmap calculations
	inline static bool _showMipmapCalcWindow = false;
	inline static float _mipBias = 0.0f;
	inline static float _mipBiasCalculated = 0.0f;
	inline static uint32_t _mipmapUpscalerQuality = 0;
	inline static float _mipmapUpscalerRatio = 0;
	inline static uint32_t _displayWidth = 0;
	inline static uint32_t _renderWidth = 0;

	// dlss enabler
	inline static int _deLimitFps = 0;

	// output scaling
	inline static float _ssRatio = 0.0f;
	inline static bool _ssEnabled = false;

	// ui scale
	inline static int _selectedScale = 0;
	inline static bool _imguiSizeUpdate = true;

	inline static bool _dx11Ready = false;
	inline static bool _dx12Ready = false;
	inline static bool _vulkanReady = false;

	inline static ImGuiKey ImGui_ImplWin32_VirtualKeyToImGuiKey(WPARAM wParam)
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

#pragma region "Hooks & WndProc"

	// for hooking
	typedef BOOL(WINAPI* PFN_SetCursorPos)(int x, int y);
	typedef BOOL(WINAPI* PFN_ClipCursor)(const RECT* lpRect);
	typedef UINT(WINAPI* PFN_SendInput)(UINT cInputs, LPINPUT pInputs, int cbSize);
	typedef void(WINAPI* PFN_mouse_event)(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
	typedef LRESULT(WINAPI* PFN_SendMessageW)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	inline static PFN_SetCursorPos pfn_SetPhysicalCursorPos = nullptr;
	inline static PFN_SetCursorPos pfn_SetCursorPos = nullptr;
	inline static PFN_ClipCursor pfn_ClipCursor = nullptr;
	inline static PFN_mouse_event pfn_mouse_event = nullptr;
	inline static PFN_SendInput pfn_SendInput = nullptr;
	inline static PFN_SendMessageW pfn_SendMessageW = nullptr;

	inline static bool pfn_SetPhysicalCursorPos_hooked = false;
	inline static bool pfn_SetCursorPos_hooked = false;
	inline static bool pfn_ClipCursor_hooked = false;
	inline static bool pfn_mouse_event_hooked = false;
	inline static bool pfn_SendInput_hooked = false;
	inline static bool pfn_SendMessageW_hooked = false;

	inline static RECT _cursorLimit{};
	inline static LPRECT _lastCursorLimit = nullptr;

	static LRESULT WINAPI hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		if (_isVisible && Msg == 0x0020)
			return TRUE;
		else
			return pfn_SendMessageW(hWnd, Msg, wParam, lParam);
	}

	static BOOL WINAPI hkSetPhysicalCursorPos(int x, int y)
	{
		if (_isVisible)
			return TRUE;
		else
			return pfn_SetPhysicalCursorPos(x, y);
	}

	static BOOL WINAPI hkSetCursorPos(int x, int y)
	{
		if (_isVisible)
			return TRUE;
		else
			return pfn_SetCursorPos(x, y);
	}

	static BOOL WINAPI hkClipCursor(RECT* lpRect)
	{
		if (_isVisible)
			return TRUE;
		else
		{
			_lastCursorLimit = lpRect;
			return pfn_ClipCursor(lpRect);
		}
	}

	static void WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
	{
		if (_isVisible)
			return;
		else
			pfn_mouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
	}

	static UINT WINAPI hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
	{
		if (_isVisible)
			return TRUE;
		else
			return pfn_SendInput(cInputs, pInputs, cbSize);
	}

	static void AttachHooks()
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

	static void DetachHooks()
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

	//Win32 message handler
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if (Config::Instance()->CurrentFeature == nullptr || _isResetRequested || (!_dx11Ready && !_dx12Ready && !_vulkanReady))
		{
			if (_isVisible)
			{
				spdlog::info("WndProc No active features, closing ImGui");

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
				vmInputReset = rawData.data.keyboard.VKey == Config::Instance()->ResetKey.value_or(VK_END);
			}
		}


		// END - REINIT MENU
		if (msg == WM_KEYDOWN && wParam == Config::Instance()->ResetKey.value_or(VK_END))
		{
			_isResetRequested = true;
			return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
		}

		// INSERT - OPEN MENU
		if (msg == WM_KEYDOWN && wParam == Config::Instance()->ShortcutKey.value_or(VK_INSERT))
		{
			_isVisible = !_isVisible;

			if (!_isVisible)
			{
				if (pfn_ClipCursor_hooked && _lastCursorLimit != nullptr)
					pfn_ClipCursor(_lastCursorLimit);

				_showMipmapCalcWindow = false;
			}
			else if (pfn_ClipCursor_hooked)
			{
				if (_lastCursorLimit != nullptr)
					pfn_ClipCursor(nullptr);

				//if (GetWindowRect(_handle, &_cursorLimit))
				//	pfn_ClipCursor(&_cursorLimit);
				//else
				//	pfn_ClipCursor(nullptr);
			}

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

			spdlog::trace("WndProc Menu key pressed, {0}", _isVisible ? "opening ImGui" : "closing ImGui");

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
				spdlog::trace("WndProc ImGui handled, hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);
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
				spdlog::trace("WndProc switch handled, hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);
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
					spdlog::trace("WndProc WM_INPUT hWnd:{0:X} msg:{1:X} wParam:{2:X} lParam:{3:X}", (ULONG64)hWnd, msg, (ULONG64)wParam, (ULONG64)lParam);

				return TRUE;

			default:
				break;
			}
		}

		return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
	}

#pragma endregion

public:

	static void Dx11Inited() { _dx11Ready = true; }
	static void Dx12Inited() { _dx12Ready = true; }
	static void VulkanInited() { _vulkanReady = true; }
	static bool IsInited() { return _isInited; }
	static bool IsVisible() { return _isVisible; }
	static bool IsResetRequested() { return _isResetRequested; }
	static HWND Handle() { return _handle; }

	static std::string GetBackendName(std::string* code)
	{
		if (*code == "fsr21")
			return "FSR 2.1.2";

		if (*code == "fsr22")
			return "FSR 2.2.1";

		if (*code == "fsr21_12")
			return "FSR 2.1.2 w/Dx12";

		if (*code == "fsr22_12")
			return "FSR 2.2.1 w/Dx12";

		if (*code == "xess")
			return "XeSS";

		if (*code == "dlss")
			return "DLSS";

		return "????";
	}

	static std::string GetBackendCode(const NVNGX_Api api)
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

	static void GetCurrentBackendInfo(const NVNGX_Api api, std::string* code, std::string* name)
	{
		*code = GetBackendCode(api);
		*name = GetBackendName(code);
	}

	static void AddDx11Backends(std::string* code, std::string* name)
	{
		std::string selectedUpscalerName = "";

		if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
			selectedUpscalerName = "FSR 2.2.1";
		else if (Config::Instance()->newBackend == "fsr22_12" || (Config::Instance()->newBackend == "" && *code == "fsr22_12"))
			selectedUpscalerName = "FSR 2.2.1 w/Dx12";
		else if (Config::Instance()->newBackend == "fsr21_12" || (Config::Instance()->newBackend == "" && *code == "fsr21_12"))
			selectedUpscalerName = "FSR 2.1.2 w/Dx12";
		else if (NVNGXProxy::IsNVNGXInited() && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
			selectedUpscalerName = "DLSS";
		else
			selectedUpscalerName = "XeSS w/Dx12";

		if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
		{
			if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
				Config::Instance()->newBackend = "fsr22";

			if (ImGui::Selectable("XeSS w/Dx12", *code == "xess"))
				Config::Instance()->newBackend = "xess";

			if (ImGui::Selectable("FSR 2.1.2 w/Dx12", *code == "fsr21_12"))
				Config::Instance()->newBackend = "fsr21_12";

			if (ImGui::Selectable("FSR 2.2.1 w/Dx12", *code == "fsr22_12"))
				Config::Instance()->newBackend = "fsr22_12";

			if (NVNGXProxy::IsNVNGXInited() && ImGui::Selectable("DLSS", *code == "dlss"))
				Config::Instance()->newBackend = "dlss";

			ImGui::EndCombo();
		}
	}

	static void AddDx12Backends(std::string* code, std::string* name)
	{
		std::string selectedUpscalerName = "";

		if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
			selectedUpscalerName = "FSR 2.1.2";
		else if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
			selectedUpscalerName = "FSR 2.2.1";
		else if (NVNGXProxy::IsNVNGXInited() && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
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

			if (NVNGXProxy::IsNVNGXInited() && ImGui::Selectable("DLSS", *code == "dlss"))
				Config::Instance()->newBackend = "dlss";

			ImGui::EndCombo();
		}
	}

	static void AddVulkanBackends(std::string* code, std::string* name)
	{
		std::string selectedUpscalerName = "";

		if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
			selectedUpscalerName = "FSR 2.1.2";
		else if (NVNGXProxy::IsNVNGXInited() && (Config::Instance()->newBackend == "dlss" || (Config::Instance()->newBackend == "" && *code == "dlss")))
			selectedUpscalerName = "DLSS";
		else
			selectedUpscalerName = "FSR 2.2.1";

		if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
		{
			if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
				Config::Instance()->newBackend = "fsr21";

			if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
				Config::Instance()->newBackend = "fsr22";

			if (Config::Instance()->DLSSEnabled.value_or(true) && ImGui::Selectable("DLSS", *code == "dlss"))
				Config::Instance()->newBackend = "dlss";

			ImGui::EndCombo();
		}
	}

	static void AddResourceBarrier(std::string name, std::optional<int>* value)
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

	static void AddRenderPreset(std::string name, std::optional<int>* value)
	{
		const char* presets[] = { "DEFAULT", "PRESET A", "PRESET B", "PRESET C", "PRESET D", "PRESET E", "PRESET F", "PRESET G" };

		int selected = value->value_or(0);

		const char* selectedName = "";

		for (int n = 0; n < 8; n++)
		{
			if (n == selected)
			{
				selectedName = presets[n];
				break;
			}
		}

		if (ImGui::BeginCombo(name.c_str(), selectedName))
		{
			if (ImGui::Selectable(presets[0], !value->has_value()))
				value->reset();

			for (int n = 1; n < 8; n++)
			{
				if (ImGui::Selectable(presets[n], selected == n))
				{
					if (n != selected)
						*value = n;
				}
			}

			ImGui::EndCombo();
		}
	}

	static void RenderMenu()
	{
		if (!_isInited || !_isVisible || Config::Instance()->CurrentFeature == nullptr)
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

			auto posX = ((float)Config::Instance()->CurrentFeature->DisplayWidth() - 770.0f) / 2.0f;
			auto posY = ((float)Config::Instance()->CurrentFeature->DisplayHeight() - 685.0f) / 2.0f;

			ImGui::SetNextWindowPos(ImVec2{ posX, posY }, ImGuiCond_FirstUseEver);

			if (ImGui::Begin(VER_PRODUCT_NAME, NULL, flags))
			{
				if (!_showMipmapCalcWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
					ImGui::SetWindowFocus();

				ImGui::SetWindowFontScale(Config::Instance()->MenuScale.value_or(1.0));

				if (ImGui::BeginTable("main", 2))
				{
					ImGui::TableNextColumn();

					// UPSCALERS -----------------------------
					ImGui::SeparatorText("Upscalers");

					std::string selectedUpscalerName = "";
					std::string currentBackend;
					std::string currentBackendName;

					GetCurrentBackendInfo(Config::Instance()->Api, &currentBackend, &currentBackendName);

					switch (Config::Instance()->Api)
					{
					case NVNGX_DX11:
						ImGui::Text("DirextX 11 - %s (%d.%d.%d)", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

						if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
							AddDx11Backends(&currentBackend, &currentBackendName);

						break;

					case NVNGX_DX12:
						ImGui::Text("DirextX 12 - %s (%d.%d.%d)", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

						if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
							AddDx12Backends(&currentBackend, &currentBackendName);

						break;

					default:
						ImGui::Text("Vulkan - %s (%d.%d.%d)", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);

						if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
							AddVulkanBackends(&currentBackend, &currentBackendName);
					}

					if (Config::Instance()->CurrentFeature->Name() != "DLSSD")
					{
						if (ImGui::Button("Apply") && Config::Instance()->newBackend != "" && Config::Instance()->newBackend != currentBackend)
							Config::Instance()->changeBackend = true;

						ImGui::SameLine(0.0f, 6.0f);

						if (ImGui::Button("Revert"))
							Config::Instance()->newBackend = "";
					}

					// DYNAMIC PROPERTIES -----------------------------

					// Dx11 with Dx12
					if (Config::Instance()->Api == NVNGX_DX11 &&
						Config::Instance()->Dx11Upscaler.value_or("fsr22") != "fsr22" && Config::Instance()->Dx11Upscaler.value_or("fsr22") != "dlss")
					{
						ImGui::SeparatorText("Dx11 with Dx12 Settings");

						const char* sync[] = { "No Syncing", "Fence", "Fence + Event", "Fence + Flush", "Fence + Flush + Event", "Only Query" };

						const char* selectedSync = sync[Config::Instance()->TextureSyncMethod.value_or(1)];

						if (ImGui::BeginCombo("Input Sync", selectedSync))
						{
							for (int n = 0; n < 6; n++)
							{
								if (ImGui::Selectable(sync[n], (Config::Instance()->TextureSyncMethod.value_or(1) == n)))
									Config::Instance()->TextureSyncMethod = n;
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
							}

							ImGui::EndCombo();
						}

						if (bool afterDx12 = Config::Instance()->SyncAfterDx12.value_or(true); ImGui::Checkbox("Sync After Dx12", &afterDx12))
							Config::Instance()->SyncAfterDx12 = afterDx12;

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
									Config::Instance()->changeBackend = true;
								}
							}

							ImGui::EndCombo();
						}

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

						bool noSpoof = Config::Instance()->DxgiXessNoSpoof.value_or(true);
						if (Config::Instance()->IsDxgiMode && ImGui::Checkbox("Disable DXGI Spoofing for XeSS", &noSpoof))
						{
							Config::Instance()->DxgiXessNoSpoof = noSpoof;
							Config::Instance()->newBackend = "xess";
							Config::Instance()->changeBackend = true;
						}
					}

					// FSR -----------------
					if (currentBackend.rfind("fsr", 0) == 0 && Config::Instance()->CurrentFeature->Name() != "DLSSD")
					{
						ImGui::SeparatorText("FSR Settings");

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

						ImGui::BeginDisabled(useVFov);

						if (ImGui::SliderFloat("Horz. FOV", &hfov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat))
							Config::Instance()->FsrHorizontalFov = hfov;

						ImGui::EndDisabled();
					}

					// DLSS -----------------
					if ((Config::Instance()->DLSSEnabled.value_or(true) && currentBackend == "dlss" && Config::Instance()->CurrentFeature->Version().major > 2) ||
						Config::Instance()->CurrentFeature->Name() == "DLSSD")
					{
						ImGui::SeparatorText("DLSS Settings");

						if (bool pOverride = Config::Instance()->RenderPresetOverride.value_or(false); ImGui::Checkbox("Render Presets Override", &pOverride))
							Config::Instance()->RenderPresetOverride = pOverride;

						ImGui::SameLine(0.0f, 6.0f);

						if (ImGui::Button("Apply Changes"))
						{
							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

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
					if (Config::Instance()->Api == NVNGX_DX12 ||
						(Config::Instance()->Api == NVNGX_DX11 && currentBackend != "fsr22" && currentBackend != "dlss"))
					{
						ImGui::SeparatorText("RCAS Settings");

						bool rcasEnabled = (Config::Instance()->Api == NVNGX_DX11 && currentBackend == "xess") ||
							(Config::Instance()->Api == NVNGX_DX12 &&
								(currentBackend == "xess" ||
									(currentBackend == "dlss" &&
										(Config::Instance()->CurrentFeature->Version().major > 2 ||
											(Config::Instance()->CurrentFeature->Version().major == 2 && Config::Instance()->CurrentFeature->Version().minor >= 5 && Config::Instance()->CurrentFeature->Version().patch >= 1))
										)
									)
								);

						if (bool rcas = Config::Instance()->RcasEnabled.value_or(rcasEnabled); ImGui::Checkbox("Enable RCAS", &rcas))
							Config::Instance()->RcasEnabled = rcas;


						ImGui::BeginDisabled(!Config::Instance()->RcasEnabled.value_or(rcasEnabled));

						if (bool overrideMotionSharpness = Config::Instance()->MotionSharpnessEnabled.value_or(false); ImGui::Checkbox("Motion Sharpness", &overrideMotionSharpness))
							Config::Instance()->MotionSharpnessEnabled = overrideMotionSharpness;

						ImGui::BeginDisabled(!Config::Instance()->MotionSharpnessEnabled.value_or(false));

						ImGui::SameLine(0.0f, 6.0f);

						if (bool overrideMSDebug = Config::Instance()->MotionSharpnessDebug.value_or(false); ImGui::Checkbox("MS Debug", &overrideMSDebug))
							Config::Instance()->MotionSharpnessDebug = overrideMSDebug;

						float motionSharpness = Config::Instance()->MotionSharpness.value_or(0.4f);
						ImGui::SliderFloat("MotionSharpness", &motionSharpness, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
						Config::Instance()->MotionSharpness = motionSharpness;

						float motionThreshod = Config::Instance()->MotionThreshold.value_or(0.0f);
						ImGui::SliderFloat("MotionThreshod", &motionThreshod, 0.0f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
						Config::Instance()->MotionThreshold = motionThreshod;

						float motionScale = Config::Instance()->MotionScaleLimit.value_or(10.0f);
						ImGui::SliderFloat("MotionRange", &motionScale, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
						Config::Instance()->MotionScaleLimit = motionScale;

						ImGui::EndDisabled();

						ImGui::EndDisabled();

					}

					// DLSS Enabler -----------------
					if (Config::Instance()->DE_Available || Config::Instance()->DE_FramerateLimit.has_value() ||
						(Config::Instance()->DE_DynamicLimitAvailable.has_value() && Config::Instance()->DE_DynamicLimitAvailable.value() > 0))
					{
						ImGui::SeparatorText("DLSS Enabler");

						if (Config::Instance()->DE_FramerateLimit.has_value() && _deLimitFps == 0)
							_deLimitFps = Config::Instance()->DE_FramerateLimit.value();

						ImGui::SliderInt("FPS Limit", &_deLimitFps, 0, 200);

						if (ImGui::Button("Apply Limit"))
							Config::Instance()->DE_FramerateLimit = _deLimitFps;

						if (Config::Instance()->DE_DynamicLimitAvailable.has_value() && Config::Instance()->DE_DynamicLimitAvailable.value() > 0)
						{
							bool dfgEnabled = Config::Instance()->DE_DynamicLimitEnabled.value_or(false);

							if (ImGui::Checkbox("Dynamic Frame Generation", &dfgEnabled))
								Config::Instance()->DE_DynamicLimitEnabled = dfgEnabled;
						}
					}

					// OUTPUT SCALING -----------------------------
					if (Config::Instance()->Api == NVNGX_DX12 ||
						(Config::Instance()->Api == NVNGX_DX11 && Config::Instance()->Dx11Upscaler.value_or("fsr22") != "fsr22" && Config::Instance()->Dx11Upscaler.value_or("fsr22") != "dlss"))
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
						}

						ImGui::Checkbox("Enable", &_ssEnabled);

						ImGui::SameLine(0.0f, 6.0f);

						if (ImGui::Button("Apply Change") && (_ssEnabled != Config::Instance()->OutputScalingEnabled.value_or(false) ||
							_ssRatio != Config::Instance()->OutputScalingMultiplier.value_or(defaultRatio)))
						{
							Config::Instance()->OutputScalingEnabled = _ssEnabled;
							Config::Instance()->OutputScalingMultiplier = _ssRatio;

							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

							Config::Instance()->changeBackend = true;
						}

						ImGui::SameLine(0.0f, 6.0f);

						if (cf != nullptr)
							ImGui::Text("%dx%d -> %dx%d", cf->RenderWidth(), cf->RenderHeight(), (uint32_t)(cf->DisplayWidth() * _ssRatio), (uint32_t)(cf->DisplayHeight() * _ssRatio));

						ImGui::SliderFloat("Ratio", &_ssRatio, 0.5f, 3.0f, "%.2f");

						ImGui::EndDisabled();
					}

					// DX12 -----------------------------
					if (Config::Instance()->Api == NVNGX_DX12)
					{
						// MIPMAP BIAS -----------------------------
						ImGui::SeparatorText("Mipmap Bias (Dx12)");

						if (Config::Instance()->MipmapBiasOverride.has_value() && _mipBias == 0.0f)
							_mipBias = Config::Instance()->MipmapBiasOverride.value();

						ImGui::SliderFloat("Mipmap Bias", &_mipBias, -15.0f, 15.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);

						ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "This will be applied after first resolution change!");

						if (ImGui::Button("Set"))
							Config::Instance()->MipmapBiasOverride = _mipBias;

						ImGui::SameLine(0.0f, 6.0f);

						if (ImGui::Button("Reset"))
						{
							Config::Instance()->MipmapBiasOverride.reset();
							_mipBias = 0.0f;
						}

						ImGui::SameLine(0.0f, 6.0f);

						if (ImGui::Button("Calculate"))
						{
							_showMipmapCalcWindow = true;
						}

						ImGui::SameLine(0.0f, 6.0f);

						ImGui::Text("Current : %.6f", Config::Instance()->lastMipBias);

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
							ImGui::SeparatorText("Hotfixes (Dx12)");

							if (bool crs = Config::Instance()->RestoreComputeSignature.value_or(false); ImGui::Checkbox("Restore Compute RS", &crs))
								Config::Instance()->RestoreComputeSignature = crs;

							ImGui::SameLine(0.0f, 6.0f);
							if (bool grs = Config::Instance()->RestoreGraphicSignature.value_or(false); ImGui::Checkbox("Restore Graphic RS", &grs))
								Config::Instance()->RestoreGraphicSignature = grs;
						}
					}

					// NEXT COLUMN -----------------
					ImGui::TableNextColumn();

					// SHARPNESS -----------------------------
					ImGui::SeparatorText("Sharpness");

					if (bool overrideSharpness = Config::Instance()->OverrideSharpness.value_or(false); ImGui::Checkbox("Override", &overrideSharpness))
					{
						Config::Instance()->OverrideSharpness = overrideSharpness;

						if (currentBackend == "dlss" && Config::Instance()->CurrentFeature->Version().major < 3)
							Config::Instance()->changeBackend = true;
					}

					ImGui::BeginDisabled(!Config::Instance()->OverrideSharpness.value_or(false));

					float sharpness = Config::Instance()->Sharpness.value_or(0.3f);
					ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
					Config::Instance()->Sharpness = sharpness;

					ImGui::EndDisabled();

					// UPSCALE RATIO OVERRIDE -----------------
					ImGui::SeparatorText("Upscale Ratio");
					if (bool upOverride = Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false); ImGui::Checkbox("Ratio Override", &upOverride))
						Config::Instance()->UpscaleRatioOverrideEnabled = upOverride;

					ImGui::BeginDisabled(!Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false));

					float urOverride = Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f);
					ImGui::SliderFloat("Override Ratio", &urOverride, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
					Config::Instance()->UpscaleRatioOverrideValue = urOverride;

					ImGui::EndDisabled();

					// QUALITY OVERRIDES -----------------------------
					ImGui::SeparatorText("Quality Overrides");

					if (bool qOverride = Config::Instance()->QualityRatioOverrideEnabled.value_or(false); ImGui::Checkbox("Quality Override", &qOverride))
						Config::Instance()->QualityRatioOverrideEnabled = qOverride;

					ImGui::BeginDisabled(!Config::Instance()->QualityRatioOverrideEnabled.value_or(false));

					float qDlaa = Config::Instance()->QualityRatio_DLAA.value_or(1.0f);
					if (ImGui::SliderFloat("DLAA", &qDlaa, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_DLAA = qDlaa;

					float qUq = Config::Instance()->QualityRatio_UltraQuality.value_or(1.3f);
					if (ImGui::SliderFloat("Ultra Quality", &qUq, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_UltraQuality = qUq;

					float qQ = Config::Instance()->QualityRatio_Quality.value_or(1.5f);
					if (ImGui::SliderFloat("Quality", &qQ, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_Quality = qQ;

					float qB = Config::Instance()->QualityRatio_Balanced.value_or(1.7f);
					if (ImGui::SliderFloat("Balanced", &qB, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_Balanced = qB;

					float qP = Config::Instance()->QualityRatio_Performance.value_or(2.0f);
					if (ImGui::SliderFloat("Performance", &qP, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_Performance = qP;

					float qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0f);
					if (ImGui::SliderFloat("Ultra Performance", &qUp, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
						Config::Instance()->QualityRatio_UltraPerformance = qUp;

					ImGui::EndDisabled();

					// DRS -----------------------------
					ImGui::SeparatorText("DRS (Dynamic Resolution Scaling)");
					if (ImGui::BeginTable("drs", 2))
					{
						ImGui::TableNextColumn();
						if (bool drsMin = Config::Instance()->DrsMinOverrideEnabled.value_or(false); ImGui::Checkbox("Override Minimum", &drsMin))
							Config::Instance()->DrsMinOverrideEnabled = drsMin;

						ImGui::TableNextColumn();
						if (bool drsMax = Config::Instance()->DrsMaxOverrideEnabled.value_or(false); ImGui::Checkbox("Override Maximum", &drsMax))
							Config::Instance()->DrsMaxOverrideEnabled = drsMax;

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

							Config::Instance()->changeBackend = true;
						}

						ImGui::TableNextColumn();
						if (bool hdr = Config::Instance()->HDR.value_or(false); ImGui::Checkbox("HDR", &hdr))
						{
							Config::Instance()->HDR = hdr;

							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

							Config::Instance()->changeBackend = true;
						}

						ImGui::TableNextColumn();
						if (bool depth = Config::Instance()->DepthInverted.value_or(false); ImGui::Checkbox("Depth Inverted", &depth))
						{
							Config::Instance()->DepthInverted = depth;

							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

							Config::Instance()->changeBackend = true;
						}

						ImGui::TableNextColumn();
						if (bool jitter = Config::Instance()->JitterCancellation.value_or(false); ImGui::Checkbox("Jitter Cancellation", &jitter))
						{
							Config::Instance()->JitterCancellation = jitter;

							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

							Config::Instance()->changeBackend = true;
						}

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

							Config::Instance()->changeBackend = true;
						}

						ImGui::TableNextColumn();
						if (bool rm = Config::Instance()->DisableReactiveMask.value_or(true); ImGui::Checkbox("Disable Reactive Mask", &rm))
						{
							Config::Instance()->DisableReactiveMask = rm;

							if (Config::Instance()->CurrentFeature->Name() == "DLSSD")
								Config::Instance()->newBackend = "dlssd";

							Config::Instance()->changeBackend = true;
						}

						ImGui::EndTable();
					}

					// LOGGING -----------------------------
					ImGui::SeparatorText("Logging");

					if (bool logging = Config::Instance()->LoggingEnabled.value_or(true); ImGui::Checkbox("Logging", &logging))
					{
						Config::Instance()->LoggingEnabled = logging;

						if (logging)
							spdlog::default_logger()->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));
						else
							spdlog::default_logger()->set_level(spdlog::level::off);
					}

					ImGui::SameLine(0.0f, 6.0f);
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

					ImGui::EndTable();

					// BOTTOM LINE ---------------
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					if (cf != nullptr)
						ImGui::Text("%dx%d -> %dx%d (%dx%d)", cf->RenderWidth(), cf->RenderHeight(), cf->TargetWidth(), cf->TargetHeight(), cf->DisplayWidth(), cf->DisplayHeight());

					ImGui::SameLine(0.0f, 4.0f);

					ImGui::Text("%.3f ms/frame (%.1f FPS) %d", 1000.0f / io.Framerate, io.Framerate, Config::Instance()->CurrentFeature->FrameCount());

					ImGui::SameLine(0.0f, 10.0f);

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
				}

				ImGui::End();
			}

			if (_showMipmapCalcWindow)
			{
				auto posX = (Config::Instance()->CurrentFeature->DisplayWidth() - 450.0f) / 2.0f;
				auto posY = (Config::Instance()->CurrentFeature->DisplayHeight() - 200.0f) / 2.0f;

				ImGui::SetNextWindowPos(ImVec2{ posX , posY }, ImGuiCond_FirstUseEver);
				ImGui::SetNextWindowSize(ImVec2{ 450.0f, 200.0f }, ImGuiCond_FirstUseEver);

				if (_displayWidth == 0)
				{
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

					if (ImGui::InputScalar("Display Width", ImGuiDataType_U32, &_displayWidth, NULL, NULL, "%u"))
					{
						if (_displayWidth <= 0)
							_displayWidth = Config::Instance()->CurrentFeature->DisplayWidth();

						_renderWidth = _displayWidth / _mipmapUpscalerRatio;
						_mipBiasCalculated = log2((float)_renderWidth / (float)_displayWidth);
					}

					const char* q[] = { "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality", "DLAA" };
					//float xr[] = { 3.0f, 2.3f, 2.0f, 1.7f, 1.5f, 1.0f };
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

					if (ImGui::SliderFloat("Upscaler Ratio", &_mipmapUpscalerRatio, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
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

	static void Init(HWND InHwnd)
	{
		_handle = InHwnd;
		_isVisible = false;
		_isResetRequested = false;

		spdlog::debug("ImGuiCommon::Init Handle: {0:X}", (ULONG64)_handle);

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
		spdlog::debug("ImGuiCommon::Init ImGui_ImplWin32_Init result: {0}", initResult);

		if (_oWndProc == nullptr)
			_oWndProc = (WNDPROC)SetWindowLongPtr(InHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		spdlog::debug("ImGuiCommon::Init _oWndProc: {0:X}", (ULONG64)_oWndProc);


		if (!pfn_SetCursorPos_hooked)
			AttachHooks();

		if (Config::Instance()->MenuScale.value_or(1.0f) < 1.0f)
			Config::Instance()->MenuScale = 1.0f;

		if (Config::Instance()->MenuScale.value_or(1.0f) > 2.0f)
			Config::Instance()->MenuScale = 2.0f;

		_selectedScale = (int)((Config::Instance()->MenuScale.value_or(1.0f) - 1.0f) / 0.1f);

		_isInited = true;
	}

	static void Shutdown()
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

	static void HideMenu()
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
};
