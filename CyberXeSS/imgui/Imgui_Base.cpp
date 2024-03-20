#include "Imgui_Base.h"
#include "../Config.h"
#include "imgui/imgui_impl_win32.h"

#include "../detours/detours.h"
#pragma comment(lib, "../detours/detours.lib")

PFN_SetCursorPos pfn_SetCursorPos = nullptr;
bool pfn_SetCursorPos_hooked = false;

bool _isVisible = false;
WNDPROC _oWndProc = nullptr;

BOOL WINAPI hkSetCursorPos(int x, int y)
{
	if (_isVisible)
		return TRUE;
	else
		return pfn_SetCursorPos(x, y);
}

void AttachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
		
	// Detour the functions
	pfn_SetCursorPos = reinterpret_cast<PFN_SetCursorPos>(DetourFindFunction("user32.dll", "SetCursorPos"));

	if (pfn_SetCursorPos)
		pfn_SetCursorPos_hooked = (DetourAttach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos) == 0);

	DetourTransactionCommit();
}

void DetachHooks()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (pfn_SetCursorPos_hooked)
		DetourDetach(&(PVOID&)pfn_SetCursorPos, hkSetCursorPos);

	DetourTransactionCommit();
}

/*Forward declare message handler from imgui_impl_win32.cpp*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// CTRL + HOME
	if (msg == WM_KEYDOWN && wParam == VK_HOME && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		_isVisible = !_isVisible;
		io.MouseDrawCursor = _isVisible;
		io.WantCaptureKeyboard = _isVisible;
		io.WantCaptureMouse = _isVisible;
		io.WantSetMousePos = _isVisible;

		return true;
	}

	// Imgui
	if (_isVisible)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return true;
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

	ImGui::Text("DirextX 11 - %s", (*name).c_str());

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

	ImGui::Text("DirextX 12 - %s", (*name).c_str());

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

	ImGui::Text("Vulkan - %s", (*name).c_str());

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

		ImGui::SetNextWindowPos(ImVec2{ 350.0f, 300.0f }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2{ 770.0f, 495.0f }, ImGuiCond_FirstUseEver);

		if (ImGui::Begin("CyberXeSS v0.4", nullptr, flags))
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
					AddDx11Backends(&currentBackend, &currentBackendName);
					break;

				case NVNGX_DX12:
					AddDx12Backends(&currentBackend, &currentBackendName);
					break;

				default:
					AddVulkanBackends(&currentBackend, &currentBackendName);
				}

				if (ImGui::Button("Apply") && Config::Instance()->newBackend != "" && Config::Instance()->newBackend != currentBackend)
					Config::Instance()->changeBackend = true;

				ImGui::SameLine();

				if (ImGui::Button("Revert"))
					Config::Instance()->newBackend = "";

				// UPSCALER SPECIFIC -----------------------------
				// XeSS
				ImGui::BeginDisabled(currentBackend != "xess");
				ImGui::SeparatorText("XeSS Settings");

				if (bool cas = Config::Instance()->CasEnabled.value_or(true); ImGui::Checkbox("CAS", &cas))
					Config::Instance()->CasEnabled = cas;

				ImGui::BeginDisabled(!Config::Instance()->CasEnabled.value_or(true));

				const char* cs[] = { "LINEAR", "GAMMA20", "GAMMA22", "SRGB_OUTPUT", "SRGB_INPUT_OUTPUT" };
				const char* selectedCs = cs[Config::Instance()->ColorResourceBarrier.value_or(0)];

				if (ImGui::BeginCombo("Color Space", selectedCs))
				{
					for (int n = 0; n < 5; n++)
					{
						if (ImGui::Selectable(cs[n], (Config::Instance()->ColorResourceBarrier.value_or(0) == n)))
						{
							Config::Instance()->ColorResourceBarrier = n;
							Config::Instance()->changeBackend = true;
						}
					}

					ImGui::EndCombo();
				}
				
				ImGui::EndDisabled();

				const char* quality[] = { "Auto", "Performance", "Balanced", "Quality", "Ultra Quality" };

				const char* selectedQuality = "Auto";

				if (Config::Instance()->OverrideQuality.has_value())
					selectedQuality = quality[Config::Instance()->OverrideQuality.value() - 100];

				if (ImGui::BeginCombo("Quality", selectedQuality))
				{
					if (ImGui::Selectable(quality[0], !Config::Instance()->OverrideQuality.has_value()))
					{
						Config::Instance()->OverrideQuality.reset();
						Config::Instance()->changeBackend = true;
					}

					for (int n = 101; n < 105; n++)
					{
						if (ImGui::Selectable(quality[n - 100], (Config::Instance()->OverrideQuality.has_value() && Config::Instance()->OverrideQuality == n)))
						{
							Config::Instance()->OverrideQuality = n;
							Config::Instance()->changeBackend = true;
						}
					}

					ImGui::EndCombo();
				}
				ImGui::EndDisabled();

				// FSR -----------------
				ImGui::BeginDisabled(currentBackend == "xess");
				ImGui::SeparatorText("FSR Settings");

				bool useVFov = Config::Instance()->FsrVerticalFov.has_value() || !Config::Instance()->FsrHorizontalFov.has_value();

				float fov;

				if (useVFov)
					fov = Config::Instance()->FsrVerticalFov.value_or(60.0f);

				if (ImGui::RadioButton("Use Vert. Fov", useVFov))
					useVFov = true;

				ImGui::SameLine();

				if (ImGui::RadioButton("Use Horz. Fov", !useVFov))
					useVFov = false;

				if (useVFov)
				{
					Config::Instance()->FsrHorizontalFov.reset();
					ImGui::SliderFloat("Vert. FOV", &fov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat);
					Config::Instance()->FsrVerticalFov = fov;
				}
				else
				{
					Config::Instance()->FsrVerticalFov.reset();
					ImGui::SliderFloat("Horz. FOV", &fov, 0.0f, 180.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat);
					Config::Instance()->FsrHorizontalFov = fov;
				}

				// Dx11
				ImGui::BeginDisabled((currentBackend != "fsr21_12") && (currentBackend != "fsr22_12"));

				const char* sync[] = { "Shared Fences", "Shared Fences + Flush", "Shared Fences + Query", "Only Queries" };

				const char* selectedSync = sync[Config::Instance()->UseSafeSyncQueries.value_or(0)];

				if (ImGui::BeginCombo("Sync", selectedSync))
				{
					for (int n = 0; n < 4; n++)
					{
						if (ImGui::Selectable(sync[n], (Config::Instance()->UseSafeSyncQueries.value_or(0) == n)))
							Config::Instance()->UseSafeSyncQueries = n;
					}

					ImGui::EndCombo();
				}

				ImGui::EndDisabled();

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

				float qUp;

				if (currentBackend == "xess")
					qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or(2.5f);
				else
					qUp = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0f);

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
				ImGui::SeparatorText("Resource Barriers");
				ImGui::BeginDisabled(Config::Instance()->Api != NVNGX_DX12);
				
				AddResourceBarrier("Color", &Config::Instance()->ColorResourceBarrier);
				AddResourceBarrier("Depth", &Config::Instance()->DepthResourceBarrier);
				AddResourceBarrier("Motion", &Config::Instance()->MVResourceBarrier);
				AddResourceBarrier("Exposure", &Config::Instance()->ExposureResourceBarrier);
				AddResourceBarrier("Mask", &Config::Instance()->MaskResourceBarrier);
				AddResourceBarrier("Output", &Config::Instance()->OutputResourceBarrier);

				ImGui::EndDisabled();

				ImGui::EndTable();

				// BOTTOM LINE ---------------
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

				ImGui::SameLine(ImGui::GetWindowWidth() - 55.0f);

				if (ImGui::Button("Close"))
					_isVisible = false;

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
	}

	ImGui::Render();
}

bool Imgui_Base::IsHandleDifferent()
{
	HWND frontWindow = GetForegroundWindow();

	if (frontWindow == _handle)
		return false;

	DWORD procId;
	GetWindowThreadProcessId(frontWindow, &procId);

	return (processId == procId);
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
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

	io.MouseDrawCursor = _isVisible;
	io.WantCaptureKeyboard = _isVisible;
	io.WantCaptureMouse = _isVisible;
	io.WantSetMousePos = _isVisible;

	_handle = handle;

	_baseInit = ImGui_ImplWin32_Init(_handle);

	if (_oWndProc == nullptr)
		_oWndProc = (WNDPROC)SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)WndProc);
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
		SetWindowLongPtr((HWND)ImGui::GetMainViewport()->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)_oWndProc);
		_oWndProc = nullptr;

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