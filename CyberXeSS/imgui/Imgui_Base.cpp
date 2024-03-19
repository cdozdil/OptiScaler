#include "Imgui_Base.h"
#include "../Config.h"

static WNDPROC _oWndProc = nullptr;
static bool _isVisible = false;
static HWND _handle;

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
		ImGui::SetNextWindowSize(ImVec2{ 770.0f, 486.0f }, ImGuiCond_FirstUseEver);

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

				if (Config::Instance()->CasEnabled.value_or(true))
				{
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
				}

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
				ImGui::BeginDisabled((currentBackend == "fsr21_12") || (currentBackend == "fsr22_12"));

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


				// UPSCALE RATIO OVERRIDE
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

				if (bool autoExposure = Config::Instance()->AutoExposure.value_or(false); ImGui::Checkbox("Auto Exposure", &autoExposure))
				{
					Config::Instance()->AutoExposure = autoExposure;
					Config::Instance()->changeBackend = true;
				}

				if (bool hdr = Config::Instance()->HDR.value_or(false); ImGui::Checkbox("HDR", &hdr))
				{
					Config::Instance()->HDR = hdr;
					Config::Instance()->changeBackend = true;
				}

				if (bool depth = Config::Instance()->DepthInverted.value_or(false); ImGui::Checkbox("Depth Inverted", &depth))
				{
					Config::Instance()->DepthInverted = depth;
					Config::Instance()->changeBackend = true;
				}

				if (bool jitter = Config::Instance()->JitterCancellation.value_or(false); ImGui::Checkbox("Jitter Cancellation", &jitter))
				{
					Config::Instance()->JitterCancellation = jitter;
					Config::Instance()->changeBackend = true;
				}

				if (bool mv = Config::Instance()->DisplayResolution.value_or(false); ImGui::Checkbox("Display Res. MV", &mv))
				{
					Config::Instance()->DisplayResolution = mv;
					Config::Instance()->changeBackend = true;
				}

				ImGui::EndTable();


				// BOTTOM LINE

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

Imgui_Base::Imgui_Base(HWND handle)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsLight();

	auto style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.40f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.40f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.70f, 0.70f, 1.00f, 1.00f);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

	_handle = handle;

	_baseInit = ImGui_ImplWin32_Init(_handle);

	//Capture WndProc
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
		SetWindowLongPtr(_handle, GWLP_WNDPROC, (LONG_PTR)_oWndProc);
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