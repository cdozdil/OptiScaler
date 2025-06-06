#pragma once
#include "pch.h"
#include <ankerl/unordered_dense.h>

void PrepareLogger();
void CloseLogger();
void WaitForEnter();

#ifdef DLSS_PARAM_DUMP

inline static ankerl::unordered_dense::map<std::string, std::string> nvParamNames;

inline static void FillNvParamNames()
{
    if (nvParamNames.size() > 0)
        return;

    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved00", "#\x00" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_SuperSampling_Available", "#\x01" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_InPainting_Available", "#\x02" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSuperResolution_Available", "#\x03" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_SlowMotion_Available", "#\x04" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_VideoSuperResolution_Available", "#\x05" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved06", "#\x06" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved07", "#\x07" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved08", "#\x08" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSignalProcessing_Available", "#\x09" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSuperResolution_ScaleFactor_2_1", "#\x0a" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSuperResolution_ScaleFactor_3_1", "#\x0b" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSuperResolution_ScaleFactor_3_2", "#\x0c" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ImageSuperResolution_ScaleFactor_4_3", "#\x0d" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_NumFrames", "#\x0e" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Scale", "#\x0f" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Width", "#\x10" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Height", "#\x11" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_OutWidth", "#\x12" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_OutHeight", "#\x13" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Sharpness", "#\x14" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Scratch", "#\x15" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Scratch_SizeInBytes", "#\x16" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_EvaluationNode", "#\x17" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input1", "#\x18" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input1_Format", "#\x19" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input1_SizeInBytes", "#\x1a" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input2", "#\x1b" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input2_Format", "#\x1c" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Input2_SizeInBytes", "#\x1d" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Color", "#\x1e" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Color_Format", "#\x1f" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Color_SizeInBytes", "#\x20" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Albedo", "#\x21" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Output", "#\x22" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Output_Format", "#\x23" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Output_SizeInBytes", "#\x24" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reset", "#\x25" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_BlendFactor", "#\x26" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_MotionVectors", "#\x27" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Rect_X", "#\x28" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Rect_Y", "#\x29" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Rect_W", "#\x2a" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Rect_H", "#\x2b" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_MV_Scale_X", "#\x2c" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_MV_Scale_Y", "#\x2d" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Model", "#\x2e" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Format", "#\x2f" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_SizeInBytes", "#\x30" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ResourceAllocCallback", "#\x31" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_BufferAllocCallback", "#\x32" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Tex2DAllocCallback", "#\x33" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_ResourceReleaseCallback", "#\x34" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_CreationNodeMask", "#\x35" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_VisibilityNodeMask", "#\x36" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_PreviousOutput", "#\x37" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_MV_Offset_X", "#\x38" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_MV_Offset_Y", "#\x39" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Hint_UseFireflySwatter", "#\x3a" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Resource_Width", "#\x3b" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Resource_Height", "#\x3c" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Depth", "#\x3d" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_DLSSOptimalSettingsCallback", "#\x3e" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_PerfQualityValue", "#\x3f" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_RTXValue", "#\x40" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_DLSSMode", "#\x41" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_DeepResolve_Available", "#\x42" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Deprecated_43", "#\x43" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_OptLevel", "#\x44" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_IsDevSnippetBranch", "#\x45" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved_46", "#\x46" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Graphics_API", "#\x47" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved_48", "#\x48" });
    nvParamNames.insert({ "NVSDK_NGX_EParameter_Reserved_49", "#\x49" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OptLevel", "Snippet.OptLevel" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_IsDevSnippetBranch", "Snippet.IsDevBranch" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SuperSampling_ScaleFactor", "SuperSampling.ScaleFactor" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSignalProcessing_ScaleFactor", "ImageSignalProcessing.ScaleFactor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SuperSampling_Available", "SuperSampling.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_InPainting_Available", "InPainting.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSuperResolution_Available", "ImageSuperResolution.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SlowMotion_Available", "SlowMotion.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_VideoSuperResolution_Available", "VideoSuperResolution.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSignalProcessing_Available", "ImageSignalProcessing.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DeepResolve_Available", "DeepResolve.Available" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver", "SuperSampling.NeedsUpdatedDriver" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_InPainting_NeedsUpdatedDriver", "InPainting.NeedsUpdatedDriver" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_NeedsUpdatedDriver", "ImageSuperResolution.NeedsUpdatedDriver" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SlowMotion_NeedsUpdatedDriver", "SlowMotion.NeedsUpdatedDriver" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_VideoSuperResolution_NeedsUpdatedDriver", "VideoSuperResolution.NeedsUpdatedDriver" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSignalProcessing_NeedsUpdatedDriver", "ImageSignalProcessing.NeedsUpdatedDriver" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DeepResolve_NeedsUpdatedDriver", "DeepResolve.NeedsUpdatedDriver" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_FrameInterpolation_NeedsUpdatedDriver", "FrameInterpolation.NeedsUpdatedDriver" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor", "SuperSampling.MinDriverVersionMajor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_InPainting_MinDriverVersionMajor", "InPainting.MinDriverVersionMajor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSuperResolution_MinDriverVersionMajor",
                          "ImageSuperResolution.MinDriverVersionMajor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SlowMotion_MinDriverVersionMajor", "SlowMotion.MinDriverVersionMajor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_VideoSuperResolution_MinDriverVersionMajor",
                          "VideoSuperResolution.MinDriverVersionMajor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSignalProcessing_MinDriverVersionMajor",
                          "ImageSignalProcessing.MinDriverVersionMajor" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DeepResolve_MinDriverVersionMajor", "DeepResolve.MinDriverVersionMajor" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_FrameInterpolation_MinDriverVersionMajor", "FrameInterpolation.MinDriverVersionMajor" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor", "SuperSampling.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_InPainting_MinDriverVersionMinor", "InPainting.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSuperResolution_MinDriverVersionMinor",
                          "ImageSuperResolution.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SlowMotion_MinDriverVersionMinor", "SlowMotion.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_VideoSuperResolution_MinDriverVersionMinor",
                          "VideoSuperResolution.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ImageSignalProcessing_MinDriverVersionMinor",
                          "ImageSignalProcessing.MinDriverVersionMinor" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DeepResolve_MinDriverVersionMinor", "DeepResolve.MinDriverVersionMinor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult", "SuperSampling.FeatureInitResult" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_InPainting_FeatureInitResult", "InPainting.FeatureInitResult" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_FeatureInitResult", "ImageSuperResolution.FeatureInitResult" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SlowMotion_FeatureInitResult", "SlowMotion.FeatureInitResult" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_VideoSuperResolution_FeatureInitResult", "VideoSuperResolution.FeatureInitResult" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSignalProcessing_FeatureInitResult", "ImageSignalProcessing.FeatureInitResult" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DeepResolve_FeatureInitResult", "DeepResolve.FeatureInitResult" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_FrameInterpolation_FeatureInitResult", "FrameInterpolation.FeatureInitResult" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_2_1", "ImageSuperResolution.ScaleFactor.2.1" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_3_1", "ImageSuperResolution.ScaleFactor.3.1" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_3_2", "ImageSuperResolution.ScaleFactor.3.2" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_ImageSuperResolution_ScaleFactor_4_3", "ImageSuperResolution.ScaleFactor.4.3" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_NumFrames", "NumFrames" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Scale", "Scale" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Width", "Width" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Height", "Height" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutWidth", "OutWidth" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutHeight", "OutHeight" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Sharpness", "Sharpness" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Scratch", "Scratch" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Scratch_SizeInBytes", "Scratch.SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input1", "Input1" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input1_Format", "Input1.Format" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input1_SizeInBytes", "Input1.SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input2", "Input2" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input2_Format", "Input2.Format" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Input2_SizeInBytes", "Input2.SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Color", "Color" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Color_Format", "Color.Format" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Color_SizeInBytes", "Color.SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Color1", "Color1" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Color2", "Color2" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Albedo", "Albedo" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Output", "Output" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Output_Format", "Output.Format" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Output_SizeInBytes", "Output.SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Output1", "Output1" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Output2", "Output2" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Output3", "Output3" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Reset", "Reset" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_BlendFactor", "BlendFactor" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MotionVectors", "MotionVectors" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_MotionVectors1", "MotionVectors1" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_MotionVectors2", "MotionVectors2" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Rect_X", "Rect.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Rect_Y", "Rect.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Rect_W", "Rect.W" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Rect_H", "Rect.H" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutRect_X", "OutRect.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutRect_Y", "OutRect.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutRect_W", "OutRect.W" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_OutRect_H", "OutRect.H" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MV_Scale_X", "MV.Scale.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MV_Scale_Y", "MV.Scale.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Model", "Model" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Format", "Format" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_SizeInBytes", "SizeInBytes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ResourceAllocCallback", "ResourceAllocCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_BufferAllocCallback", "BufferAllocCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Tex2DAllocCallback", "Tex2DAllocCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ResourceReleaseCallback", "ResourceReleaseCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_CreationNodeMask", "CreationNodeMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_VisibilityNodeMask", "VisibilityNodeMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MV_Offset_X", "MV.Offset.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MV_Offset_Y", "MV.Offset.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Hint_UseFireflySwatter", "Hint.UseFireflySwatter" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Resource_Width", "ResourceWidth" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Resource_Height", "ResourceHeight" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Resource_OutWidth", "ResourceOutWidth" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Resource_OutHeight", "ResourceOutHeight" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Depth", "Depth" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Depth1", "Depth1" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Depth2", "Depth2" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback", "DLSSOptimalSettingsCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSSGetStatsCallback", "DLSSGetStatsCallback" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_PerfQualityValue", "PerfQualityValue" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_RTXValue", "RTXValue" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSSMode", "DLSSMode" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_Mode", "FIMode" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_OF_Preset", "FIOFPreset" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FI_OF_GridSize", "FIOFGridSize" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Jitter_Offset_X", "Jitter.Offset.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Jitter_Offset_Y", "Jitter.Offset.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Denoise", "Denoise" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_TransparencyMask", "TransparencyMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_ExposureTexture", "ExposureTexture" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Feature_Create_Flags", "DLSS.Feature.Create.Flags" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Checkerboard_Jitter_Hack", "DLSS.Checkerboard.Jitter.Hack" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Normals", "GBuffer.Normals" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Albedo", "GBuffer.Albedo" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Roughness", "GBuffer.Roughness" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_DiffuseAlbedo", "GBuffer.DiffuseAlbedo" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_SpecularAlbedo", "GBuffer.SpecularAlbedo" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_IndirectAlbedo", "GBuffer.IndirectAlbedo" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_SpecularMvec", "GBuffer.SpecularMvec" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_DisocclusionMask", "GBuffer.DisocclusionMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Metallic", "GBuffer.Metallic" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Specular", "GBuffer.Specular" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Subsurface", "GBuffer.Subsurface" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_ShadingModelId", "GBuffer.ShadingModelId" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_MaterialId", "GBuffer.MaterialId" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_8", "GBuffer.Attrib.8" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_9", "GBuffer.Attrib.9" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_10", "GBuffer.Attrib.10" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_11", "GBuffer.Attrib.11" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_12", "GBuffer.Attrib.12" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_13", "GBuffer.Attrib.13" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_14", "GBuffer.Attrib.14" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_GBuffer_Atrrib_15", "GBuffer.Attrib.15" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_TonemapperType", "TonemapperType" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FreeMemOnReleaseFeature", "FreeMemOnReleaseFeature" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MotionVectors3D", "MotionVectors3D" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_IsParticleMask", "IsParticleMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_AnimatedTextureMask", "AnimatedTextureMask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DepthHighRes", "DepthHighRes" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_Position_ViewSpace", "Position.ViewSpace" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_FrameTimeDeltaInMsec", "FrameTimeDeltaInMsec" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_RayTracingHitDistance", "RayTracingHitDistance" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_MotionVectorsReflection", "MotionVectorsReflection" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Enable_Output_Subrects", "DLSS.Enable.Output.Subrects" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_X", "DLSS.Input.Color.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Color_Subrect_Base_Y", "DLSS.Input.Color.Subrect.Base.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_X", "DLSS.Input.Depth.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Depth_Subrect_Base_Y", "DLSS.Input.Depth.Subrect.Base.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_X", "DLSS.Input.MV.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_MV_SubrectBase_Y", "DLSS.Input.MV.Subrect.Base.Y" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_X", "DLSS.Input.Translucency.Subrect.Base.X" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Input_Translucency_SubrectBase_Y", "DLSS.Input.Translucency.Subrect.Base.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_X", "DLSS.Output.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Output_Subrect_Base_Y", "DLSS.Output.Subrect.Base.Y" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Width", "DLSS.Render.Subrect.Dimensions.Width" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Render_Subrect_Dimensions_Height", "DLSS.Render.Subrect.Dimensions.Height" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Pre_Exposure", "DLSS.Pre.Exposure" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Exposure_Scale", "DLSS.Exposure.Scale" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_Mask", "DLSS.Input.Bias.Current.Color.Mask" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_X",
                          "DLSS.Input.Bias.Current.Color.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Input_Bias_Current_Color_SubrectBase_Y",
                          "DLSS.Input.Bias.Current.Color.Subrect.Base.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Indicator_Invert_Y_Axis", "DLSS.Indicator.Invert.Y.Axis" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Indicator_Invert_X_Axis", "DLSS.Indicator.Invert.X.Axis" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_INV_VIEW_PROJECTION_MATRIX", "InvViewProjectionMatrix" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_CLIP_TO_PREV_CLIP_MATRIX", "ClipToPrevClipMatrix" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_TransparencyLayer", "DLSS.TransparencyLayer" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_TransparencyLayer_Subrect_Base_X", "DLSS.TransparencyLayer.Subrect.Base.X" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_TransparencyLayer_Subrect_Base_Y", "DLSS.TransparencyLayer.Subrect.Base.Y" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_TransparencyLayerOpacity", "DLSS.TransparencyLayerOpacity" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_TransparencyLayerOpacity_Subrect_Base_X",
                          "DLSS.TransparencyLayerOpacity.Subrect.Base.X" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_TransparencyLayerOpacity_Subrect_Base_Y",
                          "DLSS.TransparencyLayerOpacity.Subrect.Base.Y" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Width", "DLSS.Get.Dynamic.Max.Render.Width" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Height", "DLSS.Get.Dynamic.Max.Render.Height" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width", "DLSS.Get.Dynamic.Min.Render.Width" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height", "DLSS.Get.Dynamic.Min.Render.Height" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA", "DLSS.Hint.Render.Preset.DLAA" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality", "DLSS.Hint.Render.Preset.Quality" });
    nvParamNames.insert({ "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced", "DLSS.Hint.Render.Preset.Balanced" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance", "DLSS.Hint.Render.Preset.Performance" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance", "DLSS.Hint.Render.Preset.UltraPerformance" });
    nvParamNames.insert(
        { "NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality", "DLSS.Hint.Render.Preset.UltraQuality" });
}

inline static void DumpNvParams(const NVSDK_NGX_Parameter* InParams)
{
    FillNvParamNames();

    spdlog::debug("DumpNvParams Dumping known NvParam values");

    for (const auto& [key, value] : nvParamNames)
    {
        unsigned long long ull = 0;
        float f = 0.0f;
        double d = 0.0;
        unsigned int ui = 0;
        int i = 0;
        void* v = nullptr;

        InParams->Get(value.c_str(), &ull);
        InParams->Get(value.c_str(), &f);
        InParams->Get(value.c_str(), &d);
        InParams->Get(value.c_str(), &ui);
        InParams->Get(value.c_str(), &i);
        InParams->Get(value.c_str(), &v);

        spdlog::debug("DumpNvParams {0} => ULL: {1}, F: {2}, D: {3}, UI: {4}, I: {5}, V*: {6:x}", key, ull, f, d, ui, i,
                      (UINT64) v);
    }
}

#endif
