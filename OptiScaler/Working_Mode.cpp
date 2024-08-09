#include "Working_Mode.h"
#include "Config.h"
#include "Util.h"
#include "NVNGX_Proxy.h"
#include "exports/Exports.h"

typedef const char* (CDECL* PFN_wine_get_version)(void);

static std::string _dllNameA;
static std::string _dllNameExA;
static std::wstring _dllNameW;
static std::wstring _dllNameExW;

static bool _isNvngxMode = false;
static bool _isWorkingWithEnabler = false;
static bool _isNvngxAvailable = false;
static bool _isModeFound = false;

static bool IsRunningOnWine()
{
    LOG_FUNC();

    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

    if (!ntdll)
    {
        spdlog::warn("IsRunningOnWine Not running on NT!?!");
        return true;
    }

    auto pWineGetVersion = (PFN_wine_get_version)GetProcAddress(ntdll, "wine_get_version");

    if (pWineGetVersion)
    {
        LOG_INFO("IsRunningOnWine Running on Wine {0}!", pWineGetVersion());
        return true;
    }

    spdlog::warn("IsRunningOnWine Wine not detected");
    return false;
}

void WorkingMode::Check()
{
    LOG_FUNC();

    auto config = Config::Instance();
    
    config->IsRunningOnLinux = IsRunningOnWine();

    std::string filename = Util::DllPath().filename().string();
    std::string lCaseFilename(filename);
    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);
    std::filesystem::path pluginPath(config->PluginPath.value_or((Util::DllPath().parent_path() / L"plugins").wstring()));

    for (size_t i = 0; i < lCaseFilename.size(); i++)
        lCaseFilename[i] = std::tolower(lCaseFilename[i]);

    HMODULE dll = nullptr;

    do
    {
        if (lCaseFilename == "nvngx.dll" || lCaseFilename == "_nvngx.dll" || lCaseFilename == "dlss-enabler-upscaler.dll")
        {
            LOG_INFO("OptiScaler working as native upscaler: {0}", filename);

            _dllNameA = "OptiScaler_DontLoad.dll";
            _dllNameExA = "OptiScaler_DontLoad";
            _dllNameW = L"OptiScaler_DontLoad.dll";
            _dllNameExW = L"OptiScaler_DontLoad";

            _isNvngxMode = true;
            _isWorkingWithEnabler = lCaseFilename == "dlss-enabler-upscaler.dll";

            if (_isWorkingWithEnabler)
                config->LogToNGX = true;

            _isModeFound = true;
            break;
        }

        // version.dll
        if (lCaseFilename == "version.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"version.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"version-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as version.dll, version-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"version.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as version.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                _dllNameA = "version.dll";
                _dllNameExA = "version";
                _dllNameW = L"version.dll";
                _dllNameExW = L"version";

                shared.LoadOriginalLibrary(dll);
                version.LoadOriginalLibrary(dll);

                _isModeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original version.dll!");
            }

            break;
        }

        // winmm.dll
        if (lCaseFilename == "winmm.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"winmm.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winmm-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winmm.dll, winmm-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winmm.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winmm.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                _dllNameA = "winmm.dll";
                _dllNameExA = "winmm";
                _dllNameW = L"winmm.dll";
                _dllNameExW = L"winmm";

                shared.LoadOriginalLibrary(dll);
                winmm.LoadOriginalLibrary(dll);
                _isModeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winmm.dll!");
            }

            break;
        }

        // wininet.dll
        if (lCaseFilename == "wininet.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"wininet.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"wininet-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as wininet.dll, wininet-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"wininet.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as wininet.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                _dllNameA = "wininet.dll";
                _dllNameExA = "wininet";
                _dllNameW = L"wininet.dll";
                _dllNameExW = L"wininet";

                shared.LoadOriginalLibrary(dll);
                wininet.LoadOriginalLibrary(dll);
                _isModeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original wininet.dll!");
            }

            break;
        }

        // optiscaler.dll
        if (lCaseFilename == "optiscaler.asi")
        {
            LOG_INFO("OptiScaler working as OptiScaler.asi");

            // quick hack for testing
            dll = dllModule;

            _dllNameA = "optiscaler.asi";
            _dllNameExA = "optiscaler";
            _dllNameW = L"optiscaler.asi";
            _dllNameExW = L"optiscaler";

            _isModeFound = true;
            break;
        }

        // winhttp.dll
        if (lCaseFilename == "winhttp.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"winhttp.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"winhttp-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as winhttp.dll, winhttp-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"winhttp.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as winhttp.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                _dllNameA = "winhttp.dll";
                _dllNameExA = "winhttp";
                _dllNameW = L"winhttp.dll";
                _dllNameExW = L"winhttp";

                shared.LoadOriginalLibrary(dll);
                winhttp.LoadOriginalLibrary(dll);
                _isModeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original winhttp.dll!");
            }

            break;
        }

        // dxgi.dll
        if (lCaseFilename == "dxgi.dll")
        {
            do
            {
                auto pluginFilePath = pluginPath / L"dxgi.dll";
                dll = LoadLibrary(pluginFilePath.wstring().c_str());

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, original dll loaded from plugin folder");
                    break;
                }

                dll = LoadLibrary(L"dxgi-original.dll");

                if (dll != nullptr)
                {
                    LOG_INFO("OptiScaler working as dxgi.dll, dxgi-original.dll loaded");
                    break;
                }

                auto sysFilePath = sysPath / L"dxgi.dll";
                dll = LoadLibrary(sysFilePath.wstring().c_str());

                if (dll != nullptr)
                    LOG_INFO("OptiScaler working as dxgi.dll, system dll loaded");

            } while (false);

            if (dll != nullptr)
            {
                _dllNameA = "dxgi.dll";
                _dllNameExA = "dxgi";
                _dllNameW = L"dxgi.dll";
                _dllNameExW = L"dxgi";

                shared.LoadOriginalLibrary(dll);
                dxgi.LoadOriginalLibrary(dll);

                config->IsDxgiMode = true;
                _isModeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original dxgi.dll!");
            }

            break;
        }

    } while (false);

    if (_isModeFound)
    {
        // Check if real DLSS available
        if (config->DLSSEnabled.value_or(true))
        {
            spdlog::info("");
            NVNGXProxy::InitNVNGX();

            if (NVNGXProxy::NVNGXModule() == nullptr)
            {
                LOG_INFO("Can't load nvngx.dll, disabling DLSS");
                config->DLSSEnabled = false;
            }
            else
            {
                LOG_INFO("nvngx.dll loaded, setting DLSS as default upscaler and disabling spoofing options set to auto");

                config->DLSSEnabled = true;

                if (!config->DxgiSpoofing.has_value())
                    config->DxgiSpoofing = false;

                if (!config->VulkanSpoofing.has_value())
                    config->VulkanSpoofing = false;

                if (!config->VulkanExtensionSpoofing.has_value())
                    config->VulkanExtensionSpoofing = false;

                // Disable Overlay Menu because of crashes on Linux with Nvidia GPUs
                if (config->IsRunningOnLinux && !config->OverlayMenu.has_value())
                    config->OverlayMenu = false;

                _isNvngxAvailable = true;
            }
        }
    }

    LOG_ERROR("Unsupported dll name: {0}", filename);
}

std::string WorkingMode::DllNameA()
{
    return _dllNameA;
}

std::string WorkingMode::DllNameExA()
{
    return _dllNameExA;
}

std::wstring WorkingMode::DllNameW()
{
    return _dllNameW;
}

std::wstring WorkingMode::DllNameExW()
{
    return _dllNameExW;
}

bool WorkingMode::IsNvngxMode()
{
    return _isNvngxMode;
}

bool WorkingMode::IsWorkingWithEnabler()
{
    return _isWorkingWithEnabler;
}

bool WorkingMode::IsNvngxAvailable()
{
    return _isNvngxAvailable;
}

bool WorkingMode::IsModeFound()
{
    return _isModeFound;
}
