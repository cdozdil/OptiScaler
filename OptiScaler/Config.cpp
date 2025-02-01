#include "pch.h"
#include "Config.h"
#include "Util.h"
#include "nvapi/fakenvapi.h"

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

Config::Config()
{
    absoluteFileName = Util::DllPath().parent_path() / fileName;
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
            Dx11Upscaler.set_from_config(readString("Upscalers", "Dx11Upscaler", true));
            Dx12Upscaler.set_from_config(readString("Upscalers", "Dx12Upscaler", true));
            VulkanUpscaler.set_from_config(readString("Upscalers", "VulkanUpscaler", true));
        }

        // Frame Generation
        {
            FGUseFGSwapChain.set_from_config(readBool("FrameGen", "UseFGSwapChain"));
            FGEnabled.set_from_config(readBool("FrameGen", "Enabled"));
            FGDebugView.set_from_config(readBool("FrameGen", "DebugView"));
            FGAsync.set_from_config(readBool("FrameGen", "AllowAsync"));
            FGHighPriority.set_from_config(readBool("FrameGen", "HighPriority"));
            FGHUDFix.set_from_config(readBool("FrameGen", "HUDFix"));
            FGHUDLimit.set_from_config(readInt("FrameGen", "HUDLimit"));
            FGHUDFixExtended.set_from_config(readBool("FrameGen", "HUDFixExtended"));
            FGImmediateCapture.set_from_config(readBool("FrameGen", "HUDFixImmadiate"));
            FGRectLeft.set_from_config(readInt("FrameGen", "RectLeft"));
            FGRectTop.set_from_config(readInt("FrameGen", "RectTop"));
            FGRectWidth.set_from_config(readInt("FrameGen", "RectWidth"));
            FGRectHeight.set_from_config(readInt("FrameGen", "RectHeight"));
            FGDisableOverlays.set_from_config(readBool("FrameGen", "DisableOverlays"));
            FGAlwaysTrackHeaps.set_from_config(readBool("FrameGen", "AlwaysTrackHeaps"));
            FGMakeDepthCopy.set_from_config(readBool("FrameGen", "MakeDepthCopy"));
            FGMakeMVCopy.set_from_config(readBool("FrameGen", "MakeMVCopy"));
            FGHudFixCloseAfterCallback.set_from_config(readBool("FrameGen", "HudFixCloseAfterCallback"));
            FGUseMutexForSwaphain.set_from_config(readBool("FrameGen", "UseMutexForSwaphain"));

            FGFramePacingTuning.set_from_config(readBool("FrameGen", "FramePacingTuning"));
            FGFPTSafetyMarginInMs.set_from_config(readFloat("FrameGen", "FPTSafetyMarginInMs"));
            FGFPTVarianceFactor.set_from_config(readFloat("FrameGen", "FPTVarianceFactor"));
            FGFPTAllowHybridSpin.set_from_config(readBool("FrameGen", "FPTHybridSpin"));
            FGFPTHybridSpinTime.set_from_config(readInt("FrameGen", "FPTHybridSpinTime"));
            FGFPTAllowWaitForSingleObjectOnFence.set_from_config(readInt("FrameGen", "FPTWaitForSingleObjectOnFence"));
        }

        // Framerate
        {
            FramerateLimit.set_from_config(readFloat("Framerate", "FramerateLimit"));
        }

        // FSR Common
        {
            FsrVerticalFov.set_from_config(readFloat("FSR", "VerticalFov"));
            FsrHorizontalFov.set_from_config(readFloat("FSR", "HorizontalFov"));
            FsrCameraNear.set_from_config(readFloat("FSR", "CameraNear"));
            FsrCameraFar.set_from_config(readFloat("FSR", "CameraFar"));
            FsrUseFsrInputValues.set_from_config(readBool("FSR", "UseFsrInputValues"));
        }

        // FSR
        {
            FsrVelocity.set_from_config(readFloat("FSR", "VelocityFactor"));
            FsrDebugView.set_from_config(readBool("FSR", "DebugView"));
            Fsr3xIndex.set_from_config(readInt("FSR", "UpscalerIndex"));
            FsrUseMaskForTransparency.set_from_config(readBool("FSR", "UseReactiveMaskForTransparency"));
            DlssReactiveMaskBias.set_from_config(readFloat("FSR", "DlssReactiveMaskBias"));
        }

        // XeSS
        {
            BuildPipelines.set_from_config(readBool("XeSS", "BuildPipelines"));
            NetworkModel.set_from_config(readInt("XeSS", "NetworkModel"));
            CreateHeaps.set_from_config(readBool("XeSS", "CreateHeaps"));
            XeSSLibrary.set_from_config(readWString("XeSS", "LibraryPath"));
        }

        // DLSS
        {
            // Don't enable again if set false because of no nvngx found
            DLSSEnabled.set_from_config(readBool("DLSS", "Enabled"));
            NvngxPath.set_from_config(readWString("DLSS", "LibraryPath"));
            NVNGX_DLSS_Library.set_from_config(readWString("DLSS", "NVNGX_DLSS_Path"));

            RenderPresetOverride.set_from_config(readBool("DLSS", "RenderPresetOverride"));

            if (auto setting = readInt("DLSS", "RenderPresetDLAA"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetDLAA.set_from_config(setting);

            if (auto setting = readInt("DLSS", "RenderPresetUltraQuality"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetUltraQuality.set_from_config(setting);

            if (auto setting = readInt("DLSS", "RenderPresetQuality"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetQuality.set_from_config(setting);

            if (auto setting = readInt("DLSS", "RenderPresetBalanced"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetBalanced.set_from_config(setting);

            if (auto setting = readInt("DLSS", "RenderPresetPerformance"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetPerformance.set_from_config(setting);

            if (auto setting = readInt("DLSS", "RenderPresetUltraPerformance"); setting.has_value() && setting >= 0 && setting <= 15)
                RenderPresetUltraPerformance.set_from_config(setting);
        }

        // DLSSG
        {
            SpoofHAGS.set_from_config(readBool("DLSSG", "SpoofHAGS"));
            MakeDepthCopy.set_from_config(readBool("DLSSG", "MakeDepthCopy"));
            DLSSGMod.set_from_config(readBool("DLSSG", "DLSSGMod"));
            if (DLSSGMod.value_or(false))
                FGUseFGSwapChain.set_volatile_value(false);
        }

        // Logging
        {
            LogLevel.set_from_config(readInt("Log", "LogLevel"));
            LogToConsole.set_from_config(readBool("Log", "LogToConsole"));
            LogToFile.set_from_config(readBool("Log", "LogToFile"));
            LogToNGX.set_from_config(readBool("Log", "LogToNGX"));
            OpenConsole.set_from_config(readBool("Log", "OpenConsole"));
            DebugWait.set_from_config(readBool("Log", "DebugWait"));
            LogSingleFile.set_from_config(readBool("Log", "SingleFile"));
            LogAsync.set_from_config(readBool("Log", "LogAsync"));
            LogAsyncThreads.set_from_config(readInt("Log", "LogAsyncThreads"));

            {
                auto setting = readString("Log", "LogFile", false);

                // Reproduce the old bug of "LogFile = " always disabling logs
                if (setting.has_value() && setting.value().empty())
                    LogFileName.set_from_config(L"");

                auto path = Util::DllPath() / setting.value_or(wstring_to_string(LogFileName.value_or_default()));
                auto filenameStem = path.stem();

                auto filename = std::filesystem::path(LogSingleFile.value_or_default() ? filenameStem.wstring() + L".log" : filenameStem.wstring() + L"_" + std::to_wstring(GetTicks()) + L".log");

                if (path.has_root_path())
                    LogFileName.set_from_config((path.parent_path() / filename).wstring());
                else
                    LogFileName.set_from_config((Util::DllPath().parent_path() / filename).wstring());
            }
        }

        // Sharpness
        {
            OverrideSharpness.set_from_config(readBool("Sharpness", "OverrideSharpness"));

            if (auto setting = readFloat("Sharpness", "Sharpness"); setting.has_value())
                Sharpness.set_from_config(std::clamp(setting.value(), 0.0f, 1.3f));
        }

        // Menu
        {
            if (auto setting = readFloat("Menu", "Scale"); setting.has_value())
                MenuScale.set_from_config(std::clamp(setting.value(), 0.5f, 2.0f));

            // Don't enable again if set false because of Linux issue
            OverlayMenu.set_from_config(readBool("Menu", "OverlayMenu"));
            ShortcutKey.set_from_config(readInt("Menu", "ShortcutKey"));
            AdvancedSettings.set_from_config(readBool("Menu", "AdvancedSettings"));
            ExtendedLimits.set_from_config(readBool("Menu", "ExtendedLimits"));
            ShowFps.set_from_config(readBool("Menu", "ShowFps"));

            if (auto setting = readInt("Menu", "FpsOverlayPos"); setting.has_value())
                FpsOverlayPos.set_from_config(std::clamp(setting.value(), 0, 3));

            if (auto setting = readInt("Menu", "FpsOverlayType"); setting.has_value())
                FpsOverlayType.set_from_config(std::clamp(setting.value(), 0, 4));

            FpsShortcutKey.set_from_config(readInt("Menu", "FpsShortcutKey"));
            FpsCycleShortcutKey.set_from_config(readInt("Menu", "FpsCycleShortcutKey"));
            FpsOverlayHorizontal.set_from_config(readBool("Menu", "FpsOverlayHorizontal"));

            if (auto setting = readFloat("Menu", "FpsOverlayAlpha"); setting.has_value())
                FpsOverlayAlpha.set_from_config(std::clamp(setting.value(), 0.0f, 1.0f));
        }

        // Hooks
        {
            HookOriginalNvngxOnly.set_from_config(readBool("Hooks", "HookOriginalNvngxOnly"));
        }

        // RCAS
        {
            RcasEnabled.set_from_config(readBool("CAS", "Enabled"));
            MotionSharpnessEnabled.set_from_config(readBool("CAS", "MotionSharpnessEnabled"));
            MotionSharpnessDebug.set_from_config(readBool("CAS", "MotionSharpnessDebug"));

            if (auto setting = readFloat("CAS", "MotionSharpness"); setting.has_value())
                MotionSharpness.set_from_config(std::clamp(setting.value(), -1.3f, 1.3f));

            if (auto setting = readFloat("CAS", "MotionThreshold"); setting.has_value())
                MotionThreshold.set_from_config(std::clamp(setting.value(), 0.0f, 100.0f));

            if (auto setting = readFloat("CAS", "MotionScaleLimit"); setting.has_value())
                MotionScaleLimit.set_from_config(std::clamp(setting.value(), 0.01f, 100.0f));
        }

        // Output Scaling
        {
            OutputScalingEnabled.set_from_config(readBool("OutputScaling", "Enabled"));
            OutputScalingUseFsr.set_from_config(readBool("OutputScaling", "UseFsr"));
            OutputScalingDownscaler.set_from_config(readInt("OutputScaling", "Downscaler"));

            if (auto setting = readFloat("OutputScaling", "Multiplier"); setting.has_value())
                OutputScalingMultiplier.set_from_config(std::clamp(setting.value(), 0.5f, 3.0f));
        }

        // Init Flags
        {
            AutoExposure.set_from_config(readBool("InitFlags", "AutoExposure"));
            HDR.set_from_config(readBool("InitFlags", "HDR"));
            DepthInverted.set_from_config(readBool("InitFlags", "DepthInverted"));
            JitterCancellation.set_from_config(readBool("InitFlags", "JitterCancellation"));
            DisplayResolution.set_from_config(readBool("InitFlags", "DisplayResolution"));
            DisableReactiveMask.set_from_config(readBool("InitFlags", "DisableReactiveMask"));
        }


        // DRS
        {
            DrsMinOverrideEnabled.set_from_config(readBool("DRS", "DrsMinOverrideEnabled"));
            DrsMaxOverrideEnabled.set_from_config(readBool("DRS", "DrsMaxOverrideEnabled"));
        }

        //Upscale Ratio Override
        {
            UpscaleRatioOverrideEnabled.set_from_config(readBool("UpscaleRatio", "UpscaleRatioOverrideEnabled"));
            UpscaleRatioOverrideValue.set_from_config(readFloat("UpscaleRatio", "UpscaleRatioOverrideValue"));
        }

        // Quality Overrides
        {
            QualityRatioOverrideEnabled.set_from_config(readBool("QualityOverrides", "QualityRatioOverrideEnabled"));
            QualityRatio_DLAA.set_from_config(readFloat("QualityOverrides", "QualityRatioDLAA"));
            QualityRatio_UltraQuality.set_from_config(readFloat("QualityOverrides", "QualityRatioUltraQuality"));
            QualityRatio_Quality.set_from_config(readFloat("QualityOverrides", "QualityRatioQuality"));
            QualityRatio_Balanced.set_from_config(readFloat("QualityOverrides", "QualityRatioBalanced"));
            QualityRatio_Performance.set_from_config(readFloat("QualityOverrides", "QualityRatioPerformance"));
            QualityRatio_UltraPerformance.set_from_config(readFloat("QualityOverrides", "QualityRatioUltraPerformance"));
        }

        // Hotfixes
        {
            RoundInternalResolution.set_from_config(readInt("Hotfix", "RoundInternalResolution"));

            if (auto setting = readFloat("Hotfix", "MipmapBiasOverride"); setting.has_value() && setting.value() <= 15.0 && setting.value() >= -15.0)
                MipmapBiasOverride.set_from_config(setting);

            // Unsure if that's needed but it resets invalid MipmapBiasOverride on config reload
            // Unexpected place for it but could be playing a role
            if (MipmapBiasOverride.has_value() && (MipmapBiasOverride.value() > 15.0 || MipmapBiasOverride.value() < -15.0))
                MipmapBiasOverride.reset();

            MipmapBiasFixedOverride.set_from_config(readBool("Hotfix", "MipmapBiasFixedOverride"));
            MipmapBiasScaleOverride.set_from_config(readBool("Hotfix", "MipmapBiasScaleOverride"));
            MipmapBiasOverrideAll.set_from_config(readBool("Hotfix", "MipmapBiasOverrideAll"));

            if (auto setting = readInt("Hotfix", "AnisotropyOverride"); setting.has_value() && setting.value() <= 16 && setting.value() >= 1)
                AnisotropyOverride.set_from_config(setting);

            if (AnisotropyOverride.has_value() && (AnisotropyOverride.value() > 16 || AnisotropyOverride.value() < 1))
                AnisotropyOverride.reset();

            RestoreComputeSignature.set_from_config(readBool("Hotfix", "RestoreComputeSignature"));
            RestoreGraphicSignature.set_from_config(readBool("Hotfix", "RestoreGraphicSignature"));
            PreferDedicatedGpu.set_from_config(readBool("Hotfix", "PreferDedicatedGpu"));
            PreferFirstDedicatedGpu.set_from_config(readBool("Hotfix", "PreferFirstDedicatedGpu"));
            SkipFirstFrames.set_from_config(readInt("Hotfix", "SkipFirstFrames"));
            UsePrecompiledShaders.set_from_config(readBool("Hotfix", "UsePrecompiledShaders"));
            UseGenericAppIdWithDlss.set_from_config(readBool("Hotfix", "UseGenericAppIdWithDlss"));
            ColorResourceBarrier.set_from_config(readInt("Hotfix", "ColorResourceBarrier"));
            MVResourceBarrier.set_from_config(readInt("Hotfix", "MotionVectorResourceBarrier"));
            DepthResourceBarrier.set_from_config(readInt("Hotfix", "DepthResourceBarrier"));
            MaskResourceBarrier.set_from_config(readInt("Hotfix", "ColorMaskResourceBarrier"));
            ExposureResourceBarrier.set_from_config(readInt("Hotfix", "ExposureResourceBarrier"));
            OutputResourceBarrier.set_from_config(readInt("Hotfix", "OutputResourceBarrier"));
        }

        // Dx11 with Dx12
        {
            TextureSyncMethod.set_from_config(readInt("Dx11withDx12", "TextureSyncMethod"));
            CopyBackSyncMethod.set_from_config(readInt("Dx11withDx12", "CopyBackSyncMethod"));
            Dx11DelayedInit.set_from_config(readInt("Dx11withDx12", "UseDelayedInit"));
            SyncAfterDx12.set_from_config(readInt("Dx11withDx12", "SyncAfterDx12"));
        }

        // NvApi
        {
            OverrideNvapiDll.set_from_config(readBool("NvApi", "OverrideNvapiDll"));
            NvapiDllPath.set_from_config(readWString("NvApi", "NvapiDllPath", true));
        }

        // Spoofing
        {
            DxgiSpoofing.set_from_config(readBool("Spoofing", "Dxgi"));
            DxgiBlacklist.set_from_config(readString("Spoofing", "DxgiBlacklist"));
            DxgiVRAM.set_from_config(readInt("Spoofing", "DxgiVRAM"));
            VulkanSpoofing.set_from_config(readBool("Spoofing", "Vulkan"));
            VulkanExtensionSpoofing.set_from_config(readBool("Spoofing", "VulkanExtensionSpoofing"));
            VulkanVRAM.set_from_config(readInt("Spoofing", "VulkanVRAM"));
            SpoofedGPUName.set_from_config(readWString("Spoofing", "SpoofedGPUName"));
        }

        // Inputs
        {
            DlssInputs.set_from_config(readBool("Inputs", "Dlss"));
            XeSSInputs.set_from_config(readBool("Inputs", "XeSS"));
            Fsr2Inputs.set_from_config(readBool("Inputs", "Fsr2"));
            Fsr2Pattern.set_from_config(readBool("Inputs", "Fsr2Pattern"));
            Fsr3Inputs.set_from_config(readBool("Inputs", "Fsr3"));
            Fsr3Pattern.set_from_config(readBool("Inputs", "Fsr3Pattern"));
            FfxInputs.set_from_config(readBool("Inputs", "Ffx"));
        }

        // Plugins
        {
            if (auto setting = readString("Plugins", "Path", true); setting.has_value())
            {
                auto path = std::filesystem::path(setting.value());

                if (path.has_root_path())
                    PluginPath.set_from_config(path.wstring());
                else
                    PluginPath.set_from_config((Util::DllPath().parent_path() / path).wstring());
            }

            LoadSpecialK.set_from_config(readBool("Plugins", "LoadSpecialK"));
            LoadReShade.set_from_config(readBool("Plugins", "LoadReShade"));
        }

        // DLSS Enabler
        {
            std::optional<std::string> buffer;
            int value = 0;

            if (!DE_Generator.has_value())
                DE_Generator = readString("FrameGeneration", "Generator", true);

            if (DE_Generator.has_value() && DE_Generator.value() != "fsr30" && DE_Generator.value() != "fsr31" && DE_Generator.value() != "dlssg")
                DE_Generator.reset();

            if (!DE_FramerateLimit.has_value() || !DE_FramerateLimitVsync.has_value())
            {
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
            }

            if (!DE_DynamicLimitAvailable.has_value() || !DE_DynamicLimitEnabled.has_value())
            {
                buffer.reset();
                buffer = readString("FrameGeneration", "FrameGenerationMode", true);
                if (buffer.has_value() && buffer.value() == "dynamic")
                {
                    DE_DynamicLimitAvailable = 1;
                    DE_DynamicLimitEnabled = 1;
                }
            }

            if (!DE_Reflex.has_value())
                DE_Reflex = readString("FrameGeneration", "Reflex", true);

            if (DE_Reflex.has_value() && DE_Reflex.value() != "off" && DE_Reflex.value() != "boost" && DE_Reflex.value() != "on")
                DE_Reflex.reset();

            if (!DE_ReflexEmu.has_value())
                DE_ReflexEmu = readString("FrameGeneration", "ReflexEmulation", true);

            if (DE_ReflexEmu.has_value() && DE_ReflexEmu.value() != "off" && DE_ReflexEmu.value() != "on")
                DE_ReflexEmu.reset();
        }

        // HDR
        {
                ForceHDR.set_from_config(readBool("HDR", "ForceHDR"));
                UseHDR10.set_from_config(readBool("HDR", "UseHDR10"));
        }

        if (fakenvapi::isUsingFakenvapi())
            return ReloadFakenvapi();

        return true;
    }

    return false;
}

bool Config::LoadFromPath(const wchar_t* InPath)
{
    std::filesystem::path iniPath(InPath);
    auto newPath = iniPath / fileName;

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
    if (State::Instance().enablerAvailable)
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
        ini.SetValue("Upscalers", "Dx11Upscaler", Instance()->Dx11Upscaler.value_for_config_or("auto").c_str());
        ini.SetValue("Upscalers", "Dx12Upscaler", Instance()->Dx12Upscaler.value_for_config_or("auto").c_str());
        ini.SetValue("Upscalers", "VulkanUpscaler", Instance()->VulkanUpscaler.value_for_config_or("auto").c_str());
    }

    // Frame Generation 
    {
        ini.SetValue("FrameGen", "UseFGSwapChain", GetBoolValue(Instance()->FGUseFGSwapChain.value_for_config()).c_str());
        ini.SetValue("FrameGen", "Enabled", GetBoolValue(Instance()->FGEnabled.value_for_config()).c_str());
        ini.SetValue("FrameGen", "DebugView", GetBoolValue(Instance()->FGDebugView.value_for_config()).c_str());
        ini.SetValue("FrameGen", "AllowAsync", GetBoolValue(Instance()->FGAsync.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HighPriority", GetBoolValue(Instance()->FGHighPriority.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HUDFix", GetBoolValue(Instance()->FGHUDFix.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HUDLimit", GetIntValue(Instance()->FGHUDLimit.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HUDFixExtended", GetBoolValue(Instance()->FGHUDFixExtended.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HUDFixImmadiate", GetBoolValue(Instance()->FGImmediateCapture.value_for_config()).c_str());
        ini.SetValue("FrameGen", "RectLeft", GetIntValue(Instance()->FGRectLeft.value_for_config()).c_str());
        ini.SetValue("FrameGen", "RectTop", GetIntValue(Instance()->FGRectTop.value_for_config()).c_str());
        ini.SetValue("FrameGen", "RectWidth", GetIntValue(Instance()->FGRectWidth.value_for_config()).c_str());
        ini.SetValue("FrameGen", "RectHeight", GetIntValue(Instance()->FGRectHeight.value_for_config()).c_str());
        ini.SetValue("FrameGen", "DisableOverlays", GetBoolValue(Instance()->FGDisableOverlays.value_for_config()).c_str());
        ini.SetValue("FrameGen", "AlwaysTrackHeaps", GetBoolValue(Instance()->FGAlwaysTrackHeaps.value_for_config()).c_str());
        ini.SetValue("FrameGen", "MakeDepthCopy", GetBoolValue(Instance()->FGMakeDepthCopy.value_for_config()).c_str());
        ini.SetValue("FrameGen", "MakeMVCopy", GetBoolValue(Instance()->FGMakeMVCopy.value_for_config()).c_str());
        ini.SetValue("FrameGen", "HudFixCloseAfterCallback", GetBoolValue(Instance()->FGHudFixCloseAfterCallback.value_for_config()).c_str());
        ini.SetValue("FrameGen", "UseMutexForSwaphain", GetBoolValue(Instance()->FGUseMutexForSwaphain.value_for_config()).c_str());

        ini.SetValue("FrameGen", "FramePacingTuning", GetBoolValue(Instance()->FGFramePacingTuning.value_for_config()).c_str());
        ini.SetValue("FrameGen", "FPTSafetyMarginInMs", GetFloatValue(Instance()->FGFPTSafetyMarginInMs.value_for_config()).c_str());
        ini.SetValue("FrameGen", "FPTVarianceFactor", GetFloatValue(Instance()->FGFPTVarianceFactor.value_for_config()).c_str());
        ini.SetValue("FrameGen", "FPTHybridSpin", GetBoolValue(Instance()->FGFPTAllowHybridSpin.value_for_config()).c_str());
        ini.SetValue("FrameGen", "FPTHybridSpinTime", GetIntValue(Instance()->FGFPTHybridSpinTime.value_for_config()).c_str());
        ini.SetValue("FrameGen", "FPTWaitForSingleObjectOnFence", GetBoolValue(Instance()->FGFPTAllowWaitForSingleObjectOnFence.value_for_config()).c_str());
    }

    // Framerate 
    {
        ini.SetValue("Framerate", "FramerateLimit", GetFloatValue(Instance()->FramerateLimit.value_for_config()).c_str());
    }

    // Output Scaling
    {
        ini.SetValue("OutputScaling", "Enabled", GetBoolValue(Instance()->OutputScalingEnabled.value_for_config()).c_str());
        ini.SetValue("OutputScaling", "Multiplier", GetFloatValue(Instance()->OutputScalingMultiplier.value_for_config()).c_str());
        ini.SetValue("OutputScaling", "UseFsr", GetBoolValue(Instance()->OutputScalingUseFsr.value_for_config()).c_str());
        ini.SetValue("OutputScaling", "Downscaler", GetBoolValue(Instance()->OutputScalingDownscaler).c_str());
    }

    // FSR common
    {
        ini.SetValue("FSR", "VerticalFov", GetFloatValue(Instance()->FsrVerticalFov.value_for_config()).c_str());
        ini.SetValue("FSR", "HorizontalFov", GetFloatValue(Instance()->FsrHorizontalFov.value_for_config()).c_str());
        ini.SetValue("FSR", "CameraNear", GetFloatValue(Instance()->FsrCameraNear.value_for_config()).c_str());
        ini.SetValue("FSR", "CameraFar", GetFloatValue(Instance()->FsrCameraFar.value_for_config()).c_str());
        ini.SetValue("FSR", "UseFsrInputValues", GetBoolValue(Instance()->FsrUseFsrInputValues.value_for_config()).c_str());
    }

    // FSR 
    {
        ini.SetValue("FSR", "VelocityFactor", GetFloatValue(Instance()->FsrVelocity.value_for_config()).c_str());
        ini.SetValue("FSR", "DebugView", GetBoolValue(Instance()->FsrDebugView.value_for_config()).c_str());
        ini.SetValue("FSR", "UpscalerIndex", GetIntValue(Instance()->Fsr3xIndex.value_for_config()).c_str());
        ini.SetValue("FSR", "UseReactiveMaskForTransparency", GetBoolValue(Instance()->FsrUseMaskForTransparency.value_for_config()).c_str());
        ini.SetValue("FSR", "DlssReactiveMaskBias", GetFloatValue(Instance()->DlssReactiveMaskBias.value_for_config()).c_str());
    }

    // XeSS
    {
        ini.SetValue("XeSS", "BuildPipelines", GetBoolValue(Instance()->BuildPipelines.value_for_config()).c_str());
        ini.SetValue("XeSS", "CreateHeaps", GetBoolValue(Instance()->CreateHeaps.value_for_config()).c_str());
        ini.SetValue("XeSS", "NetworkModel", GetIntValue(Instance()->NetworkModel.value_for_config()).c_str());
        ini.SetValue("XeSS", "LibraryPath",  wstring_to_string(Instance()->XeSSLibrary.value_for_config_or(L"auto")).c_str());
    }

    // DLSS
    {
        ini.SetValue("DLSS", "Enabled", GetBoolValue(Instance()->DLSSEnabled.value_for_config()).c_str());
        ini.SetValue("DLSS", "LibraryPath", wstring_to_string(Instance()->NvngxPath.value_for_config_or(L"auto")).c_str());
        ini.SetValue("DLSS", "NVNGX_DLSS_Library", wstring_to_string(Instance()->NVNGX_DLSS_Library.value_for_config_or(L"auto")).c_str());
        ini.SetValue("DLSS", "RenderPresetOverride", GetBoolValue(Instance()->RenderPresetOverride.value_for_config()).c_str());
        ini.SetValue("DLSS", "RenderPresetDLAA", GetIntValue(Instance()->RenderPresetDLAA).c_str());
        ini.SetValue("DLSS", "RenderPresetUltraQuality", GetIntValue(Instance()->RenderPresetUltraQuality).c_str());
        ini.SetValue("DLSS", "RenderPresetQuality", GetIntValue(Instance()->RenderPresetQuality).c_str());
        ini.SetValue("DLSS", "RenderPresetBalanced", GetIntValue(Instance()->RenderPresetBalanced).c_str());
        ini.SetValue("DLSS", "RenderPresetPerformance", GetIntValue(Instance()->RenderPresetPerformance).c_str());
        ini.SetValue("DLSS", "RenderPresetUltraPerformance", GetIntValue(Instance()->RenderPresetUltraPerformance).c_str());
    }

    // DLSSG
    {
        ini.SetValue("DLSSG", "SpoofHAGS", GetBoolValue(Instance()->SpoofHAGS.value_for_config()).c_str());
        ini.SetValue("DLSSG", "MakeDepthCopy", GetBoolValue(Instance()->MakeDepthCopy.value_for_config()).c_str());
        ini.SetValue("DLSSG", "DLSSGMod", GetBoolValue(Instance()->DLSSGMod.value_for_config()).c_str());
    }

    // Sharpness
    {
        ini.SetValue("Sharpness", "OverrideSharpness", GetBoolValue(Instance()->OverrideSharpness.value_for_config()).c_str());
        ini.SetValue("Sharpness", "Sharpness", GetFloatValue(Instance()->Sharpness.value_for_config()).c_str());
    }

    // Menu
    {
        ini.SetValue("Menu", "Scale", GetFloatValue(Instance()->MenuScale.value_for_config()).c_str());
        ini.SetValue("Menu", "OverlayMenu", GetBoolValue(Instance()->OverlayMenu.value_for_config()).c_str());
        ini.SetValue("Menu", "ShortcutKey", GetIntValue(Instance()->ShortcutKey.value_for_config()).c_str());
        ini.SetValue("Menu", "AdvancedSettings", GetBoolValue(Instance()->AdvancedSettings.value_for_config()).c_str());
        ini.SetValue("Menu", "ExtendedLimits", GetBoolValue(Instance()->ExtendedLimits.value_for_config()).c_str());
        ini.SetValue("Menu", "ShowFps", GetBoolValue(Instance()->ShowFps.value_for_config()).c_str());
        ini.SetValue("Menu", "FpsOverlayPos", GetIntValue(Instance()->FpsOverlayPos.value_for_config()).c_str());
        ini.SetValue("Menu", "FpsOverlayType", GetIntValue(Instance()->FpsOverlayType.value_for_config()).c_str());
        ini.SetValue("Menu", "FpsOverlayHorizontal", GetBoolValue(Instance()->FpsOverlayHorizontal.value_for_config()).c_str());
        ini.SetValue("Menu", "FpsOverlayAlpha", GetFloatValue(Instance()->FpsOverlayAlpha.value_for_config()).c_str());
    }

    // Hooks
    {
        ini.SetValue("Hooks", "HookOriginalNvngxOnly", GetBoolValue(Instance()->HookOriginalNvngxOnly.value_for_config()).c_str());
    }

    // CAS
    {
        ini.SetValue("CAS", "Enabled", GetBoolValue(Instance()->RcasEnabled.value_for_config()).c_str());
        ini.SetValue("CAS", "MotionSharpnessEnabled", GetBoolValue(Instance()->MotionSharpnessEnabled.value_for_config()).c_str());
        ini.SetValue("CAS", "MotionSharpnessDebug", GetBoolValue(Instance()->MotionSharpnessDebug.value_for_config()).c_str());
        ini.SetValue("CAS", "MotionSharpness", GetFloatValue(Instance()->MotionSharpness.value_for_config()).c_str());
        ini.SetValue("CAS", "MotionThreshold", GetFloatValue(Instance()->MotionThreshold.value_for_config()).c_str());
        ini.SetValue("CAS", "MotionScaleLimit", GetFloatValue(Instance()->MotionScaleLimit.value_for_config()).c_str());
    }

    // InitFlags
    {
        ini.SetValue("InitFlags", "AutoExposure", GetBoolValue(Instance()->AutoExposure.value_for_config()).c_str());
        ini.SetValue("InitFlags", "HDR", GetBoolValue(Instance()->HDR.value_for_config()).c_str());
        ini.SetValue("InitFlags", "DepthInverted", GetBoolValue(Instance()->DepthInverted.value_for_config()).c_str());
        ini.SetValue("InitFlags", "JitterCancellation", GetBoolValue(Instance()->JitterCancellation.value_for_config()).c_str());
        ini.SetValue("InitFlags", "DisplayResolution", GetBoolValue(Instance()->DisplayResolution.value_for_config()).c_str());
        ini.SetValue("InitFlags", "DisableReactiveMask", GetBoolValue(Instance()->DisableReactiveMask.value_for_config()).c_str());
    }

    // Upscale Ratio Override
    {
        ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideEnabled", GetBoolValue(Instance()->UpscaleRatioOverrideEnabled.value_for_config()).c_str());
        ini.SetValue("UpscaleRatio", "UpscaleRatioOverrideValue", GetFloatValue(Instance()->UpscaleRatioOverrideValue.value_for_config()).c_str());
    }

    // Quality Overrides
    {
        ini.SetValue("QualityOverrides", "QualityRatioOverrideEnabled", GetBoolValue(Instance()->QualityRatioOverrideEnabled.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioDLAA", GetFloatValue(Instance()->QualityRatio_DLAA.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioUltraQuality", GetFloatValue(Instance()->QualityRatio_UltraQuality.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioQuality", GetFloatValue(Instance()->QualityRatio_Quality.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioBalanced", GetFloatValue(Instance()->QualityRatio_Balanced.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioPerformance", GetFloatValue(Instance()->QualityRatio_Performance.value_for_config()).c_str());
        ini.SetValue("QualityOverrides", "QualityRatioUltraPerformance", GetFloatValue(Instance()->QualityRatio_UltraPerformance.value_for_config()).c_str());
    }

    // Hotfixes
    {
        ini.SetValue("Hotfix", "MipmapBiasOverride", GetFloatValue(Instance()->MipmapBiasOverride.value_for_config()).c_str());
        ini.SetValue("Hotfix", "MipmapBiasOverrideAll", GetBoolValue(Instance()->MipmapBiasOverrideAll.value_for_config()).c_str());
        ini.SetValue("Hotfix", "MipmapBiasFixedOverride", GetBoolValue(Instance()->MipmapBiasFixedOverride.value_for_config()).c_str());
        ini.SetValue("Hotfix", "MipmapBiasScaleOverride", GetBoolValue(Instance()->MipmapBiasScaleOverride.value_for_config()).c_str());

        ini.SetValue("Hotfix", "AnisotropyOverride", GetIntValue(Instance()->AnisotropyOverride.value_for_config()).c_str());
        ini.SetValue("Hotfix", "RoundInternalResolution", GetIntValue(Instance()->RoundInternalResolution.value_for_config()).c_str());

        ini.SetValue("Hotfix", "RestoreComputeSignature", GetBoolValue(Instance()->RestoreComputeSignature.value_for_config()).c_str());
        ini.SetValue("Hotfix", "RestoreGraphicSignature", GetBoolValue(Instance()->RestoreGraphicSignature.value_for_config()).c_str());
        ini.SetValue("Hotfix", "SkipFirstFrames", GetIntValue(Instance()->SkipFirstFrames.value_for_config()).c_str());

        ini.SetValue("Hotfix", "UsePrecompiledShaders", GetBoolValue(Instance()->UsePrecompiledShaders.value_for_config()).c_str());
        ini.SetValue("Hotfix", "PreferDedicatedGpu", GetBoolValue(Instance()->PreferDedicatedGpu.value_for_config()).c_str());
        ini.SetValue("Hotfix", "PreferFirstDedicatedGpu", GetBoolValue(Instance()->PreferFirstDedicatedGpu.value_for_config()).c_str());

        ini.SetValue("Hotfix", "UseGenericAppIdWithDlss", GetBoolValue(Instance()->UseGenericAppIdWithDlss.value_for_config()).c_str());

        ini.SetValue("Hotfix", "ColorResourceBarrier", GetIntValue(Instance()->ColorResourceBarrier.value_for_config()).c_str());
        ini.SetValue("Hotfix", "MotionVectorResourceBarrier", GetIntValue(Instance()->MVResourceBarrier.value_for_config()).c_str());
        ini.SetValue("Hotfix", "DepthResourceBarrier", GetIntValue(Instance()->DepthResourceBarrier.value_for_config()).c_str());
        ini.SetValue("Hotfix", "ColorMaskResourceBarrier", GetIntValue(Instance()->MaskResourceBarrier.value_for_config()).c_str());
        ini.SetValue("Hotfix", "ExposureResourceBarrier", GetIntValue(Instance()->ExposureResourceBarrier.value_for_config()).c_str());
        ini.SetValue("Hotfix", "OutputResourceBarrier", GetIntValue(Instance()->OutputResourceBarrier.value_for_config()).c_str());
    }

    // Dx11 with Dx12
    {
        ini.SetValue("Dx11withDx12", "TextureSyncMethod", GetIntValue(Instance()->TextureSyncMethod.value_for_config()).c_str());
        ini.SetValue("Dx11withDx12", "CopyBackSyncMethod", GetIntValue(Instance()->CopyBackSyncMethod.value_for_config()).c_str());
        ini.SetValue("Dx11withDx12", "SyncAfterDx12", GetBoolValue(Instance()->SyncAfterDx12.value_for_config()).c_str());
        ini.SetValue("Dx11withDx12", "UseDelayedInit", GetBoolValue(Instance()->Dx11DelayedInit.value_for_config()).c_str());
        ini.SetValue("Dx11withDx12", "DontUseNTShared", GetBoolValue(Instance()->DontUseNTShared.value_for_config()).c_str());
    }

    // Logging
    {
        ini.SetValue("Log", "LogLevel", GetIntValue(Instance()->LogLevel.value_for_config()).c_str());
        ini.SetValue("Log", "LogToConsole", GetBoolValue(Instance()->LogToConsole.value_for_config()).c_str());
        ini.SetValue("Log", "LogToFile", GetBoolValue(Instance()->LogToFile.value_for_config()).c_str());
        ini.SetValue("Log", "LogToNGX", GetBoolValue(Instance()->LogToNGX.value_for_config()).c_str());
        ini.SetValue("Log", "OpenConsole", GetBoolValue(Instance()->OpenConsole.value_for_config()).c_str());
        ini.SetValue("Log", "LogFile", wstring_to_string(Instance()->LogFileName.value_for_config_or(L"auto")).c_str());
        ini.SetValue("Log", "SingleFile", GetBoolValue(Instance()->LogSingleFile.value_for_config()).c_str());
        ini.SetValue("Log", "LogAsync", GetBoolValue(Instance()->LogAsync.value_for_config()).c_str());
        ini.SetValue("Log", "LogAsyncThreads", GetIntValue(Instance()->LogAsyncThreads.value_for_config()).c_str());
    }

    // NvApi
    {
        ini.SetValue("NvApi", "OverrideNvapiDll", GetBoolValue(Instance()->OverrideNvapiDll.value_for_config()).c_str());
        ini.SetValue("NvApi", "NvapiDllPath", wstring_to_string(Instance()->NvapiDllPath.value_for_config_or(L"auto")).c_str());
    }

    // DRS
    {
        ini.SetValue("DRS", "DrsMinOverrideEnabled", GetBoolValue(Instance()->DrsMinOverrideEnabled.value_for_config()).c_str());
        ini.SetValue("DRS", "DrsMaxOverrideEnabled", GetBoolValue(Instance()->DrsMaxOverrideEnabled.value_for_config()).c_str());
    }

    // Spoofing
    {
        ini.SetValue("Spoofing", "Dxgi", GetBoolValue(Instance()->DxgiSpoofing.value_for_config()).c_str());
        ini.SetValue("Spoofing", "DxgiBlacklist", Instance()->DxgiBlacklist.value_for_config_or("auto").c_str());
        ini.SetValue("Spoofing", "Vulkan", GetBoolValue(Instance()->VulkanSpoofing.value_for_config()).c_str());
        ini.SetValue("Spoofing", "VulkanExtensionSpoofing", GetBoolValue(Instance()->VulkanExtensionSpoofing.value_for_config()).c_str());
        ini.SetValue("Spoofing", "VulkanVRAM", GetIntValue(Instance()->VulkanVRAM.value_for_config()).c_str());
        ini.SetValue("Spoofing", "DxgiVRAM", GetIntValue(Instance()->DxgiVRAM.value_for_config()).c_str());
        ini.SetValue("Spoofing", "SpoofedGPUName", wstring_to_string(Instance()->SpoofedGPUName.value_for_config_or(L"auto")).c_str());
    }

    // Plugins
    {
        
        ini.SetValue("Plugins", "Path", wstring_to_string(Instance()->PluginPath.value_for_config_or(L"auto")).c_str());
        ini.SetValue("Plugins", "LoadSpecialK", GetBoolValue(Instance()->LoadSpecialK.value_for_config()).c_str());
    }

    // inputs
    {
        ini.SetValue("Inputs", "Dlss", GetBoolValue(Instance()->DlssInputs.value_for_config()).c_str());
        ini.SetValue("Inputs", "XeSS", GetBoolValue(Instance()->XeSSInputs.value_for_config()).c_str());
        ini.SetValue("Inputs", "Fsr2", GetBoolValue(Instance()->Fsr2Inputs.value_for_config()).c_str());
        ini.SetValue("Inputs", "Fsr2Pattern", GetBoolValue(Instance()->Fsr2Pattern.value_for_config()).c_str());
        ini.SetValue("Inputs", "Fsr3", GetBoolValue(Instance()->Fsr3Inputs.value_for_config()).c_str());
        ini.SetValue("Inputs", "Fsr3Pattern", GetBoolValue(Instance()->Fsr3Pattern.value_for_config()).c_str());
        ini.SetValue("Inputs", "Ffx", GetBoolValue(Instance()->FfxInputs.value_for_config()).c_str());
    }

    auto pathWStr = absoluteFileName.wstring();

    LOG_INFO("Trying to save ini to: {0}", wstring_to_string(pathWStr));

    return ini.SaveFile(absoluteFileName.wstring().c_str()) >= 0;
}

bool Config::ReloadFakenvapi() {
    auto FN_iniPath = Util::DllPath().parent_path() / L"fakenvapi.ini";
    if (NvapiDllPath.has_value())
        FN_iniPath = std::filesystem::path(NvapiDllPath.value()).parent_path() / L"fakenvapi.ini";

    auto pathWStr = FN_iniPath.wstring();

    LOG_INFO("Trying to load fakenvapi's ini from: {0}", wstring_to_string(pathWStr));

    if (fakenvapiIni.LoadFile(FN_iniPath.c_str()) == SI_OK)
    {
        FN_EnableLogs = fakenvapiIni.GetLongValue("fakenvapi", "enable_logs", true);
        FN_EnableTraceLogs = fakenvapiIni.GetLongValue("fakenvapi", "enable_trace_logs", false);
        FN_ForceLatencyFlex = fakenvapiIni.GetLongValue("fakenvapi", "force_latencyflex", false);
        FN_LatencyFlexMode = fakenvapiIni.GetLongValue("fakenvapi", "latencyflex_mode", 0);
        FN_ForceReflex = fakenvapiIni.GetLongValue("fakenvapi", "force_reflex", 0);

        return true;
    }

    return false;
}

bool Config::SaveFakenvapiIni() {
    auto FN_iniPath = Util::DllPath().parent_path() / L"fakenvapi.ini";
    if (NvapiDllPath.has_value())
        FN_iniPath = std::filesystem::path(NvapiDllPath.value()).parent_path() / L"fakenvapi.ini";

    auto pathWStr = FN_iniPath.wstring();

    LOG_INFO("Trying to save fakenvapi's ini to: {0}", wstring_to_string(pathWStr));

    fakenvapiIni.SetLongValue("fakenvapi", "enable_logs", FN_EnableLogs.value_or(true));
    fakenvapiIni.SetLongValue("fakenvapi", "enable_trace_logs", FN_EnableTraceLogs.value_or(false));
    fakenvapiIni.SetLongValue("fakenvapi", "force_latencyflex", FN_ForceLatencyFlex.value_or(false));
    fakenvapiIni.SetLongValue("fakenvapi", "latencyflex_mode", FN_LatencyFlexMode.value_or(0));
    fakenvapiIni.SetLongValue("fakenvapi", "force_reflex", FN_ForceReflex.value_or(0));

    return fakenvapiIni.SaveFile(FN_iniPath.wstring().c_str()) >= 0;
}

void Config::CheckUpscalerFiles()
{
    if (!State::Instance().nvngxExists)
        State::Instance().nvngxExists = std::filesystem::exists(Util::ExePath().parent_path() / L"nvngx.dll");

    if (!State::Instance().nvngxExists)
        State::Instance().nvngxExists = std::filesystem::exists(Util::ExePath().parent_path() / L"_nvngx.dll");

    if (!State::Instance().nvngxExists)
    {
        State::Instance().nvngxExists = GetModuleHandle(L"nvngx.dll") != nullptr;

        if (!State::Instance().nvngxExists)
            State::Instance().nvngxExists = GetModuleHandle(L"_nvngx.dll") != nullptr;

        if (State::Instance().nvngxExists)
            LOG_INFO("nvngx.dll found in memory");
        else
            LOG_WARN("nvngx.dll not found!");

    }
    else
    {
        LOG_INFO("nvngx.dll found in game folder");
    }

    State::Instance().libxessExists = std::filesystem::exists(Util::ExePath().parent_path() / L"libxess.dll");
    if (!State::Instance().libxessExists)
    {
        State::Instance().libxessExists = GetModuleHandle(L"libxess.dll") != nullptr;

        if (State::Instance().libxessExists)
            LOG_INFO("libxess.dll found in memory");
        else
            LOG_WARN("libxess.dll not found!");

    }
    else
    {
        LOG_INFO("libxess.dll found in game folder");
    }
}

std::optional<std::string> Config::readString(std::string section, std::string key, bool lowercase)
{
    std::string value = ini.GetValue(section.c_str(), key.c_str(), "auto");

    std::string lower = value;
    std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "auto")
        return std::nullopt;

    return lowercase ? lower : value;
}

std::optional<std::wstring> Config::readWString(std::string section, std::string key, bool lowercase)
{
    std::string value = ini.GetValue(section.c_str(), key.c_str(), "auto");

    std::string lower = value;
    std::ranges::transform(lower, lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "auto")
        return std::nullopt;

    return lowercase ? string_to_wstring(lower) : string_to_wstring(value);
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
        _config = new Config();

    return _config;
}