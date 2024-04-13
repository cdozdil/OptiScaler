#include "../Config.h"
#include "../Logger.h"

#include "Imgui_Base.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"
#pragma comment(lib, "../detours/detours.lib")

PFN_SetCursorPos pfn_SetPhysicalCursorPos = nullptr;
PFN_SetCursorPos pfn_SetCursorPos = nullptr;
PFN_mouse_event pfn_mouse_event = nullptr;
PFN_SendInput pfn_SendInput = nullptr;
PFN_SendMessageW pfn_SendMessageW = nullptr;

bool pfn_SetPhysicalCursorPos_hooked = false;
bool pfn_SetCursorPos_hooked = false;
bool pfn_mouse_event_hooked = false;
bool pfn_SendInput_hooked = false;
bool pfn_SendMessageW_hooked = false;

bool showMipmapCalcWindow = false;
float mipBias = 0.0f;
float mipBiasCalculated = 0.0f;
uint32_t mipmapUpscalerQuality = 0;
float mipmapUpscalerRatio = 0;
uint32_t displayWidth = 0;
uint32_t renderWidth = 0;

float ssRatio = 0.0f;
bool ssEnabled = false;

bool _isVisible = false;
WNDPROC _oWndProc = nullptr;

LRESULT WINAPI hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (_isVisible && Msg == 0x0020)
		return TRUE;
	else
		return pfn_SendMessageW(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI hkSetPhysicalCursorPos(int x, int y)
{
	if (_isVisible)
		return TRUE;
	else
		return pfn_SetPhysicalCursorPos(x, y);
}

BOOL WINAPI hkSetCursorPos(int x, int y)
{
	if (_isVisible)
		return TRUE;
	else
		return pfn_SetCursorPos(x, y);
}

void WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo)
{
	if (_isVisible)
		return;
	else
		pfn_mouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
}

UINT WINAPI hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
	if (_isVisible)
		return TRUE;
	else
		return pfn_SendInput(cInputs, pInputs, cbSize);
}

void AttachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// Detour the functions
	pfn_SetPhysicalCursorPos = reinterpret_cast<PFN_SetCursorPos>(DetourFindFunction("user32.dll", "SetPhysicalCursorPos"));
	pfn_SetCursorPos = reinterpret_cast<PFN_SetCursorPos>(DetourFindFunction("user32.dll", "SetCursorPos"));
	pfn_mouse_event = reinterpret_cast<PFN_mouse_event>(DetourFindFunction("user32.dll", "mouse_event"));
	pfn_SendInput = reinterpret_cast<PFN_SendInput>(DetourFindFunction("user32.dll", "SendInput"));
	pfn_SendMessageW = reinterpret_cast<PFN_SendMessageW>(DetourFindFunction("user32.dll", "SendMessageW"));

	if (pfn_SetPhysicalCursorPos && (pfn_SetPhysicalCursorPos != pfn_SetCursorPos))
		pfn_SetPhysicalCursorPos_hooked = (DetourAttach(&(PVOID&)pfn_SetPhysicalCursorPos, hkSetPhysicalCursorPos) == 0);

	if (pfn_SetCursorPos)
		pfn_SetCursorPos_hooked = (DetourAttach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos) == 0);

	if (pfn_mouse_event)
		pfn_mouse_event_hooked = (DetourAttach(&(PVOID&)pfn_mouse_event, hkmouse_event) == 0);

	if (pfn_SendInput)
		pfn_SendInput_hooked = (DetourAttach(&(PVOID&)pfn_SendInput, hkSendInput) == 0);

	if (pfn_SendMessageW)
		pfn_SendMessageW_hooked = (DetourAttach(&(PVOID&)pfn_SendMessageW, hkSendMessageW) == 0);

	DetourTransactionCommit();
}

void DetachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (pfn_SetPhysicalCursorPos_hooked)
		DetourDetach(&(PVOID&)pfn_SetPhysicalCursorPos, hkSetPhysicalCursorPos);

	if (pfn_SetCursorPos_hooked)
		DetourDetach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos);

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

/*Forward declare message handler from imgui_impl_win32.cpp*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// CTRL + HOME
	if (msg == WM_KEYDOWN && wParam == VK_HOME) // && (GetKeyState(VK_SHIFT) & 0x8000))
	{
		_isVisible = !_isVisible;
		io.MouseDrawCursor = _isVisible;
		io.WantCaptureKeyboard = _isVisible;
		io.WantCaptureMouse = _isVisible;

		return TRUE;
	}

	if (msg == WM_KEYDOWN && wParam == VK_DELETE && (GetKeyState(VK_SHIFT) & 0x8000))
	{
		Config::Instance()->xessDebug = true;
		return TRUE;
	}

	// Imgui
	if (_isVisible)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return TRUE;

		switch (msg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_MOUSEMOVE:
		case WM_SETCURSOR:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_MOUSELAST:
		case WM_INPUT:
			return TRUE;

		default:
			break;
		}

		auto log = std::format("WNDPROC MSG : {}", msg);
		OutputDebugStringA(log.c_str());
	}

	return CallWindowProc(_oWndProc, hWnd, msg, wParam, lParam);
}

std::string GetBackendName(std::string* code)
{
	if (*code == "fsr21")
		return "FSR 2.1.2";

	if (*code == "fsr22")
		return "FSR 2.2.1";

	if (*code == "fsr21_12")
		return "FSR 2.1.2 w/Dx12";

	if (*code == "fsr22_12")
		return "FSR 2.2.1  w/Dx12";

	if (*code == "xess")
		return "XeSS";

	return "????";
}

std::string GetBackendCode(const NVNGX_Api api)
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

void GetCurrentBackendInfo(const NVNGX_Api api, std::string* code, std::string* name)
{
	*code = GetBackendCode(api);
	*name = GetBackendName(code);
}

void AddDx11Backends(std::string* code, std::string* name)
{
	std::string selectedUpscalerName = "";

	if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
		selectedUpscalerName = "FSR 2.2.1";
	else if (Config::Instance()->newBackend == "fsr22_12" || (Config::Instance()->newBackend == "" && *code == "fsr22_12"))
		selectedUpscalerName = "FSR 2.2.1 w/Dx12";
	else if (Config::Instance()->newBackend == "fsr21_12" || (Config::Instance()->newBackend == "" && *code == "fsr21_12"))
		selectedUpscalerName = "FSR 2.1.2 w/Dx12";
	else
		selectedUpscalerName = "XeSS w/Dx12";

	if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
	{
		if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
			Config::Instance()->newBackend = "fsr22";

		if (ImGui::Selectable("XeSS w/Dx12", false, *code == "xess"))
			Config::Instance()->newBackend = "xess";

		if (ImGui::Selectable("FSR 2.1.2 w/Dx12", *code == "fsr21_12"))
			Config::Instance()->newBackend = "fsr21_12";

		if (ImGui::Selectable("FSR 2.2.1 w/Dx12", *code == "fsr22_12"))
			Config::Instance()->newBackend = "fsr22_12";

		ImGui::EndCombo();
	}
}

void AddDx12Backends(std::string* code, std::string* name)
{
	std::string selectedUpscalerName = "";

	if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
		selectedUpscalerName = "FSR 2.1.2";
	else if (Config::Instance()->newBackend == "fsr22" || (Config::Instance()->newBackend == "" && *code == "fsr22"))
		selectedUpscalerName = "FSR 2.2.1";
	else
		selectedUpscalerName = "XeSS";

	if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
	{
		if (ImGui::Selectable("XeSS", false, *code == "xess"))
			Config::Instance()->newBackend = "xess";

		if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
			Config::Instance()->newBackend = "fsr21";

		if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
			Config::Instance()->newBackend = "fsr22";

		ImGui::EndCombo();
	}
}

void AddVulkanBackends(std::string* code, std::string* name)
{
	std::string selectedUpscalerName = "";

	if (Config::Instance()->newBackend == "fsr21" || (Config::Instance()->newBackend == "" && *code == "fsr21"))
		selectedUpscalerName = "FSR 2.1.2";
	else
		selectedUpscalerName = "FSR 2.2.1";

	if (ImGui::BeginCombo("Select", selectedUpscalerName.c_str()))
	{
		if (ImGui::Selectable("FSR 2.1.2", *code == "fsr21"))
			Config::Instance()->newBackend = "fsr21";

		if (ImGui::Selectable("FSR 2.2.1", *code == "fsr22"))
			Config::Instance()->newBackend = "fsr22";

		ImGui::EndCombo();
	}
}

void AddResourceBarrier(std::string name, std::optional<int>* value)
{
	const char* states[] = { "Auto", "COMMON", "VERTEX_AND_CONSTANT_BUFFER", "INDEX_BUFFER", "RENDER_TARGET", "UNORDERED_ACCESS", "DEPTH_WRITE",
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

void Imgui_Base::RenderMenu()
{
	ImGuiIO const& io = ImGui::GetIO(); (void)io;

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		ImGuiWindowFlags flags = 0;
		flags |= ImGuiWindowFlags_NoSavedSettings;
		flags |= ImGuiWindowFlags_NoCollapse;

		auto posX = (Config::Instance()->CurrentFeature->DisplayWidth() - 770.0f) / 2.0f;
		auto posY = (Config::Instance()->CurrentFeature->DisplayHeight() - 670.0f) / 2.0f;

		ImGui::SetNextWindowPos(ImVec2{ posX , posY }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2{ 770.0f, 670.0f }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("OmniScaler v0.4", nullptr, flags))
		{
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
					AddDx11Backends(&currentBackend, &currentBackendName);
					break;

				case NVNGX_DX12:
					ImGui::Text("DirextX 12 - %s (%d.%d.%d)", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);
					AddDx12Backends(&currentBackend, &currentBackendName);
					break;

				default:
					ImGui::Text("Vulkan - %s (%d.%d.%d)", Config::Instance()->CurrentFeature->Name(), Config::Instance()->CurrentFeature->Version().major, Config::Instance()->CurrentFeature->Version().minor, Config::Instance()->CurrentFeature->Version().patch);
					AddVulkanBackends(&currentBackend, &currentBackendName);
				}

				if (ImGui::Button("Apply") && Config::Instance()->newBackend != "" && Config::Instance()->newBackend != currentBackend)
					Config::Instance()->changeBackend = true;

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Revert"))
					Config::Instance()->newBackend = "";

				// Dx11
				ImGui::BeginDisabled(Config::Instance()->Api != NVNGX_DX11);

				const char* sync[] = { "No Syncing", "Shared Fences", "Shared Fences + Flush", "Shared Fences + Query", "Mostly Queries" };

				const char* selectedSync = sync[Config::Instance()->UseSafeSyncQueries.value_or(1)];

				if (ImGui::BeginCombo("Dx12wDx11 Sync", selectedSync))
				{
					for (int n = 0; n < 5; n++)
					{
						if (ImGui::Selectable(sync[n], (Config::Instance()->UseSafeSyncQueries.value_or(1) == n)))
							Config::Instance()->UseSafeSyncQueries = n;
					}

					ImGui::EndCombo();
				}

				ImGui::EndDisabled();

				// UPSCALER SPECIFIC -----------------------------
				// XeSS
				ImGui::BeginDisabled(currentBackend != "xess");
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

				if (bool cas = Config::Instance()->CasEnabled.value_or(true); ImGui::Checkbox("CAS", &cas))
					Config::Instance()->CasEnabled = cas;

				ImGui::SameLine(0.0f, 6.0f);
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

				ImGui::BeginDisabled(!Config::Instance()->CasEnabled.value_or(true));

				const char* cs[] = { "LINEAR", "GAMMA20", "GAMMA22", "SRGB_OUTPUT", "SRGB_INPUT_OUTPUT" };
				auto configCs = Config::Instance()->CasColorSpaceConversion.value_or(0);

				if (configCs < 0)
					configCs = 0;
				else if (configCs > 4)
					configCs = 4;

				const char* selectedCs = cs[configCs];

				if (ImGui::BeginCombo("Color Space", selectedCs))
				{
					for (int n = 0; n < 5; n++)
					{
						if (ImGui::Selectable(cs[n], (Config::Instance()->CasColorSpaceConversion.value_or(0) == n)))
						{
							Config::Instance()->CasColorSpaceConversion = n;
							Config::Instance()->changeCAS = true;
						}
					}

					ImGui::EndCombo();
				}

				ImGui::EndDisabled();

				ImGui::EndDisabled();

				// FSR -----------------
				ImGui::BeginDisabled(currentBackend == "xess");
				ImGui::SeparatorText("FSR Settings");

				bool useVFov = Config::Instance()->FsrVerticalFov.has_value() || !Config::Instance()->FsrHorizontalFov.has_value();

				float vfov;
				float hfov;

				vfov = Config::Instance()->FsrVerticalFov.value_or(60.0f);
				hfov = Config::Instance()->FsrHorizontalFov.value_or(90.0f);

				if (ImGui::RadioButton("Use Vert. Fov", useVFov))
				{
					Config::Instance()->FsrHorizontalFov.reset();
					useVFov = true;
				}

				ImGui::SameLine();

				if (ImGui::RadioButton("Use Horz. Fov", !useVFov))
				{
					Config::Instance()->FsrVerticalFov.reset();
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

				ImGui::EndDisabled();

				// SUPERSAMPLING -----------------------------
				ImGui::SeparatorText("Supersampling");

				bool isXess12 = (Config::Instance()->Dx12Upscaler.value_or("xess") == "xess" || Config::Instance()->Dx11Upscaler.value_or("fsr22") == "xess") &&
					Config::Instance()->CurrentFeature->Version().major <= 1 && Config::Instance()->CurrentFeature->Version().minor <= 2;

				float defaultRatio = 2.5f;

				if (ssRatio == 0.0f)
				{
					ssRatio = Config::Instance()->SuperSamplingMultiplier.value_or(2.5f);
					ssEnabled = Config::Instance()->SuperSamplingEnabled.value_or(false);
				}

				ImGui::BeginDisabled(
					(Config::Instance()->Api == NVNGX_VULKAN || (Config::Instance()->Api == NVNGX_DX11 && Config::Instance()->Dx11Upscaler.value_or("fsr22") == "fsr22")) ||
					Config::Instance()->DisplayResolution.value_or(false)
				);

				ImGui::Checkbox("Enable", &ssEnabled);

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Apply Change"))
				{
					if (ssEnabled != Config::Instance()->SuperSamplingEnabled.value_or(false) || ssRatio != Config::Instance()->SuperSamplingMultiplier.value_or(defaultRatio))
					{
						Config::Instance()->SuperSamplingEnabled = ssEnabled;
						Config::Instance()->SuperSamplingMultiplier = ssRatio;
						Config::Instance()->changeBackend = true;
					}
				}

				ImGui::SliderFloat("Ratio", &ssRatio, (float)Config::Instance()->CurrentFeature->DisplayWidth() / (float)Config::Instance()->CurrentFeature->RenderWidth(),
					isXess12 ? 2.5f : 5.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);

				ImGui::EndDisabled();

				// SHARPNESS -----------------------------
				ImGui::SeparatorText("Sharpness");

				if (bool overrideSharpness = Config::Instance()->OverrideSharpness.value_or(false); ImGui::Checkbox("Override", &overrideSharpness))
					Config::Instance()->OverrideSharpness = overrideSharpness;

				ImGui::BeginDisabled(!Config::Instance()->OverrideSharpness.value_or(false));

				float sharpness = Config::Instance()->Sharpness.value_or(0.3f);
				ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->Sharpness = sharpness;

				ImGui::EndDisabled();

				// MIPMAP BIAS -----------------------------
				ImGui::SeparatorText("Mipmap Bias (Dx12)");

				ImGui::BeginDisabled(Config::Instance()->Api != NVNGX_DX12);

				if (Config::Instance()->MipmapBiasOverride.has_value() && mipBias == 0.0f)
					mipBias = Config::Instance()->MipmapBiasOverride.value();

				ImGui::SliderFloat("Mipmap Bias", &mipBias, -15.0f, 15.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);

				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "This will be applied after first resolution change!");

				if (ImGui::Button("Set"))
					Config::Instance()->MipmapBiasOverride = mipBias;

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Reset"))
				{
					Config::Instance()->MipmapBiasOverride.reset();
					mipBias = 0.0f;
				}

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Calculate"))
				{
					showMipmapCalcWindow = true;
				}

				ImGui::SameLine(0.0f, 6.0f);

				ImGui::Text("Current : %.6f", Config::Instance()->lastMipBias);

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

				ImGui::TableNextColumn();

				// QUALITY OVERRIDES -----------------------------
				ImGui::SeparatorText("Quality Overrides");

				if (bool qOverride = Config::Instance()->QualityRatioOverrideEnabled.value_or(false); ImGui::Checkbox("Quality Override", &qOverride))
					Config::Instance()->QualityRatioOverrideEnabled = qOverride;

				ImGui::BeginDisabled(!Config::Instance()->QualityRatioOverrideEnabled.value_or(false));

				float qUq = Config::Instance()->QualityRatio_UltraQuality.value_or(1.3f);
				ImGui::SliderFloat("Ultra Quality", &qUq, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->QualityRatio_UltraQuality = qUq;

				float qQ = Config::Instance()->QualityRatio_Quality.value_or(1.5f);
				ImGui::SliderFloat("Quality", &qQ, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->QualityRatio_Quality = qQ;

				float qB = Config::Instance()->QualityRatio_Balanced.value_or(1.7f);
				ImGui::SliderFloat("Balanced", &qB, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->QualityRatio_Balanced = qB;

				float qP = Config::Instance()->QualityRatio_Performance.value_or(2.0f);
				ImGui::SliderFloat("Performance", &qP, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->QualityRatio_Performance = qP;

				float qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0f);
				ImGui::SliderFloat("Ultra Performance", &qUp, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat);
				Config::Instance()->QualityRatio_UltraPerformance = qUp;

				ImGui::EndDisabled();

				// INIT -----------------------------
				ImGui::SeparatorText("Init Flags");
				if (ImGui::BeginTable("init", 2))
				{
					ImGui::TableNextColumn();
					if (bool autoExposure = Config::Instance()->AutoExposure.value_or(false); ImGui::Checkbox("Auto Exposure", &autoExposure))
					{
						Config::Instance()->AutoExposure = autoExposure;
						Config::Instance()->changeBackend = true;
					}

					ImGui::TableNextColumn();
					if (bool hdr = Config::Instance()->HDR.value_or(false); ImGui::Checkbox("HDR", &hdr))
					{
						Config::Instance()->HDR = hdr;
						Config::Instance()->changeBackend = true;
					}

					ImGui::TableNextColumn();
					if (bool depth = Config::Instance()->DepthInverted.value_or(false); ImGui::Checkbox("Depth Inverted", &depth))
					{
						Config::Instance()->DepthInverted = depth;
						Config::Instance()->changeBackend = true;
					}

					ImGui::TableNextColumn();
					if (bool jitter = Config::Instance()->JitterCancellation.value_or(false); ImGui::Checkbox("Jitter Cancellation", &jitter))
					{
						Config::Instance()->JitterCancellation = jitter;
						Config::Instance()->changeBackend = true;
					}

					ImGui::TableNextColumn();
					if (bool mv = Config::Instance()->DisplayResolution.value_or(false); ImGui::Checkbox("Display Res. MV", &mv))
					{
						Config::Instance()->DisplayResolution = mv;
						Config::Instance()->changeBackend = true;
					}

					ImGui::TableNextColumn();
					if (bool rm = Config::Instance()->DisableReactiveMask.value_or(true); ImGui::Checkbox("Disable Reactive Mask", &rm))
					{
						Config::Instance()->DisableReactiveMask = rm;
						Config::Instance()->changeBackend = true;
					}

					ImGui::EndTable();
				}

				// BARRIERS -----------------------------
				ImGui::SeparatorText("Resource Barriers (Dx12)");
				ImGui::BeginDisabled(Config::Instance()->Api != NVNGX_DX12);

				AddResourceBarrier("Color", &Config::Instance()->ColorResourceBarrier);
				AddResourceBarrier("Depth", &Config::Instance()->DepthResourceBarrier);
				AddResourceBarrier("Motion", &Config::Instance()->MVResourceBarrier);
				AddResourceBarrier("Exposure", &Config::Instance()->ExposureResourceBarrier);
				AddResourceBarrier("Mask", &Config::Instance()->MaskResourceBarrier);
				AddResourceBarrier("Output", &Config::Instance()->OutputResourceBarrier);

				ImGui::EndDisabled();

				// HOTFIXES -----------------------------
				ImGui::SeparatorText("Hotfixes (Dx12)");

				ImGui::BeginDisabled(Config::Instance()->Api != NVNGX_DX12);

				if (bool crs = Config::Instance()->RestoreComputeSignature.value_or(false); ImGui::Checkbox("Restore Compute RS", &crs))
					Config::Instance()->RestoreComputeSignature = crs;

				ImGui::SameLine(0.0f, 6.0f);
				if (bool grs = Config::Instance()->RestoreGraphicSignature.value_or(false); ImGui::Checkbox("Restore Graphic RS", &grs))
					Config::Instance()->RestoreGraphicSignature = grs;

				ImGui::EndDisabled();

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

				auto cf = Config::Instance()->CurrentFeature;
				if (cf != nullptr)
					ImGui::Text("%dx%d -> %dx%d (%dx%d)", cf->RenderWidth(), cf->RenderHeight(), cf->TargetWidth(), cf->TargetHeight(), cf->DisplayWidth(), cf->DisplayHeight());

				ImGui::SameLine(0.0f, 4.0f);

				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

				ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);

				if (ImGui::Button("Save INI"))
					Config::Instance()->SaveIni("nvngx.ini");

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Close"))
				{
					showMipmapCalcWindow = false;
					_isVisible = false;
				}

				ImGui::Spacing();
				ImGui::Separator();
			}

			auto pos = ImGui::GetWindowPos();
			_top = pos.y;
			_left = pos.x;

			auto size = ImGui::GetWindowSize();
			_width = size.x;
			_height = size.y;

			ImGui::End();
		}

		if (showMipmapCalcWindow)
		{
			posX = (Config::Instance()->CurrentFeature->DisplayWidth() - 450.0f) / 2.0f;
			posY = (Config::Instance()->CurrentFeature->DisplayHeight() - 200.0f) / 2.0f;

			ImGui::SetNextWindowPos(ImVec2{ posX , posY }, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2{ 450.0f, 200.0f }, ImGuiCond_FirstUseEver);

			if (displayWidth == 0)
			{
				displayWidth = Config::Instance()->CurrentFeature->DisplayWidth();
				renderWidth = displayWidth / 3.0;
				mipmapUpscalerQuality = 0;
				mipmapUpscalerRatio = 3.0f;
				mipBiasCalculated = log2((float)renderWidth / (float)displayWidth);
			}

			if (ImGui::Begin("Mipmap Bias", nullptr, flags))
			{
				if (ImGui::InputScalar("Display Width", ImGuiDataType_U32, &displayWidth, NULL, NULL, "%u"))
				{
					if (displayWidth <= 0)
						displayWidth = Config::Instance()->CurrentFeature->DisplayWidth();

					renderWidth = displayWidth / mipmapUpscalerRatio;
					mipBiasCalculated = log2((float)renderWidth / (float)displayWidth);
				}

				const char* q[] = { "Ultra Performance", "Performance", "Balanced", "Quality", "Ultra Quality", "DLAA" };
				float xr[] = { 3.0f, 2.3f, 2.0f, 1.7f, 1.5f, 1.0f };
				float fr[] = { 3.0f, 2.0f, 1.7f, 1.5f, 1.3f, 1.0f };
				auto configQ = mipmapUpscalerQuality;

				const char* selectedQ = q[configQ];

				ImGui::BeginDisabled(Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false));

				if (ImGui::BeginCombo("Upscaler Quality", selectedQ))
				{
					for (int n = 0; n < 6; n++)
					{
						if (ImGui::Selectable(q[n], (mipmapUpscalerQuality == n)))
						{
							mipmapUpscalerQuality = n;

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
							{
								mipmapUpscalerRatio = ov;
							}
							else
							{
								if (Config::Instance()->Dx12Upscaler.value_or("xess") == "xess")
									mipmapUpscalerRatio = xr[n];
								else
									mipmapUpscalerRatio = fr[n];
							}

							renderWidth = displayWidth / mipmapUpscalerRatio;
							mipBiasCalculated = log2((float)renderWidth / (float)displayWidth);
						}
					}

					ImGui::EndCombo();
				}

				ImGui::EndDisabled();

				if (ImGui::SliderFloat("Upscaler Ratio", &mipmapUpscalerRatio, 1.0f, 3.0f, "%.3f", ImGuiSliderFlags_NoRoundToFormat))
				{
					renderWidth = displayWidth / mipmapUpscalerRatio;
					mipBiasCalculated = log2((float)renderWidth / (float)displayWidth);
				}

				if (ImGui::InputScalar("Render Width", ImGuiDataType_U32, &renderWidth, NULL, NULL, "%u"))
					mipBiasCalculated = log2((float)renderWidth / (float)displayWidth);

				ImGui::SliderFloat("Mipmap Bias", &mipBiasCalculated, -15.0f, 0.0f, "%.6f", ImGuiSliderFlags_NoRoundToFormat);

				// BOTTOM LINE
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);

				if (ImGui::Button("Use Value"))
				{
					mipBias = mipBiasCalculated;
					showMipmapCalcWindow = false;
				}

				ImGui::SameLine(0.0f, 6.0f);

				if (ImGui::Button("Close"))
					showMipmapCalcWindow = false;

				ImGui::Spacing();
				ImGui::Separator();

				ImGui::End();
			}
		}
	}

	ImGui::Render();
}

bool Imgui_Base::IsHandleDifferent()
{
	DWORD procId;
	GetWindowThreadProcessId(_handle, &procId);

	HWND frontWindow = GetForegroundWindow(); // Util::GetProcessWindow(); -- for linux compatibility

	if (processId == procId && frontWindow == _handle)
		return false;

	GetWindowThreadProcessId(frontWindow, &procId);

	_handle = frontWindow;

	return true;
}

Imgui_Base::Imgui_Base(HWND handle)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsDark();

	auto style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.20f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.20f);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	io.MouseDrawCursor = _isVisible;
	io.WantCaptureKeyboard = _isVisible;
	io.WantCaptureMouse = _isVisible;
	io.WantSetMousePos = _isVisible;

	_handle = handle;

	_baseInit = ImGui_ImplWin32_Init(_handle);

	if (IsHandleDifferent())
		return;

	if (_oWndProc == nullptr)
		_oWndProc = (WNDPROC)SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)WndProc);

	if (!pfn_SetCursorPos_hooked)
		AttachHooks();
}

Imgui_Base::~Imgui_Base()
{
	if (!_baseInit)
		return;

	if (auto currCtx = ImGui::GetCurrentContext(); currCtx && context != currCtx)
	{
		ImGui::SetCurrentContext(context);
		ImGui_ImplWin32_Shutdown();
		ImGui::SetCurrentContext(currCtx);
	}
	else
	{
		if (_oWndProc != nullptr)
		{
			SetWindowLongPtr((HWND)ImGui::GetMainViewport()->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)_oWndProc);
			_oWndProc = nullptr;
		}

		if (pfn_SetCursorPos_hooked)
			DetachHooks();

		ImGui_ImplWin32_Shutdown();
	}

	ImGui::DestroyContext(context);
}

bool Imgui_Base::IsVisible() const
{
	return _isVisible;
}

void Imgui_Base::SetVisible(bool visible) const
{
	_isVisible = visible;
}