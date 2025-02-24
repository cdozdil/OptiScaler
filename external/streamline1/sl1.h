/*
* Copyright (c) 2022 NVIDIA CORPORATION. All rights reserved
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include "sl1_consts.h"

//#define SL_API extern "C"
#define SL_API

namespace sl1 {

    using CommandBuffer = void;

    //! Buffer types used for tagging
    //! 
    //! IMPORTANT: Each tag must use the unique id
    //! 
    enum BufferType : uint32_t
    {
        //! Depth buffer - IMPORTANT - Must be suitable to use with clipToPrevClip transformation (see Constants below)
        eBufferTypeDepth,
        //! Object and optional camera motion vectors (see Constants below)
        eBufferTypeMVec,
        //! Color buffer with all post-processing effects applied but without any UI/HUD elements
        eBufferTypeHUDLessColor,
        //! Color buffer containing jittered input data for the image scaling pass
        eBufferTypeScalingInputColor,
        //! Color buffer containing results from the image scaling pass
        eBufferTypeScalingOutputColor,
        //! Normals
        eBufferTypeNormals,
        //! Roughness
        eBufferTypeRoughness,
        //! Albedo
        eBufferTypeAlbedo,
        //! Specular Albedo
        eBufferTypeSpecularAlbedo,
        //! Indirect Albedo
        eBufferTypeIndirectAlbedo,
        //! Specular Mvec
        eBufferTypeSpecularMVec,
        //! Disocclusion Mask
        eBufferTypeDisocclusionMask,
        //! Emissive
        eBufferTypeEmissive,
        //! Exposure
        eBufferTypeExposure,
        //! Buffer with normal and roughness in alpha channel
        eBufferTypeNormalRoughness,
        //! Diffuse and camera ray length
        eBufferTypeDiffuseHitNoisy,
        //! Diffuse denoised
        eBufferTypeDiffuseHitDenoised,
        //! Specular and reflected ray length
        eBufferTypeSpecularHitNoisy,
        //! Specular denoised
        eBufferTypeSpecularHitDenoised,
        //! Shadow noisy
        eBufferTypeShadowNoisy,
        //! Shadow denoised
        eBufferTypeShadowDenoised,
        //! AO noisy
        eBufferTypeAmbientOcclusionNoisy,
        //! AO denoised
        eBufferTypeAmbientOcclusionDenoised,

        //! Optional - UI/HUD pixels hint (set to 1 if a pixel belongs to the UI/HUD elements, 0 otherwise)
        eBufferTypeUIHint,
        //! Optional - Shadow pixels hint (set to 1 if a pixel belongs to the shadow area, 0 otherwise)
        eBufferTypeShadowHint,
        //! Optional - Reflection pixels hint (set to 1 if a pixel belongs to the reflection area, 0 otherwise)
        eBufferTypeReflectionHint,
        //! Optional - Particle pixels hint (set to 1 if a pixel represents a particle, 0 otherwise)
        eBufferTypeParticleHint,
        //! Optional - Transparency pixels hint (set to 1 if a pixel belongs to the transparent area, 0 otherwise)
        eBufferTypeTransparencyHint,
        //! Optional - Animated texture pixels hint (set to 1 if a pixel belongs to the animated texture area, 0 otherwise)
        eBufferTypeAnimatedTextureHint,
        //! Optional - Bias for current color vs history hint - lerp(history, current, bias) (set to 1 to completely reject history)
        eBufferTypeBiasCurrentColorHint,
        //! Optional - Ray-tracing distance (camera ray length)
        eBufferTypeRaytracingDistance,
        //! Optional - Motion vectors for reflections
        eBufferTypeReflectionMotionVectors,
        //! Optional - Position, in same space as eBufferTypeNormals
        eBufferTypePosition,
        //! Optional - Indicates (via non-zero value) which pixels have motion/depth values that do not match the final color content at that pixel (e.g. overlaid, opaque Picture-in-Picture)
        eBufferTypeInvalidDepthMotionHint
    };

    //! Features supported with this SDK
    //! 
    //! IMPORTANT: Each feature must use the unique id
    //! 
    enum Feature : uint32_t
    {
        //! Deep Learning Super Sampling
        eFeatureDLSS = 0,
        //! Real-Time Denoiser
        eFeatureNRD = 1,
        //! NVIDIA Image Scaling
        eFeatureNIS = 2,
        //! Low-Latency
        eFeatureReflex = 3,
        //! Common feature, NOT intended to be used directly
        eFeatureCommon = UINT_MAX
    };

    //! Different levels for logging
    enum LogLevel
    {
        //! No logging
        eLogLevelOff,
        //! Default logging
        eLogLevelDefault,
        //! Verbose logging
        eLogLevelVerbose,
        //! Total count
        eLogLevelCount
    };

    //! Resource types
    enum ResourceType : char
    {
        eResourceTypeTex2d,
        eResourceTypeBuffer
    };

    //! Resource description
    struct ResourceDesc
    {
        //! Indicates the type of resource
        ResourceType type = eResourceTypeTex2d;
        //! D3D12_RESOURCE_DESC/VkImageCreateInfo/VkBufferCreateInfo
        void* desc{};
        //! Initial state as D3D12_RESOURCE_STATES or VkMemoryPropertyFlags
        uint32_t state = 0;
        //! CD3DX12_HEAP_PROPERTIES or nullptr
        void* heap{};
        //! Reserved for future expansion, must be set to null
        void* ext{};
    };

    //! Native resource
    struct Resource
    {
        //! Indicates the type of resource
        ResourceType type = eResourceTypeTex2d;
        //! ID3D11Resource/ID3D12Resource/VkBuffer/VkImage
        void* native{};
        //! vkDeviceMemory or nullptr
        void* memory{};
        //! VkImageView/VkBufferView or nullptr
        void* view{};
        //! State as D3D12_RESOURCE_STATES or VkImageLayout
        //! 
        //! IMPORTANT: State needs to be correct when tagged resources are actually used.
        //! 
        uint32_t state{};
        //! Reserved for future expansion, must be set to null
        void* ext{};
    };

    //! Resource allocation/deallocation callbacks
    //!
    //! Use these callbacks to gain full control over 
    //! resource life cycle and memory allocation tracking.
    //!
    //! @param device - Device to be used (vkDevice or ID3D11Device or ID3D12Device)
    //!
    //! IMPORTANT: Textures must have the pixel shader resource
    //! and the unordered access view flags set
    using pfunResourceAllocateCallback = Resource(const ResourceDesc* desc, void* device);
    using pfunResourceReleaseCallback = void(Resource* resource, void* device);

    //! Log type
    enum LogType
    {
        //! Controlled by LogLevel, SL can show more information in eLogLevelVerbose mode
        eLogTypeInfo,
        //! Always shown regardless of LogLevel
        eLogTypeWarn,
        eLogTypeError,
        //! Total count
        eLogTypeCount
    };

    //! Logging callback
    //!
    //! Use these callbacks to track messages posted in the log.
    //! If any of the SL methods returns false use eLogTypeError
    //! type to track down what went wrong and why.
    using pfunLogMessageCallback = void(LogType type, const char* msg);

    //! Optional flags
    enum PreferenceFlags : uint64_t
    {
        //! IMPORTANT: If this flag is set then the host application is responsible for restoring CL state correctly after each 'slEvaluateFeature' call
        ePreferenceFlagDisableCLStateTracking = 1 << 0,
        //! Disables debug text on screen in development builds
        ePreferenceFlagDisableDebugText = 1 << 1,
    };

    SL_ENUM_OPERATORS_64(PreferenceFlags)

        //! Optional preferences
        struct Preferences
    {
        //! Optional - In non-production builds it is useful to enable debugging console window
        bool showConsole = false;
        //! Optional - Various logging levels
        LogLevel logLevel = eLogLevelDefault;
        //! Optional - Absolute paths to locations where to look for plugins, first path in the list has the highest priority
        const wchar_t** pathsToPlugins = {};
        //! Optional - Number of paths to search
        uint32_t numPathsToPlugins = 0;
        //! Optional - Absolute path to location where logs and other data should be stored
        //! NOTE: Set this to nullptr in order to disable logging to a file
        const wchar_t* pathToLogsAndData = {};
        //! Optional - Allows resource allocation tracking on the host side
        pfunResourceAllocateCallback* allocateCallback = {};
        //! Optional - Allows resource deallocation tracking on the host side
        pfunResourceReleaseCallback* releaseCallback = {};
        //! Optional - Allows log message tracking including critical errors if they occur
        pfunLogMessageCallback* logMessageCallback = {};
        //! Optional - Flags used to enable or disable advanced options
        PreferenceFlags flags{};
        //! Required - Features to load (assuming appropriate plugins are found), if not specified NO features will be loaded by default
        const Feature* featuresToLoad = {};
        //! Required - Number of features to load, only used when list is not a null pointer
        uint32_t numFeaturesToLoad = 0;
        //! Reserved for future expansion, must be set to null
        void* ext = {};
    };

    //! Unique application ID
    constexpr int kUniqueApplicationId = 0;

    //! Streamline API functions
    //! 
    using PFunSlInit = bool(const sl1::Preferences& pref, int applicationId);
    using PFunSlShutdown = bool();
    using PFunSlSetFeatureEnabled = bool(sl1::Feature feature, bool enabled);
    using PFunSlIsFeatureEnabled = bool(sl1::Feature feature);
    using PFunSlIsFeatureSupported = bool(sl1::Feature feature, uint32_t* adapterBitMask);
    using PFunSlSetTag = bool(const sl1::Resource* resource, sl1::BufferType tag, uint32_t id, const sl1::Extent* extent);
    using PFunSlSetConstants = bool(const sl1::Constants& values, uint32_t frameIndex, uint32_t id);
    using PFunSlSetFeatureConstants = bool(sl1::Feature feature, const void* consts, uint32_t frameIndex, uint32_t id);
    using PFunSlGetFeatureSettings = bool(sl1::Feature feature, const void* consts, void* settings);
    using PFunSlEvaluateFeature = bool(sl1::CommandBuffer* cmdBuffer, sl1::Feature feature, uint32_t frameIndex, uint32_t id);
    using PFunSlAllocateResources = bool(sl1::CommandBuffer* cmdBuffer, sl1::Feature feature, uint32_t id);
    using PFunSlFreeResources = bool(sl1::Feature feature, uint32_t id);

    //! Initializes the SL module
    //!
    //! Call this method when the game is initializing. 
    //!
    //! @param pref Specifies preferred behavior for the SL library (SL will keep a copy)
    //! @param applicationId Unique id for your application.
    //! @return false if SL is not supported on the system true otherwise.
    //!
    //! This method is NOT thread safe.
    SL_API bool slInit(const sl1::Preferences& pref, int applicationId = sl1::kUniqueApplicationId);

    //! Shuts down the SL module
    //!
    //! Call this method when the game is shutting down. 
    //!
    //! @return false if SL did not shutdown correctly true otherwise.
    //!
    //! This method is NOT thread safe.
    SL_API bool slShutdown();

    //! Checks if specific feature is supported or not.
    //!
    //! Call this method to check if certain eFeature* (see above) is available.
    //! This method must be called after init otherwise it will always return false.
    //!
    //! @param feature Specifies which feature to check
    //! @param adapterBitMask Optional bit-mask specifying which adapter supports the give feature
    //! @return false if feature is not supported on the system true otherwise.
    //!
    //! NOTE: You can provide the adapter bit mask to ensure that feature is available on the adapter
    //! for which you are planning to create a device. For the adapter at index N you can check the bit 1 << N.
    //!
    //! This method is NOT thread safe.
    SL_API bool slIsFeatureSupported(sl1::Feature feature, uint32_t* adapterBitMask = nullptr);

    //! Checks if specified feature is enabled or not.
    //!
    //! Call this method to check if feature is enabled.
    //! All supported features are enabled by default and have to be disabled explicitly if needed.
    //!
    //! @param feature Specifies which feature to check
    //! @return false if feature is disabled, not supported on the system or if device has not been created yet, true otherwise.
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slIsFeatureEnabled(sl1::Feature feature);

    //! Sets the specified feature to either enabled or disabled state.
    //!
    //! Call this method to enable or disable certain eFeature*. 
    //! All supported features are enabled by default and have to be disabled explicitly if needed.
    //!
    //! @param feature Specifies which feature to check
    //! @param enabled Value specifying if feature should be enabled or disabled.
    //! @return false if feature is not supported on the system or if device has not been created yet, true otherwise.
    //!
    //! NOTE: When this method is called no other DXGI/D3D/Vulkan APIs should be invoked in parallel so
    //! make sure to flush your pipeline before calling this method.
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slSetFeatureEnabled(sl1::Feature feature, bool enabled);

    //! Tags resource
    //!
    //! Call this method to tag the appropriate buffers.
    //!
    //! @param resource Pointer to resource to tag, set to null to remove the specified tag
    //! @param tag Specific tag for the resource
    //! @param id Unique id (can be viewport id | instance id etc.)
    //! @param extent The area of the tagged resource to use (if using the entire resource leave as null)
    //! @return false if resource cannot be tagged or if device has not been created yet, true otherwise.
    //!
    //! This method is thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slSetTag(const sl1::Resource* resource, sl1::BufferType tag, uint32_t id = 0, const sl1::Extent* extent = nullptr);

    //! Sets common constants.
    //!
    //! Call this method to provide the required data (SL will keep a copy).
    //!
    //! @param values Common constants required by SL plugins (SL will keep a copy)
    //! @param frameIndex Index of the current frame
    //! @param id Unique id (can be viewport id | instance id etc.)
    //! @return false if constants cannot be set or if device has not been created yet, true otherwise.
    //! 
    //! This method is thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slSetConstants(const sl1::Constants& values, uint32_t frameIndex, uint32_t id = 0);

    //! Sets feature specific constants.
    //!
    //! Call this method to provide the required data
    //! for the specified feature (SL will keep a copy).
    //!
    //! @param feature Feature we are working with
    //! @param consts Pointer to the feature specific constants (SL will keep a copy)
    //! @param frameIndex Index of the current frame
    //! @param id Unique id (can be viewport id | instance id etc.)
    //! @return false if constants cannot be set or if device has not been created yet, true otherwise.
    //!
    //! This method is thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slSetFeatureConstants(sl1::Feature feature, const void* consts, uint32_t frameIndex, uint32_t id = 0);

    //! Gets feature specific settings.
    //!
    //! Call this method to obtain settings for the specified feature.
    //!
    //! @param feature Feature we are working with
    //! @param consts Pointer to the feature specific constants
    //! @param settings Pointer to the returned feature specific settings
    //! @return false if feature does not have settings or if device has not been created yet, true otherwise.
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slGetFeatureSettings(sl1::Feature feature, const void* consts, void* settings);

    //! Allocates resources for the specified feature.
    //!
    //! Call this method to explicitly allocate resources
    //! for an instance of the specified feature.
    //! 
    //! @param cmdBuffer Command buffer to use (must be created on device where feature is supported but can be null if not needed)
    //! @param feature Feature we are working with
    //! @param id Unique id (instance handle)
    //! @return false if resources cannot be allocated or if device has not been created yet, true otherwise.
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slAllocateResources(sl1::CommandBuffer* cmdBuffer, sl1::Feature feature, uint32_t id);

    //! Frees resources for the specified feature.
    //!
    //! Call this method to explicitly free resources
    //! for an instance of the specified feature.
    //! 
    //! @param feature Feature we are working with
    //! @param id Unique id (instance handle)
    //! @return false if resources cannot be freed or if device has not been created yet, true otherwise.
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slFreeResources(sl1::Feature feature, uint32_t id);

    //! Evaluates feature
    //! 
    //! Use this method to mark the section in your rendering pipeline 
    //! where specific feature should be injected.
    //!
    //! @param cmdBuffer Command buffer to use (must be created on device where feature is supported but can be null if not needed)
    //! @param feature Feature we are working with
    //! @param frameIndex Current frame index (can be 0 if not needed)
    //! @param id Unique id (can be viewport id | instance id etc.)
    //! @return false if feature event cannot be injected in the command buffer or if device has not been created yet, true otherwise.
    //! 
    //! IMPORTANT: frameIndex and id must match whatever is used to set common and or feature constants (if any)
    //!
    //! This method is NOT thread safe and requires DX/VK device to be created before calling it.
    SL_API bool slEvaluateFeature(sl1::CommandBuffer* cmdBuffer, sl1::Feature feature, uint32_t frameIndex, uint32_t id = 0);
}