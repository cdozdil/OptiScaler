#pragma once
#include "pch.h"
#include "Config.h"
#include "Util.h"

static inline int64_t GetTicks()
{
    LARGE_INTEGER ticks;

    if (!QueryPerformanceCounter(&ticks))
        return 0;

    return ticks.QuadPart;
}

static inline bool isInteger(const std::string& str, int& value) {
    std::istringstream iss(str);
    return (iss >> value) && iss.eof();
}

static inline bool isUInt(const std::string& str, uint32_t& value) {
    std::istringstream iss(str);
    return (iss >> value) && iss.eof();
}

static inline bool isFloat(const std::string& str, float& value) {
    std::istringstream iss(str);
    return (iss >> value) && iss.eof();
}

Config::Config(std::wstring fileName)
{
    absoluteFileName = Util::DllPath().parent_path() / fileName;;
    Reload(absoluteFileName);
}

bool Config::Reload(std::filesystem::path iniPath)
{
    auto pathWStr = iniPath.wstring();

    LOG_INFO("Trying to load ini from: {0}", wstring_to_string(pathWStr));
    if (ini.LoadFile(iniPath.c_str()) == SI_OK)
    {
        // Upscalers
        {
            Dx11Upscaler = readString("Upscalers", "Dx11Upscaler", true);
            Dx12Upscaler = readString("Upscalers", "Dx12Upscaler", true);
            VulkanUpscaler = readString("Upscalers", "VulkanUpscaler", true);
        }

        // FSR
        {
            FsrVerticalFov = readFloat("FSR", "VerticalFov");
            FsrHorizontalFov = readFloat("FSR", "HorizontalFov");
            FsrCameraNear = readFloat("FSR", "CameraNear");
            FsrCameraFar = readFloat("FSR", "CameraFar");
            FsrVelocity = readFloat("FSR", "VelocityFactor");
            FsrDebugView = readBool("FSR", "DebugView");
            Fsr3xIndex = readInt("FSR", "UpscalerIndex");
            FsrUseMaskForTransparency = readBool("FSR", "UseReactiveMaskForTransparency");
            DlssReactiveMaskBias = readFloat("FSR", "DlssReactiveMaskBias");
        }

        // XeSS
        {
            BuildPipelines = readBool("XeSS", "BuildPipelines");
            NetworkModel = readInt("XeSS", "NetworkModel");
            CreateHeaps = readBool("XeSS", "CreateHeaps");

            auto xessLibraryPathA = readString("XeSS", "LibraryPath");

            if (xessLibraryPathA.has_value())
                XeSSLibrary = string_to_wstring(xessLibraryPathA.value());
        }

        // DLSS
        {
            // Don't enable again if set false because of no nvngx found
            if (!DLSSEnabled.has_value())
                DLSSEnabled = readBool("DLSS", "Enabled");

            auto dlssLibraryPathA = readString("DLSS", "LibraryPath");
            if (dlssLibraryPathA.has_value())
                DLSSLibrary = string_to_wstring(dlssLibraryPathA.value());

            dlssLibraryPathA = readString("DLSS", "NVNGX_DLSS_Path");
            if (dlssLibraryPathA.has_value())
                NVNGX_DLSS_Library = string_to_wstring(dlssLibraryPathA.value());

            RenderPresetOverride = readBool("DLSS", "RenderPresetOverride");
            RenderPresetDLAA = readInt("DLSS", "RenderPresetDLAA");
            RenderPresetUltraQuality = readInt("DLSS", "RenderPresetUltraQuality");
            RenderPresetQuality = readInt("DLSS", "RenderPresetQuality");
            RenderPresetBalanced = readInt("DLSS", "RenderPresetBalanced");
            RenderPresetPerformance = readInt("DLSS", "RenderPresetPerformance");
            RenderPresetUltraPerformance = readInt("DLSS", "RenderPresetUltraPerformance");

            if (RenderPresetDLAA.has_value() && (RenderPresetDLAA.value() < 0 || RenderPresetDLAA.value() > 7))
                RenderPresetDLAA.reset();

            if (RenderPresetUltraQuality.has_value() && (RenderPresetUltraQuality.value() < 0 || RenderPresetUltraQuality.value() > 7))
                RenderPresetUltraQuality.reset();

            if (RenderPresetQuality.has_value() && (RenderPresetQuality.value() < 0 || RenderPresetQuality.value() > 7))
                RenderPresetQuality.reset();

            if (RenderPresetBalanced.has_value() && (RenderPresetBalanced.value() < 0 || RenderPresetBalanced.value() > 7))
                RenderPresetBalanced.reset();

            if (RenderPresetPerformance.has_value() && (RenderPresetPerformance.value() < 0 || RenderPresetPerformance.value() > 7))
                RenderPresetPerformance.reset();

            if (RenderPresetUltraPerformance.has_value() && (RenderPresetUltraPerformance.value() < 0 || RenderPresetUltraPerformance.value() > 7))
                RenderPresetUltraPerformance.reset();
        }


        // Logging
        {
            LogLevel = readInt("Log", "LogLevel");
            LogToConsole = readBool("Log", "LogToConsole");
            LogToFile = readBool("Log", "LogToFile");
            LogToNGX = readBool("Log", "LogToNGX");
            OpenConsole = readBool("Log", "OpenConsole");
            DebugWait = readBool("Log", "DebugWait");

            auto logFileA = readString("Log", "LogFile");

            if (logFileA.has_value())
                LogFileName = string_to_wstring(logFileA.value());

            LogSingleFile = readBool("Log", "SingleFile");

            if (!LogFileName.has_value())
            {
                if (LogSingleFile.value_or(true))
                {
                    auto logFile = Util::DllPath().parent_path() / "OptiScaler.log";
                    LogFileName = logFile.wstring();
                }
                else
                {
                    auto logFile = Util::DllPath().parent_path() / ("OptiScaler_" + std::to_string(GetTicks()) + ".log");
                    LogFileName = logFile.wstring();
                }
            }
        }

        // Sharpness
        {
            OverrideSharpness = readBool("Sharpness", "OverrideSharpness");
            Sharpness = readFloat("Sharpness", "Sharpness");
        }

        // Menu
        {
            MenuScale = readFloat("Menu", "Scale");

            // Don't enable again if set false because of Linux issue
            if (!OverlayMenu.has_value())
                OverlayMenu = readBool("Menu", "OverlayMenu");

            ShortcutKey = readInt("Menu", "ShortcutKey");
            AdvancedSettings = readBool("Menu", "AdvancedSettings");
            ExtendedLimits = readBool("Menu", "ExtendedLimits");
        }

        // Hooks
        {
            HookOriginalNvngxOnly = readBool("Hooks", "HookOriginalNvngxOnly");
        }

        // RCAS
        {
            RcasEnabled = readBool("CAS", "Enabled");
            MotionSharpnessEnabled = readBool("CAS", "MotionSharpnessEnabled");
            MotionSharpness = readFloat("CAS", "MotionSharpness");
            MotionSharpnessDebug = readBool("CAS", "MotionSharpnessDebug");
            MotionThreshold = readFloat("CAS", "MotionThreshold");
            MotionScaleLimit = readFloat("CAS", "MotionScaleLimit");

            if (Sharpness.has_value())
            {
                if (Sharpness.value() > 1.3f)
                    Sharpness = 1.3f;
                else if (Sharpness.value() < 0.0f)
                    Sharpness.reset();
            }

            if (MotionSharpness.has_value())
            {
                if (MotionSharpness.value() > 1.3f)
                    MotionSharpness = 1.3f;
                else if (MotionSharpness.value() < -1.3f)
                    MotionSharpness = -1.3f;
            }

            if (MotionThreshold.has_value())
            {
                if (MotionThreshold.value() > 100.0f)
                    MotionThreshold = 100.0f;
                else if (MotionThreshold.value() < 0.0f)
                    MotionThreshold.reset();
            }

            if (MotionScaleLimit.has_value())
            {
                if (MotionScaleLimit.value() > 100.0f)
                    MotionScaleLimit = 100.0f;
                else if (MotionScaleLimit.value() < 0.01f)
                    MotionScaleLimit.reset();
            }
        }

        // Output Scaling
        {
            OutputScalingEnabled = readBool("OutputScaling", "Enabled");
            OutputScalingMultiplier = readFloat("OutputScaling", "Multiplier");
            OutputScalingUseFsr = readBool("OutputScaling", "UseFsr");

            if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() < 0.5f)
                OutputScalingMultiplier = 0.5f;
            else if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() > 3.0f)
                OutputScalingMultiplier = 3.0f;
        }

        // Init Flags
        {
            AutoExposure = readBool("InitFlags", "AutoExposure");
            HDR = readBool("InitFlags", "HDR");
            DepthInverted = readBool("InitFlags", "DepthInverted");
            JitterCancellation = readBool("InitFlags", "JitterCancellation");
            DisplayResolution = readBool("InitFlags", "DisplayResolution");
            DisableReactiveMask = readBool("InitFlags", "DisableReactiveMask");
        }


        // DRS
        {
            DrsMinOverrideEnabled = readBool("DRS", "DrsMinOverrideEnabled");
            DrsMaxOverrideEnabled = readBool("DRS", "DrsMaxOverrideEnabled");
        }

        //Upscale Ratio Override
        {
            UpscaleRatioOverrideEnabled = readBool("UpscaleRatio", "UpscaleRatioOverrideEnabled");
            UpscaleRatioOverrideValue = readFloat("UpscaleRatio", "UpscaleRatioOverrideValue");
        }

        // Quality Overrides
        {
            QualityRatioOverrideEnabled = readBool("QualityOverrides", "QualityRatioOverrideEnabled");
            QualityRatio_DLAA = readFloat("QualityOverrides", "QualityRatioDLAA");
            QualityRatio_UltraQuality = readFloat("QualityOverrides", "QualityRatioUltraQuality");
            QualityRatio_Quality = readFloat("QualityOverrides", "QualityRatioQuality");
            QualityRatio_Balanced = readFloat("QualityOverrides", "QualityRatioBalanced");
            QualityRatio_Performance = readFloat("QualityOverrides", "QualityRatioPerformance");
            QualityRatio_UltraPerformance = readFloat("QualityOverrides", "QualityRatioUltraPerformance");
        }

        // Hotfixes
        {
            RoundInternalResolution = readInt("Hotfix", "RoundInternalResolution");

            MipmapBiasOverride = readFloat("Hotfix", "MipmapBiasOverride");
            if (MipmapBiasOverride.has_value() && (MipmapBiasOverride.value() > 15.0 || MipmapBiasOverride.value() < -15.0))
                MipmapBiasOverride.reset();

            AnisotropyOverride = readInt("Hotfix", "AnisotropyOverride");
            if (AnisotropyOverride.has_value() && (AnisotropyOverride.value() > 16 || AnisotropyOverride.value() < 1))
                AnisotropyOverride.reset();

            RestoreComputeSignature = readBool("Hotfix", "RestoreComputeSignature");
            RestoreComputeSignature = readBool("Hotfix", "RestoreComputeSignature");
            SkipFirstFrames = readInt("Hotfix", "SkipFirstFrames");

            UsePrecompiledShaders = readBool("Hotfix", "UsePrecompiledShaders");

            UseGenericAppIdWithDlss = readBool("Hotfix", "UseGenericAppIdWithDlss");

            ColorResourceBarrier = readInt("Hotfix", "ColorResourceBarrier");
            MVResourceBarrier = readInt("Hotfix", "MotionVectorResourceBarrier");
            DepthResourceBarrier = readInt("Hotfix", "DepthResourceBarrier");
            MaskResourceBarrier = readInt("Hotfix", "ColorMaskResourceBarrier");
            ExposureResourceBarrier = readInt("Hotfix", "ExposureResourceBarrier");
            OutputResourceBarrier = readInt("Hotfix", "OutputResourceBarrier");
        }

        // Dx11 with Dx12
        {
            TextureSyncMethod = readInt("Dx11withDx12", "TextureSyncMethod");
            CopyBackSyncMethod = readInt("Dx11withDx12", "CopyBackSyncMethod");
            Dx11DelayedInit = readInt("Dx11withDx12", "UseDelayedInit");
            SyncAfterDx12 = readInt("Dx11withDx12", "SyncAfterDx12");
        }

        // NvApi
        {
            OverrideNvapiDll = readBool("NvApi", "OverrideNvapiDll");

            auto nvapiPathA = readString("NvApi", "NvapiDllPath", true);

            if (nvapiPathA.has_value())
                NvapiDllPath = string_to_wstring(nvapiPathA.value());
        }

        // Spoofing
        {
            DxgiSpoofing = readBool("Spoofing", "Dxgi");
            DxgiBlacklist = readString("Spoofing", "DxgiBlacklist");
            DxgiVRAM = readInt("Spoofing", "DxgiVRAM");
            VulkanSpoofing = readBool("Spoofing", "Vulkan");
            VulkanExtensionSpoofing = readBool("Spoofing", "VulkanExtensionSpoofing");

            auto gpuName = readString("Spoofing", "SpoofedGPUName");
            if (gpuName.has_value())
                SpoofedGPUName = string_to_wstring(gpuName.value());
        }

        // Plugins
        {

            auto pluginsPathA = readString("Plugins", "Path", true);

            if (!pluginsPathA.has_value())
            {
                auto pluginFolder = (Util::DllPath().parent_path() / "plugins");
                PluginPath = pluginFolder.wstring();
            }
            else
            {
                PluginPath = string_to_wstring(pluginsPathA.value());
            }

            LoadSpecialK = readBool("Plugins", "LoadSpecialK");
        }

        // DLSS Enabler
        {
            std::optional<std::string> buffer;
            int value = 0;

            DE_Generator = readString("FrameGeneration", "Generator", true);

            if (DE_Generator.has_value() && DE_Generator.value() != "fsr30" && DE_Generator.value() != "fsr31" && DE_Generator.value() != "dlssg")
                DE_Generator.reset();

            buffer = readString("FrameGeneration", "FramerateLimit", true);
            if (buffer.has_value())
            {
                if (buffer.value() == "vsync")
                {
                    DE_FramerateLimit = 0;
                    DE_FramerateLimitVsync = true;
                }
                else if (isInteger(buffer.value(), value))
                {
                    DE_FramerateLimit = value;
                    DE_FramerateLimitVsync = false;
                }
                else
                {
                    DE_FramerateLimit = 0;
                    DE_FramerateLimitVsync = false;
                }
            }

            buffer.reset();
            buffer = readString("FrameGeneration", "FrameGenerationMode", true);
            if (buffer.has_value() && buffer.value() == "dynamic")
            {
                DE_DynamicLimitAvailable = 1;
                DE_DynamicLimitEnabled = 1;
            }

            DE_Reflex = readString("FrameGeneration", "Reflex", true);

            if (DE_Reflex.has_value() && DE_Reflex.value() != "off" && DE_Reflex.value() != "boost" && DE_Reflex.value() != "on")
                DE_Reflex.reset();

            DE_ReflexEmu = readString("FrameGeneration", "ReflexEmulation", true);

            if (DE_ReflexEmu.has_value() && DE_ReflexEmu.value() != "off" && DE_ReflexEmu.value() != "on")
                DE_ReflexEmu.reset();
        }

        return true;
    }

    return false;
}

bool Config::LoadFromPath(const wchar_t* InPath)
{
    std::filesystem::path iniPath(InPath);
    auto newPath = iniPath / L"nvngx.ini";

    if (Reload(newPath))
    {
        absoluteFileName = newPath;
        return true;
    }

    return false;
}

std::string GetBoolValue(std::optional<bool> value)
{
    if (!value.has_value())
        return "auto";

    return value.value() ? "true" : "false";
}

std::string GetIntValue(std::optional<int> value)
{
    if (!value.has_value())
        return "auto";

    return std::to_string(value.value());
}

std::string GetFloatValue(std::optional<float> value)
{
    if (!value.has_value())
        return "auto";

    return std::to_string(value.value());
}

bool Config::SaveIni()
{
    // DLSS Enabler
    if (DE_Available)
    {
        ini.SetValue("FrameGeneration", "Generator", Instance()->DE_Generator.value_or("auto").c_str());
        ini.SetValue("FrameGeneration", "Reflex", Instance()->DE_Reflex.value_or("on").c_str());
        ini.SetValue("FrameGeneration", "ReflexEmulation", Instance()->DE_ReflexEmu.value_or("auto").c_str());

        if (DE_FramerateLimitVsync.has_value() && DE_FramerateLimitVsync.value())
            ini.SetValue("FrameGeneration", "FramerateLimit", "vsync");
        else if (!DE_FramerateLimit.has_value() || (DE_FramerateLimit.has_value() && DE_FramerateLimit.value() < 1))
            ini.SetValue("FrameGeneration", "FramerateLimit", "off");
        else
            ini.SetValue("FrameGeneration", "FramerateLimit", std::to_string(DE_FramerateLimit.value()).c_str());

        if (DE_DynamicLimitEnabled.has_value() && DE_DynamicLimitEnabled.value())
            ini.SetValue("FrameGeneration", "FrameGenerationMode", "dynamic");
        else
            ini.SetValue("FrameGeneration", "FrameGenerationMode", "auto");
    }

    // Upscalers 
    {
        ini.SetValue("Upscalers", "Dx11Upscaler", Instance()->Dx11Upscaler.value_or("auto").c_str());
        ini.SetValue("Upscalers", "Dx12Upscaler", Instance()->Dx12Upscaler.value_or("auto").c_str());
        ini.SetValue("Upscalers", "VulkanUpscaler", Instance()->VulkanUpscaler.value_or("auto").c_str());
    }

    // Output Scaling
    {
        ini.SetValue("OutputScaling", "Enabled", GetBoolValue(Instance()->OutputScalingEnabled).c_str());
        ini.SetValue("OutputScaling", "Multiplier", GetFloatValue(Instance()->OutputScalingMultiplier).c_str());
        ini.SetValue("OutputScaling", "UseFsr", GetBoolValue(Instance()->OutputScalingUseFsr).c_str());
    }

    // FSR
    {
        ini.SetValue("FSR", "VerticalFov", GetFloatValue(Instance()->FsrVerticalFov).c_str());
        ini.SetValue("FSR", "HorizontalFov", GetFloatValue(Instance()->FsrHorizontalFov).c_str());
        ini.SetValue("FSR", "CameraNear", GetFloatValue(Instance()->FsrCameraNear).c_str());
        ini.SetValue("FSR", "CameraFar", GetFloatValue(Instance()->FsrCameraFar).c_str());
        ini.SetValue("FSR", "VelocityFactor", GetFloatValue(Instance()->FsrVelocity).c_str());
        ini.SetValue("FSR", "DebugView", GetBoolValue(Instance()->FsrDebugView).c_str());
        ini.SetValue("FSR", "UpscalerIndex", GetIntValue(Instance()->Fsr3xIndex).c_str());
        ini.SetValue("FSR", "UseReactiveMaskForTransparency", GetBoolValue(Instance()->FsrUseMaskForTransparency).c_str());
        ini.SetValue("FSR", "DlssReactiveMaskBias", GetFloatValue(Instance()->DlssReactiveMaskBias).c_str());
    }

    // XeSS
    {
        ini.SetValue("XeSS", "BuildPipelines", GetBoolValue(Instance()->BuildPipelines).c_str());
        ini.SetValue("XeSS", "CreateHeaps", GetBoolValue(Instance()->CreateHeaps).c_str());
        ini.SetValue("XeSS", "NetworkModel", GetIntValue(Instance()->NetworkModel).c_str());
        ini.SetValue("XeSS", "LibraryPath", Instance()->XeSSLibrary.has_value() ? wstring_to_string(Instance()->XeSSLibrary.value()).c_str() : "auto");
    }

    // DLSS
    {
        ini.SetValue("DLSS", "Enabled", GetBoolValue(Instance()->DLSSEnabled).c_str());
        ini.SetValue("DLSS", "LibraryPath", Instance()->DLSSLibrary.has_value() ? wstring_to_string(Instance()->DLSSLibrary.value()).c_str() : "auto");
        ini.SetValue("DLSS", "NVNGX_DLSS_Library", Instance()->NVNGX_DLSS_Library.has_value() ? wstring_to_string(Instance()->NVNGX_DLSS_Library.value()).c_str() : "auto");
        ini.SetValue("DLSS", "RenderPresetOverride", GetBoolValue(Instance()->RenderPresetOverride).c_str());
        ini.SetValue("DLSS", "RenderPresetDLAA", GetIntValue(Instance()->RenderPresetDLAA).c_str());
        ini.SetValue("DLSS", "RenderPresetUltraQuality", GetIntValue(Instance()->RenderPresetUltraQuality).c_str());
        ini.SetValue("DLSS", "RenderPresetQuality", GetIntValue(Instance()->RenderPresetQuality).c_str());
        ini.SetValue("DLSS", "RenderPresetBalanced", GetIntValue(Instance()->RenderPresetBalanced).c_str());
        ini.SetValue("DLSS", "RenderPresetPerformance", GetIntValue(Instance()->RenderPresetPerformance).c_str());
        ini.SetValue("DLSS", "RenderPresetUltraPerformance", GetIntValue(Instance()->RenderPresetUltraPerformance).c_str());
    }

    // Sharpness
    {
        ini.SetValue("Sharpness", "OverrideSharpness", GetBoolValue(Instance()->OverrideSharpness).c_str());
        ini.SetValue("Sharpness", "Sharpness", GetFloatValue(Instance()->Sharpness).c_str());
    }

    // Menu
    {
        ini.SetValue("Menu", "Scale", GetFloatValue(Instance()->MenuScale).c_str());
        ini.SetValue("Menu", "OverlayMenu", GetBoolValue(Instance()->OverlayMenu).c_str());
        ini.SetValue("Menu", "ShortcutKey", GetIntValue(Instance()->ShortcutKey).c_str());
        ini.SetValue("Menu", "AdvancedSettings", GetBoolValue(Instance()->AdvancedSettings).c_str());
        ini.SetValue("Menu", "ExtendedLimits", GetBoolValue(Instance()->ExtendedLimits).c_str());
    }

    // Hooks
    {
        ini.SetValue("Hooks", "HookOriginalNvngxOnly", GetBoolValue(Instance()->HookOriginalNvngxOnly).c_str());
    }

    // CAS
    {
        ini.SetValue("CAS", "Enabled", GetBoolValue(Instance()->RcasEnabled).c_str());
        ini.SetValue("CAS", "MotionSharpnessEnabled", GetBoolValue(Instance()->MotionSharpnessEnabled).c_str());
        ini.SetValue("CAS", "MotionSharpnessDebug", GetBoolValue(Instance()->MotionSharpnessDebug).c_str());
        ini.SetValue("CAS", "MotionSharpness", GetFloatValue(Instance()->MotionSharpness).c_str());
        ini.SetValue("CAS", "MotionThreshold", GetFloatValue(Instance()->MotionThreshold).c_str());
        ini.SetValue("CAS", "MotionScaleLimit", GetFloatValue(Instance()->MotionScaleLimit).c_str());
    }

    // InitFlags
    {
        ini.SetValue("InitFlags", "AutoExposure", GetBoolValue(Instance()->AutoExposure).c_str());
        ini.SetValue("InitFlags", "HDR", GetBoolValue(Instance()->HDR).c_str());
        ini.SetValue("InitFlags", "DepthInverted", GetBoolValue(Instance()->DepthInverted).c_str());
        ini.SetValue("InitFlags", "JitterCancellation", GetBoolValue(Instance()->JitterCancellation).c_str());
        ini.SetValue("InitFlags", "DisplayResolution", GetBoolValue(Instance()->DisplayResolution).c_str());
        ini.SetValue("InitFlags", "DisableReactiveMask", GetBoolValue(Instance()->DisableReactiveMask).c_str());
    }

    // Upscale Ratio Override
    {
        ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideEnabled", GetBoolValue(Instance()->UpscaleRatioOverrideEnabled).c_str());
        ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideValue", GetFloatValue(Instance()->UpscaleRatioOverrideValue).c_str());
    }

    // Quality Overrides
    {
        ini.SetValue("QualityOverrides", "QualityRatioOverrideEnabled", GetBoolValue(Instance()->QualityRatioOverrideEnabled).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioDLAA", GetFloatValue(Instance()->QualityRatio_DLAA).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioUltraQuality", GetFloatValue(Instance()->QualityRatio_UltraQuality).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioQuality", GetFloatValue(Instance()->QualityRatio_Quality).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioBalanced", GetFloatValue(Instance()->QualityRatio_Balanced).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioPerformance", GetFloatValue(Instance()->QualityRatio_Performance).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioUltraPerformance", GetFloatValue(Instance()->QualityRatio_UltraPerformance).c_str());
    }

    // Hotfixes
    {
        ini.SetValue("Hotfix", "MipmapBiasOverride", GetFloatValue(Instance()->MipmapBiasOverride).c_str());
        ini.SetValue("Hotfix", "AnisotropyOverride", GetIntValue(Instance()->AnisotropyOverride).c_str());
        ini.SetValue("Hotfix", "RoundInternalResolution", GetIntValue(Instance()->RoundInternalResolution).c_str());

        ini.SetValue("Hotfix", "RestoreComputeSignature", GetBoolValue(Instance()->RestoreComputeSignature).c_str());
        ini.SetValue("Hotfix", "RestoreGraphicSignature", GetBoolValue(Instance()->RestoreGraphicSignature).c_str());
        ini.SetValue("Hotfix", "SkipFirstFrames", GetIntValue(Instance()->SkipFirstFrames).c_str());

        ini.SetValue("Hotfix", "UsePrecompiledShaders", GetBoolValue(Instance()->UsePrecompiledShaders).c_str());

        ini.SetValue("Hotfix", "UseGenericAppIdWithDlss", GetBoolValue(Instance()->UseGenericAppIdWithDlss).c_str());

        ini.SetValue("Hotfix", "ColorResourceBarrier", GetIntValue(Instance()->ColorResourceBarrier).c_str());
        ini.SetValue("Hotfix", "MotionVectorResourceBarrier", GetIntValue(Instance()->MVResourceBarrier).c_str());
        ini.SetValue("Hotfix", "DepthResourceBarrier", GetIntValue(Instance()->DepthResourceBarrier).c_str());
        ini.SetValue("Hotfix", "ColorMaskResourceBarrier", GetIntValue(Instance()->MaskResourceBarrier).c_str());
        ini.SetValue("Hotfix", "ExposureResourceBarrier", GetIntValue(Instance()->ExposureResourceBarrier).c_str());
        ini.SetValue("Hotfix", "OutputResourceBarrier", GetIntValue(Instance()->OutputResourceBarrier).c_str());
    }

    // Dx11 with Dx12
    {
        ini.SetValue("Dx11withDx12", "TextureSyncMethod", GetIntValue(Instance()->TextureSyncMethod).c_str());
        ini.SetValue("Dx11withDx12", "CopyBackSyncMethod", GetIntValue(Instance()->CopyBackSyncMethod).c_str());
        ini.SetValue("Dx11withDx12", "SyncAfterDx12", GetBoolValue(Instance()->SyncAfterDx12).c_str());
        ini.SetValue("Dx11withDx12", "UseDelayedInit", GetBoolValue(Instance()->Dx11DelayedInit).c_str());
        ini.SetValue("Dx11withDx12", "DontUseNTShared", GetBoolValue(Instance()->DontUseNTShared).c_str());
    }

    // Logging
    {
        ini.SetValue("Log", "LogLevel", GetIntValue(Instance()->LogLevel).c_str());
        ini.SetValue("Log", "LogToConsole", GetBoolValue(Instance()->LogToConsole).c_str());
        ini.SetValue("Log", "LogToFile", GetBoolValue(Instance()->LogToFile).c_str());
        ini.SetValue("Log", "LogToNGX", GetBoolValue(Instance()->LogToNGX).c_str());
        ini.SetValue("Log", "OpenConsole", GetBoolValue(Instance()->OpenConsole).c_str());
        ini.SetValue("Log", "LogFile", Instance()->LogFileName.has_value() ? wstring_to_string(Instance()->LogFileName.value()).c_str() : "auto");
        ini.SetValue("Log", "SingleFile", GetBoolValue(Instance()->LogSingleFile).c_str());
    }

    // NvApi
    {
        ini.SetValue("NvApi", "OverrideNvapiDll", GetBoolValue(Instance()->OverrideNvapiDll).c_str());
        ini.SetValue("NvApi", "NvapiDllPath", Instance()->NvapiDllPath.has_value() ? wstring_to_string(Instance()->NvapiDllPath.value()).c_str() : "auto");
    }

    // DRS
    {
        ini.SetValue("DRS", "DrsMinOverrideEnabled", GetBoolValue(Instance()->DrsMinOverrideEnabled).c_str());
        ini.SetValue("DRS", "DrsMaxOverrideEnabled", GetBoolValue(Instance()->DrsMaxOverrideEnabled).c_str());
    }

    // Spoofing
    {
        ini.SetValue("Spoofing", "Dxgi", GetBoolValue(Instance()->DxgiSpoofing).c_str());
        ini.SetValue("Spoofing", "DxgiBlacklist", Instance()->DxgiBlacklist.value_or("auto").c_str());
        ini.SetValue("Spoofing", "Vulkan", GetBoolValue(Instance()->VulkanSpoofing).c_str());
        ini.SetValue("Spoofing", "VulkanExtensionSpoofing", GetBoolValue(Instance()->VulkanExtensionSpoofing).c_str());
        ini.SetValue("Spoofing", "DxgiVRAM", GetIntValue(Instance()->DxgiVRAM).c_str());
        ini.SetValue("Spoofing", "SpoofedGPUName", Instance()->SpoofedGPUName.has_value() ? wstring_to_string(Instance()->SpoofedGPUName.value()).c_str() : "auto");
    }

    // Plugins
    {
        ini.SetValue("Plugins", "Path", Instance()->PluginPath.has_value() ? wstring_to_string(Instance()->PluginPath.value()).c_str() : "auto");
        ini.SetValue("Plugins", "LoadSpecialK", GetBoolValue(Instance()->LoadSpecialK).c_str());
    }

    auto pathWStr = absoluteFileName.wstring();

    LOG_INFO("Trying to save ini to: {0}", wstring_to_string(pathWStr));

    return ini.SaveFile(absoluteFileName.wstring().c_str()) >= 0;
}

std::optional<std::string> Config::readString(std::string section, std::string key, bool lowercase)
{
    std::string value = ini.GetValue(section.c_str(), key.c_str(), "auto");

    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "auto")
        return std::nullopt;

    return lowercase ? lower : value;
}

std::optional<float> Config::readFloat(std::string section, std::string key)
{
    auto value = readString(section, key);

    try
    {
        float result;

        if (value.has_value() && isFloat(value.value(), result))
            return result;

        return std::nullopt;
    }
    catch (const std::bad_optional_access&) // missing or auto value
    {
        return std::nullopt;
    }
    catch (const std::invalid_argument&) // invalid float string for std::stof
    {
        return std::nullopt;
    }
    catch (const std::out_of_range&) // out of range for 32 bit float
    {
        return std::nullopt;
    }
}

std::optional<int> Config::readInt(std::string section, std::string key)
{
    auto value = readString(section, key);
    try
    {
        int result;

        if (value.has_value() && isInteger(value.value(), result))
            return result;

        return std::nullopt;
    }
    catch (const std::bad_optional_access&) // missing or auto value
    {
        return std::nullopt;
    }
    catch (const std::invalid_argument&) // invalid float string for std::stof
    {
        return std::nullopt;
    }
    catch (const std::out_of_range&) // out// out of range for 32 bit float
    {
        return std::nullopt;
    }
}

std::optional<uint32_t> Config::readUInt(std::string section, std::string key)
{
    auto value = readString(section, key);
    try
    {
        uint32_t result;

        if (value.has_value() && isUInt(value.value(), result))
            return result;

        return std::nullopt;
    }
    catch (const std::bad_optional_access&) // missing or auto value
    {
        return std::nullopt;
    }
    catch (const std::invalid_argument&) // invalid float string for std::stof
    {
        return std::nullopt;
    }
    catch (const std::out_of_range&) // out// out of range for 32 bit float
    {
        return std::nullopt;
    }
}

std::optional<bool> Config::readBool(std::string section, std::string key)
{
    auto value = readString(section, key, true);
    if (value == "true")
    {
        return true;
    }
    else if (value == "false")
    {
        return false;
    }

    return std::nullopt;
}

Config* Config::Instance()
{
    if (!_config)
        _config = new Config(L"nvngx.ini");

    return _config;
}