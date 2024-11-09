#include "WorkingMode.h"

#include "Util.h"
#include "Config.h"
#include "exports/Exports.h"

typedef const char* (CDECL* PFN_wine_get_version)(void);

static bool IsRunningOnWine()
{
    LOG_FUNC();

    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");

    if (!ntdll)
    {
        LOG_WARN("Not running on NT!?!");
        return true;
    }

    auto pWineGetVersion = (PFN_wine_get_version)GetProcAddress(ntdll, "wine_get_version");

    if (pWineGetVersion)
    {
        LOG_INFO("Running on Wine {0}!", pWineGetVersion());
        return true;
    }

    LOG_WARN("Wine not detected");
    return false;
}

bool WorkingMode::Check()
{
    LOG_FUNC();

    // Check for Wine
    Config::Instance()->IsRunningOnLinux = IsRunningOnWine();

    bool modeFound = false;
    
    filename = Util::DllPath().filename().string();
    lCaseFilename = filename;
    
    wchar_t sysFolder[MAX_PATH];
    GetSystemDirectory(sysFolder, MAX_PATH);
    std::filesystem::path sysPath(sysFolder);
    std::filesystem::path pluginPath(Config::Instance()->PluginPath.value_or((Util::DllPath().parent_path() / L"plugins").wstring()));

    for (size_t i = 0; i < lCaseFilename.size(); i++)
        lCaseFilename[i] = std::tolower(lCaseFilename[i]);

    HMODULE dll = nullptr;

    do
    {
        if (lCaseFilename == "nvngx.dll" || lCaseFilename == "_nvngx.dll" || lCaseFilename == "libxess.dll" || lCaseFilename == "dlss-enabler-upscaler.dll")
        {
            LOG_INFO("OptiScaler working as native upscaler: {0}", filename);

            dllNames.push_back("OptiScaler_DontLoad.dll");
            dllNames.push_back("OptiScaler_DontLoad");
            dllNamesW.push_back(L"OptiScaler_DontLoad.dll");
            dllNamesW.push_back(L"OptiScaler_DontLoad");

            isNvngxMode = true;
            isWorkingWithEnabler = lCaseFilename == "dlss-enabler-upscaler.dll";

            if (isWorkingWithEnabler)
                Config::Instance()->LogToNGX = true;

            modeFound = true;
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
                dllNames.push_back("version.dll");
                dllNames.push_back("version");
                dllNamesW.push_back(L"version.dll");
                dllNamesW.push_back(L"version");

                shared.LoadOriginalLibrary(dll);
                version.LoadOriginalLibrary(dll);

                modeFound = true;
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
                dllNames.push_back("winmm.dll");
                dllNames.push_back("winmm");
                dllNamesW.push_back(L"winmm.dll");
                dllNamesW.push_back(L"winmm");

                shared.LoadOriginalLibrary(dll);
                winmm.LoadOriginalLibrary(dll);
                modeFound = true;
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
                dllNames.push_back("wininet.dll");
                dllNames.push_back("wininet");
                dllNamesW.push_back(L"wininet.dll");
                dllNamesW.push_back(L"wininet");

                shared.LoadOriginalLibrary(dll);
                wininet.LoadOriginalLibrary(dll);
                modeFound = true;
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

            dllNames.push_back("optiscaler.asi");
            dllNames.push_back("optiscaler");
            dllNamesW.push_back(L"optiscaler.asi");
            dllNamesW.push_back(L"optiscaler");

            modeFound = true;
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
                dllNames.push_back("winhttp.dll");
                dllNames.push_back("winhttp");
                dllNamesW.push_back(L"winhttp.dll");
                dllNamesW.push_back(L"winhttp");

                shared.LoadOriginalLibrary(dll);
                winhttp.LoadOriginalLibrary(dll);
                modeFound = true;
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
                dllNames.push_back("dxgi.dll");
                dllNames.push_back("dxgi");
                dllNamesW.push_back(L"dxgi.dll");
                dllNamesW.push_back(L"dxgi");

                dxgi.LoadOriginalLibrary(dll);

                Config::Instance()->IsDxgiMode = true;
                isDxgiMode = true;
                modeFound = true;
            }
            else
            {
                spdlog::error("OptiScaler can't find original dxgi.dll!");
            }

            break;
        }

    } while (false);

    if (modeFound)
    {
        Config::Instance()->OverlayMenu = (!isNvngxMode || isWorkingWithEnabler) && Config::Instance()->OverlayMenu.value_or(true);
        return true;
    }

    LOG_ERROR("Unsupported dll name: {0}", filename);
    return false;
}
