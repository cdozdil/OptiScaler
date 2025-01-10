#pragma once
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
            if (!Dx11Upscaler.has_value())
                Dx11Upscaler = readString("Upscalers", "Dx11Upscaler", true);

            if (!Dx12Upscaler.has_value())
                Dx12Upscaler = readString("Upscalers", "Dx12Upscaler", true);

            if (!VulkanUpscaler.has_value())
                VulkanUpscaler = readString("Upscalers", "VulkanUpscaler", true);
        }

        // Frame Generation
        {
            if (!FGUseFGSwapChain.has_value())
                FGUseFGSwapChain = readBool("FrameGen", "UseFGSwapChain");

            if (!FGEnabled.has_value())
                FGEnabled = readBool("FrameGen", "Enabled");

            if (!FGDebugView.has_value())
                FGDebugView = readBool("FrameGen", "DebugView");

            if (!FGAsync.has_value())
                FGAsync = readBool("FrameGen", "AllowAsync");

            if (!FGHighPriority.has_value())
                FGHighPriority = readBool("FrameGen", "HighPriority");

            if (!FGHUDFix.has_value())
                FGHUDFix = readBool("FrameGen", "HUDFix");

            if (!FGHUDLimit.has_value())
                FGHUDLimit = readInt("FrameGen", "HUDLimit");

            if (!FGHUDFixExtended.has_value())
                FGHUDFixExtended = readBool("FrameGen", "HUDFixExtended");

            if (!FGImmediateCapture.has_value())
                FGImmediateCapture = readBool("FrameGen", "HUDFixImmadiate");

            if (!FGRectLeft.has_value())
                FGRectLeft = readInt("FrameGen", "RectLeft");

            if (!FGRectTop.has_value())
                FGRectTop = readInt("FrameGen", "RectTop");

            if (!FGRectWidth.has_value())
                FGRectWidth = readInt("FrameGen", "RectWidth");

            if (!FGRectHeight.has_value())
                FGRectHeight = readInt("FrameGen", "RectHeight");

            if (!FGDisableOverlays.has_value())
                FGDisableOverlays = readBool("FrameGen", "DisableOverlays");

            if (!FGAlwaysTrackHeaps.has_value())
                FGAlwaysTrackHeaps = readBool("FrameGen", "AlwaysTrackHeaps");

            if (!FGHybridSpin.has_value())
                FGHybridSpin = readBool("FrameGen", "HybridSpin");
        }

        // Framerate
        {
            FramerateLimit = readFloat("Framerate", "FramerateLimit");
        }

        // FSR Common
        {
            if (!FsrVerticalFov.has_value())
                FsrVerticalFov = readFloat("FSR", "VerticalFov");

            if (!FsrHorizontalFov.has_value())
                FsrHorizontalFov = readFloat("FSR", "HorizontalFov");

            if (!FsrCameraNear.has_value())
                FsrCameraNear = readFloat("FSR", "CameraNear");

            if (!FsrCameraFar.has_value())
                FsrCameraFar = readFloat("FSR", "CameraFar");

            if (!FsrUseFsrInputValues.has_value())
                FsrUseFsrInputValues = readBool("FSR", "UseFsrInputValues");
        }

        // FSR
        {
            if (!FsrVelocity.has_value())
                FsrVelocity = readFloat("FSR", "VelocityFactor");

            if (!FsrDebugView.has_value())
                FsrDebugView = readBool("FSR", "DebugView");

            if (!Fsr3xIndex.has_value())
                Fsr3xIndex = readInt("FSR", "UpscalerIndex");

            if (!FsrUseMaskForTransparency.has_value())
                FsrUseMaskForTransparency = readBool("FSR", "UseReactiveMaskForTransparency");

            if (!DlssReactiveMaskBias.has_value())
                DlssReactiveMaskBias = readFloat("FSR", "DlssReactiveMaskBias");
        }

        // XeSS
        {
            if (!BuildPipelines.has_value())
                BuildPipelines = readBool("XeSS", "BuildPipelines");

            if (!NetworkModel.has_value())
                NetworkModel = readInt("XeSS", "NetworkModel");

            if (!CreateHeaps.has_value())
                CreateHeaps = readBool("XeSS", "CreateHeaps");

            if (!XeSSLibrary.has_value())
            {
                auto xessLibraryPathA = readString("XeSS", "LibraryPath");

                if (xessLibraryPathA.has_value())
                    XeSSLibrary = string_to_wstring(xessLibraryPathA.value());
            }
        }

        // DLSS
        {
            // Don't enable again if set false because of no nvngx found
            if (!DLSSEnabled.has_value())
                DLSSEnabled = readBool("DLSS", "Enabled");

            if (!DLSSLibrary.has_value())
            {
                auto dlssLibraryPathA = readString("DLSS", "LibraryPath");
                if (dlssLibraryPathA.has_value())
                    DLSSLibrary = string_to_wstring(dlssLibraryPathA.value());
            }

            if (!NVNGX_DLSS_Library.has_value())
            {
                auto dlssLibraryPathA = readString("DLSS", "NVNGX_DLSS_Path");
                if (dlssLibraryPathA.has_value())
                    NVNGX_DLSS_Library = string_to_wstring(dlssLibraryPathA.value());
            }

            if (!RenderPresetOverride.has_value())
                RenderPresetOverride = readBool("DLSS", "RenderPresetOverride");

            if (!RenderPresetDLAA.has_value())
                RenderPresetDLAA = readInt("DLSS", "RenderPresetDLAA");

            if (!RenderPresetUltraQuality.has_value())
                RenderPresetUltraQuality = readInt("DLSS", "RenderPresetUltraQuality");

            if (!RenderPresetQuality.has_value())
                RenderPresetQuality = readInt("DLSS", "RenderPresetQuality");

            if (!RenderPresetBalanced.has_value())
                RenderPresetBalanced = readInt("DLSS", "RenderPresetBalanced");

            if (!RenderPresetPerformance.has_value())
                RenderPresetPerformance = readInt("DLSS", "RenderPresetPerformance");

            if (!RenderPresetUltraPerformance.has_value())
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
            if (!LogLevel.has_value())
                LogLevel = readInt("Log", "LogLevel");

            if (!LogToConsole.has_value())
                LogToConsole = readBool("Log", "LogToConsole");

            if (!LogToFile.has_value())
                LogToFile = readBool("Log", "LogToFile");

            if (!LogToNGX.has_value())
                LogToNGX = readBool("Log", "LogToNGX");

            if (!OpenConsole.has_value())
                OpenConsole = readBool("Log", "OpenConsole");

            if (!DebugWait.has_value())
                DebugWait = readBool("Log", "DebugWait");

            if (!LogSingleFile.has_value())
                LogSingleFile = readBool("Log", "SingleFile");

            if (!LogFileName.has_value())
            {
                auto logFileA = readString("Log", "LogFile");

                if (logFileA.has_value())
                    LogFileName = string_to_wstring(logFileA.value());
            }

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
            if (!OverrideSharpness.has_value())
                OverrideSharpness = readBool("Sharpness", "OverrideSharpness");

            if (!Sharpness.has_value())
                Sharpness = readFloat("Sharpness", "Sharpness");
        }

        // Menu
        {
            if (!MenuScale.has_value())
                MenuScale = readFloat("Menu", "Scale");

            // Don't enable again if set false because of Linux issue
            if (!OverlayMenu.has_value())
                OverlayMenu = readBool("Menu", "OverlayMenu");

            if (!ShortcutKey.has_value())
                ShortcutKey = readInt("Menu", "ShortcutKey");

            if (!AdvancedSettings.has_value())
                AdvancedSettings = readBool("Menu", "AdvancedSettings");

            if (!ExtendedLimits.has_value())
                ExtendedLimits = readBool("Menu", "ExtendedLimits");

            if (!ShowFps.has_value())
                ShowFps = readBool("Menu", "ShowFps");

            if (!FpsOverlayPos.has_value())
                FpsOverlayPos = readInt("Menu", "FpsOverlayPos");

            if (FpsOverlayPos.has_value())
            {
                if (Config::Instance()->FpsOverlayPos.value_or(0) < 0)
                    Config::Instance()->FpsOverlayPos = 0;

                if (Config::Instance()->FpsOverlayPos.value_or(0) > 3)
                    Config::Instance()->FpsOverlayPos = 3;
            }

            if (!FpsOverlayType.has_value())
                FpsOverlayType = readInt("Menu", "FpsOverlayType");

            if (FpsOverlayType.has_value())
            {
                if (Config::Instance()->FpsOverlayType.value_or(0) < 0)
                    Config::Instance()->FpsOverlayType = 0;

                if (Config::Instance()->FpsOverlayType.value_or(0) > 4)
                    Config::Instance()->FpsOverlayType = 4;
            }

            if (!FpsShortcutKey.has_value())
                FpsShortcutKey = readInt("Menu", "FpsShortcutKey");

            if (!FpsCycleShortcutKey.has_value())
                FpsCycleShortcutKey = readInt("Menu", "FpsCycleShortcutKey");

        }

        // Hooks
        {
            if (!HookOriginalNvngxOnly.has_value())
                HookOriginalNvngxOnly = readBool("Hooks", "HookOriginalNvngxOnly");
        }

        // RCAS
        {
            if (!RcasEnabled.has_value())
                RcasEnabled = readBool("CAS", "Enabled");

            if (!MotionSharpnessEnabled.has_value())
                MotionSharpnessEnabled = readBool("CAS", "MotionSharpnessEnabled");

            if (!MotionSharpness.has_value())
                MotionSharpness = readFloat("CAS", "MotionSharpness");

            if (!MotionSharpnessDebug.has_value())
                MotionSharpnessDebug = readBool("CAS", "MotionSharpnessDebug");

            if (!MotionThreshold.has_value())
                MotionThreshold = readFloat("CAS", "MotionThreshold");

            if (!MotionScaleLimit.has_value())
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
            if (!OutputScalingEnabled.has_value())
                OutputScalingEnabled = readBool("OutputScaling", "Enabled");

            if (!OutputScalingMultiplier.has_value())
                OutputScalingMultiplier = readFloat("OutputScaling", "Multiplier");

            if (!OutputScalingUseFsr.has_value())
                OutputScalingUseFsr = readBool("OutputScaling", "UseFsr");

            if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() < 0.5f)
                OutputScalingMultiplier = 0.5f;
            else if (OutputScalingMultiplier.has_value() && OutputScalingMultiplier.value() > 3.0f)
                OutputScalingMultiplier = 3.0f;

            if (!OutputScalingDownscaler.has_value())
                OutputScalingDownscaler = readInt("OutputScaling", "Downscaler");

        }

        // Init Flags
        {
            if (!AutoExposure.has_value())
                AutoExposure = readBool("InitFlags", "AutoExposure");

            if (!HDR.has_value())
                HDR = readBool("InitFlags", "HDR");

            if (!DepthInverted.has_value())
                DepthInverted = readBool("InitFlags", "DepthInverted");

            if (!JitterCancellation.has_value())
                JitterCancellation = readBool("InitFlags", "JitterCancellation");

            if (!DisplayResolution.has_value())
                DisplayResolution = readBool("InitFlags", "DisplayResolution");

            if (!DisableReactiveMask.has_value())
                DisableReactiveMask = readBool("InitFlags", "DisableReactiveMask");
        }


        // DRS
        {
            if (!DrsMinOverrideEnabled.has_value())
                DrsMinOverrideEnabled = readBool("DRS", "DrsMinOverrideEnabled");

            if (!DrsMaxOverrideEnabled.has_value())
                DrsMaxOverrideEnabled = readBool("DRS", "DrsMaxOverrideEnabled");
        }

        //Upscale Ratio Override
        {
            if (!UpscaleRatioOverrideEnabled.has_value())
                UpscaleRatioOverrideEnabled = readBool("UpscaleRatio", "UpscaleRatioOverrideEnabled");

            if (!UpscaleRatioOverrideValue.has_value())
                UpscaleRatioOverrideValue = readFloat("UpscaleRatio", "UpscaleRatioOverrideValue");
        }

        // Quality Overrides
        {
            if (!QualityRatioOverrideEnabled.has_value())
                QualityRatioOverrideEnabled = readBool("QualityOverrides", "QualityRatioOverrideEnabled");

            if (!QualityRatio_DLAA.has_value())
                QualityRatio_DLAA = readFloat("QualityOverrides", "QualityRatioDLAA");

            if (!QualityRatio_UltraQuality.has_value())
                QualityRatio_UltraQuality = readFloat("QualityOverrides", "QualityRatioUltraQuality");

            if (!QualityRatio_Quality.has_value())
                QualityRatio_Quality = readFloat("QualityOverrides", "QualityRatioQuality");

            if (!QualityRatio_Balanced.has_value())
                QualityRatio_Balanced = readFloat("QualityOverrides", "QualityRatioBalanced");

            if (!QualityRatio_Performance.has_value())
                QualityRatio_Performance = readFloat("QualityOverrides", "QualityRatioPerformance");

            if (!QualityRatio_UltraPerformance.has_value())
                QualityRatio_UltraPerformance = readFloat("QualityOverrides", "QualityRatioUltraPerformance");
        }

        // Hotfixes
        {
            if (!RoundInternalResolution.has_value())
                RoundInternalResolution = readInt("Hotfix", "RoundInternalResolution");

            if (!MipmapBiasOverride.has_value())
                MipmapBiasOverride = readFloat("Hotfix", "MipmapBiasOverride");

            if (MipmapBiasOverride.has_value() && (MipmapBiasOverride.value() > 15.0 || MipmapBiasOverride.value() < -15.0))
                MipmapBiasOverride.reset();

            if (!MipmapBiasFixedOverride.has_value())
                MipmapBiasFixedOverride = readBool("Hotfix", "MipmapBiasFixedOverride");

            if (!MipmapBiasScaleOverride.has_value())
                MipmapBiasScaleOverride = readBool("Hotfix", "MipmapBiasScaleOverride");

            if (!MipmapBiasOverrideAll.has_value())
                MipmapBiasOverrideAll = readBool("Hotfix", "MipmapBiasOverrideAll");

            if (!AnisotropyOverride.has_value())
                AnisotropyOverride = readInt("Hotfix", "AnisotropyOverride");

            if (AnisotropyOverride.has_value() && (AnisotropyOverride.value() > 16 || AnisotropyOverride.value() < 1))
                AnisotropyOverride.reset();

            if (!RestoreComputeSignature.has_value())
                RestoreComputeSignature = readBool("Hotfix", "RestoreComputeSignature");

            if (!RestoreGraphicSignature.has_value())
                RestoreGraphicSignature = readBool("Hotfix", "RestoreGraphicSignature");

            if (!PreferDedicatedGpu.has_value())
                PreferDedicatedGpu = readBool("Hotfix", "PreferDedicatedGpu");

            if (!PreferFirstDedicatedGpu.has_value())
                PreferFirstDedicatedGpu = readBool("Hotfix", "PreferFirstDedicatedGpu");

            if (!SkipFirstFrames.has_value())
                SkipFirstFrames = readInt("Hotfix", "SkipFirstFrames");

            if (!UsePrecompiledShaders.has_value())
                UsePrecompiledShaders = readBool("Hotfix", "UsePrecompiledShaders");

            if (!UseGenericAppIdWithDlss.has_value())
                UseGenericAppIdWithDlss = readBool("Hotfix", "UseGenericAppIdWithDlss");

            if (!ColorResourceBarrier.has_value())
                ColorResourceBarrier = readInt("Hotfix", "ColorResourceBarrier");

            if (!MVResourceBarrier.has_value())
                MVResourceBarrier = readInt("Hotfix", "MotionVectorResourceBarrier");

            if (!DepthResourceBarrier.has_value())
                DepthResourceBarrier = readInt("Hotfix", "DepthResourceBarrier");

            if (!MaskResourceBarrier.has_value())
                MaskResourceBarrier = readInt("Hotfix", "ColorMaskResourceBarrier");

            if (!ExposureResourceBarrier.has_value())
                ExposureResourceBarrier = readInt("Hotfix", "ExposureResourceBarrier");

            if (!OutputResourceBarrier.has_value())
                OutputResourceBarrier = readInt("Hotfix", "OutputResourceBarrier");
        }

        // Dx11 with Dx12
        {
            if (!TextureSyncMethod.has_value())
                TextureSyncMethod = readInt("Dx11withDx12", "TextureSyncMethod");

            if (!CopyBackSyncMethod.has_value())
                CopyBackSyncMethod = readInt("Dx11withDx12", "CopyBackSyncMethod");

            if (!Dx11DelayedInit.has_value())
                Dx11DelayedInit = readInt("Dx11withDx12", "UseDelayedInit");

            if (!SyncAfterDx12.has_value())
                SyncAfterDx12 = readInt("Dx11withDx12", "SyncAfterDx12");
        }

        // NvApi
        {
            if (!OverrideNvapiDll.has_value())
                OverrideNvapiDll = readBool("NvApi", "OverrideNvapiDll");

            if (!NvapiDllPath.has_value())
            {
                auto nvapiPathA = readString("NvApi", "NvapiDllPath", true);

                if (nvapiPathA.has_value())
                    NvapiDllPath = string_to_wstring(nvapiPathA.value());
            }
        }

        // Spoofing
        {
            if (!DxgiSpoofing.has_value())
                DxgiSpoofing = readBool("Spoofing", "Dxgi");

            if (!DxgiBlacklist.has_value())
                DxgiBlacklist = readString("Spoofing", "DxgiBlacklist");

            if (!DxgiVRAM.has_value())
                DxgiVRAM = readInt("Spoofing", "DxgiVRAM");

            if (!VulkanSpoofing.has_value())
                VulkanSpoofing = readBool("Spoofing", "Vulkan");

            if (!VulkanExtensionSpoofing.has_value())
                VulkanExtensionSpoofing = readBool("Spoofing", "VulkanExtensionSpoofing");

            if (!VulkanVRAM.has_value())
                VulkanVRAM = readInt("Spoofing", "VulkanVRAM");

            if (!SpoofedGPUName.has_value())
            {
                auto gpuName = readString("Spoofing", "SpoofedGPUName");
                if (gpuName.has_value())
                    SpoofedGPUName = string_to_wstring(gpuName.value());
            }
        }

        // Inputs
        {
            if (!DlssInputs.has_value())
                DlssInputs = readBool("Inputs", "Dlss");

            if (!XeSSInputs.has_value())
                XeSSInputs = readBool("Inputs", "XeSS");

            if (!Fsr2Inputs.has_value())
                Fsr2Inputs = readBool("Inputs", "Fsr2");

            if (!Fsr3Inputs.has_value())
                Fsr3Inputs = readBool("Inputs", "Fsr3");

            if (!FfxInputs.has_value())
                FfxInputs = readBool("Inputs", "Ffx");
        }

        // Plugins
        {

            if (!PluginPath.has_value())
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
            }

            if (!LoadSpecialK.has_value())
                LoadSpecialK = readBool("Plugins", "LoadSpecialK");

            if (!LoadReShade.has_value())
                LoadReShade = readBool("Plugins", "LoadReShade");
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
            if (!forceHdr.has_value())
                forceHdr = readBool("HDR", "ForceHDR");

            if (!useHDR10.has_value())
                useHDR10 = readBool("HDR", "UseHDR10");
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

    // Frame Generation 
    {
        ini.SetValue("FrameGen", "UseFGSwapChain", GetBoolValue(Instance()->FGUseFGSwapChain).c_str());
        ini.SetValue("FrameGen", "Enabled", GetBoolValue(Instance()->FGEnabled).c_str());
        ini.SetValue("FrameGen", "DebugView", GetBoolValue(Instance()->FGDebugView).c_str());
        ini.SetValue("FrameGen", "AllowAsync", GetBoolValue(Instance()->FGAsync).c_str());
        ini.SetValue("FrameGen", "HighPriority", GetBoolValue(Instance()->FGHighPriority).c_str());
        ini.SetValue("FrameGen", "HUDFix", GetBoolValue(Instance()->FGHUDFix).c_str());
        ini.SetValue("FrameGen", "HUDLimit", GetIntValue(Instance()->FGHUDLimit).c_str());
        ini.SetValue("FrameGen", "HUDFixExtended", GetBoolValue(Instance()->FGHUDFixExtended).c_str());
        ini.SetValue("FrameGen", "HUDFixImmadiate", GetBoolValue(Instance()->FGImmediateCapture).c_str());
        ini.SetValue("FrameGen", "RectLeft", GetIntValue(Instance()->FGRectLeft).c_str());
        ini.SetValue("FrameGen", "RectTop", GetIntValue(Instance()->FGRectTop).c_str());
        ini.SetValue("FrameGen", "RectWidth", GetIntValue(Instance()->FGRectWidth).c_str());
        ini.SetValue("FrameGen", "RectHeight", GetIntValue(Instance()->FGRectHeight).c_str());
        ini.SetValue("FrameGen", "DisableOverlays", GetBoolValue(Instance()->FGDisableOverlays).c_str());
        ini.SetValue("FrameGen", "AlwaysTrackHeaps", GetBoolValue(Instance()->FGAlwaysTrackHeaps).c_str());
        ini.SetValue("FrameGen", "HybridSpin", GetBoolValue(Instance()->FGHybridSpin).c_str());
    }

    // Framerate 
    {
        ini.SetValue("Framerate", "FramerateLimit", GetFloatValue(Instance()->FramerateLimit).c_str());
    }

    // Output Scaling
    {
        ini.SetValue("OutputScaling", "Enabled", GetBoolValue(Instance()->OutputScalingEnabled).c_str());
        ini.SetValue("OutputScaling", "Multiplier", GetFloatValue(Instance()->OutputScalingMultiplier).c_str());
        ini.SetValue("OutputScaling", "UseFsr", GetBoolValue(Instance()->OutputScalingUseFsr).c_str());
        ini.SetValue("OutputScaling", "Downscaler", GetBoolValue(Instance()->OutputScalingDownscaler).c_str());
    }

    // FSR common
    {
        ini.SetValue("FSR", "VerticalFov", GetFloatValue(Instance()->FsrVerticalFov).c_str());
        ini.SetValue("FSR", "HorizontalFov", GetFloatValue(Instance()->FsrHorizontalFov).c_str());
        ini.SetValue("FSR", "CameraNear", GetFloatValue(Instance()->FsrCameraNear).c_str());
        ini.SetValue("FSR", "CameraFar", GetFloatValue(Instance()->FsrCameraFar).c_str());
        ini.SetValue("FSR", "UseFsrInputValues", GetBoolValue(Instance()->FsrUseFsrInputValues).c_str());
    }

    // FSR 
    {
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
        ini.SetValue("Menu", "ShowFps", GetBoolValue(Instance()->ShowFps).c_str());
        ini.SetValue("Menu", "FpsOverlayPos", GetIntValue(Instance()->FpsOverlayPos).c_str());
        ini.SetValue("Menu", "FpsOverlayType", GetIntValue(Instance()->FpsOverlayType).c_str());
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
        ini.SetValue("Hotfix", "MipmapBiasOverrideAll", GetBoolValue(Instance()->MipmapBiasOverrideAll).c_str());
        ini.SetValue("Hotfix", "MipmapBiasFixedOverride", GetBoolValue(Instance()->MipmapBiasFixedOverride).c_str());
        ini.SetValue("Hotfix", "MipmapBiasScaleOverride", GetBoolValue(Instance()->MipmapBiasScaleOverride).c_str());

        ini.SetValue("Hotfix", "AnisotropyOverride", GetIntValue(Instance()->AnisotropyOverride).c_str());
        ini.SetValue("Hotfix", "RoundInternalResolution", GetIntValue(Instance()->RoundInternalResolution).c_str());

        ini.SetValue("Hotfix", "RestoreComputeSignature", GetBoolValue(Instance()->RestoreComputeSignature).c_str());
        ini.SetValue("Hotfix", "RestoreGraphicSignature", GetBoolValue(Instance()->RestoreGraphicSignature).c_str());
        ini.SetValue("Hotfix", "SkipFirstFrames", GetIntValue(Instance()->SkipFirstFrames).c_str());

        ini.SetValue("Hotfix", "UsePrecompiledShaders", GetBoolValue(Instance()->UsePrecompiledShaders).c_str());
        ini.SetValue("Hotfix", "PreferDedicatedGpu", GetBoolValue(Instance()->PreferDedicatedGpu).c_str());
        ini.SetValue("Hotfix", "PreferFirstDedicatedGpu", GetBoolValue(Instance()->PreferFirstDedicatedGpu).c_str());

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
        ini.SetValue("Spoofing", "VulkanVRAM", GetIntValue(Instance()->VulkanVRAM).c_str());
        ini.SetValue("Spoofing", "DxgiVRAM", GetIntValue(Instance()->DxgiVRAM).c_str());
        ini.SetValue("Spoofing", "SpoofedGPUName", Instance()->SpoofedGPUName.has_value() ? wstring_to_string(Instance()->SpoofedGPUName.value()).c_str() : "auto");
    }

    // Plugins
    {
        ini.SetValue("Plugins", "Path", Instance()->PluginPath.has_value() ? wstring_to_string(Instance()->PluginPath.value()).c_str() : "auto");
        ini.SetValue("Plugins", "LoadSpecialK", GetBoolValue(Instance()->LoadSpecialK).c_str());
    }

    // inputs
    {
        ini.SetValue("Inputs", "Dlss", GetBoolValue(Instance()->DlssInputs).c_str());
        ini.SetValue("Inputs", "XeSS", GetBoolValue(Instance()->XeSSInputs).c_str());
        ini.SetValue("Inputs", "Fsr2", GetBoolValue(Instance()->Fsr2Inputs).c_str());
        ini.SetValue("Inputs", "Fsr3", GetBoolValue(Instance()->Fsr3Inputs).c_str());
        ini.SetValue("Inputs", "Ffx", GetBoolValue(Instance()->FfxInputs).c_str());
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
    if (!nvngxExist)
        nvngxExist = std::filesystem::exists(Util::ExePath().parent_path() / L"nvngx.dll");

    if (!nvngxExist)
    {
        nvngxExist = GetModuleHandle(L"nvngx.dll") != nullptr;

        if (nvngxExist)
            LOG_INFO("nvngx.dll found in memory");
        else
            LOG_WARN("nvngx.dll not found!");

    }
    else
    {
        LOG_INFO("nvngx.dll found in game folder");
    }

    libxessExist = std::filesystem::exists(Util::ExePath().parent_path() / L"libxess.dll");
    if (!libxessExist)
    {
        libxessExist = GetModuleHandle(L"libxess.dll") != nullptr;

        if (libxessExist)
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