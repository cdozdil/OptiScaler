// This file is part of the FidelityFX SDK.
// 
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#pragma once

#include <stdint.h>

///
/// @defgroup ffxSDK SDK
/// The FidelityFX SDK
///

/// @defgroup ffxHost Host
/// The FidelityFX SDK host (CPU-side) References
///
/// @ingroup ffxSDK

/// @defgroup Defines Defines
/// Top level defines used by the FidelityFX SDK
///
/// @ingroup ffxHost

#if defined (FFX_GCC)
/// FidelityFX exported functions
///
/// @ingroup Defines
#define FFX_API
#else
/// FidelityFX exported functions
///
/// @ingroup Defines
#define FFX_API __declspec(dllexport)
#endif // #if defined (FFX_GCC)

#define FFX_SDK_DEFAULT_CONTEXT_SIZE (1024 * 128)

/// Maximum supported number of simultaneously bound SRVs.
///
/// @ingroup Defines
#define FFX_MAX_NUM_SRVS            16

/// Maximum supported number of simultaneously bound UAVs.
///
/// @ingroup Defines
#define FFX_MAX_NUM_UAVS            16

/// Maximum number of constant buffers bound.
///
/// @ingroup Defines
#define FFX_MAX_NUM_CONST_BUFFERS   3

/// Maximum size of bound constant buffers.
#define FFX_MAX_CONST_SIZE          256

/// Maximum number of characters in a resource name
///
/// @ingroup Defines
#define FFX_RESOURCE_NAME_SIZE      64

/// Maximum number of queued frames in the backend
///
/// @ingroup Defines
#define FFX_MAX_QUEUED_FRAMES          (4)

/// Maximum number of resources per effect context
///
/// @ingroup Defines
#define FFX_MAX_RESOURCE_COUNT         (128)

/// Maximum number of passes per effect component
///
/// @ingroup Defines
#define FFX_MAX_PASS_COUNT             (50)

/// Total ring buffer size needed for a single effect context
///
/// @ingroup Defines
#define FFX_RING_BUFFER_SIZE           (FFX_MAX_QUEUED_FRAMES * FFX_MAX_PASS_COUNT * FFX_MAX_RESOURCE_COUNT)

/// Size of constant buffer entry in the ring buffer table
///
/// @ingroup Defines
#define FFX_BUFFER_SIZE                (768)

/// Total ring buffer memory size for a single effect context
///
/// @ingroup Defines
#define FFX_RING_BUFFER_MEM_BLOCK_SIZE (FFX_RING_BUFFER_SIZE * FFX_BUFFER_SIZE)

/// Maximum number of barriers per flush
///
/// @ingroup Defines
#define FFX_MAX_BARRIERS               (16)

/// Maximum number of GPU jobs per submission
///
/// @ingroup Defines
#define FFX_MAX_GPU_JOBS               (64)

/// Maximum number of samplers supported
///
/// @ingroup Defines
#define FFX_MAX_SAMPLERS               (16)

/// Maximum number of simultaneous upload jobs
///
/// @ingroup Defines
#define UPLOAD_JOB_COUNT               (16)

// Off by default warnings
#pragma warning(disable : 4365 4710 4820 5039)

#ifdef __cplusplus
extern "C" {
#endif  // #ifdef __cplusplus

/// @defgroup CPUTypes CPU Types
/// CPU side type defines for all commonly used variables
///
/// @ingroup ffxHost

/// A typedef for a boolean value.
///
/// @ingroup CPUTypes
typedef bool FfxBoolean;

/// A typedef for a unsigned 8bit integer.
///
/// @ingroup CPUTypes
typedef uint8_t FfxUInt8;

/// A typedef for a unsigned 16bit integer.
///
/// @ingroup CPUTypes
typedef uint16_t FfxUInt16;

/// A typedef for a unsigned 32bit integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32;

/// A typedef for a unsigned 64bit integer.
///
/// @ingroup CPUTypes
typedef uint64_t FfxUInt64;

/// A typedef for a signed 8bit integer.
///
/// @ingroup CPUTypes
typedef int8_t FfxInt8;

/// A typedef for a signed 16bit integer.
///
/// @ingroup CPUTypes
typedef int16_t FfxInt16;

/// A typedef for a signed 32bit integer.
///
/// @ingroup CPUTypes
typedef int32_t FfxInt32;

/// A typedef for a signed 64bit integer.
///
/// @ingroup CPUTypes
typedef int64_t FfxInt64;

/// A typedef for a floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32;

/// A typedef for a 2-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x2[2];

/// A typedef for a 3-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x3[3];

/// A typedef for a 4-dimensional floating point value.
///
/// @ingroup CPUTypes
typedef float FfxFloat32x4[4];

/// A typedef for a 2-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x2[2];

/// A typedef for a 3-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x3[3];

/// A typedef for a 4-dimensional 32bit unsigned integer.
///
/// @ingroup CPUTypes
typedef uint32_t FfxUInt32x4[4];

/// @defgroup SDKTypes SDK Types
/// Structure and Enumeration definitions used by the FidelityFX SDK
///
/// @ingroup ffxHost


/// An enumeration of surface formats.
///
/// @ingroup SDKTypes
typedef enum FfxSurfaceFormat {

    FFX_SURFACE_FORMAT_UNKNOWN,                ///< Unknown format
    FFX_SURFACE_FORMAT_R32G32B32A32_TYPELESS,  ///< 32 bit per channel, 4 channel typeless format
    FFX_SURFACE_FORMAT_R32G32B32A32_UINT,      ///< 32 bit per channel, 4 channel uint format
    FFX_SURFACE_FORMAT_R32G32B32A32_FLOAT,     ///< 32 bit per channel, 4 channel float format
    FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT,     ///< 16 bit per channel, 4 channel float format
    FFX_SURFACE_FORMAT_R32G32_FLOAT,           ///< 32 bit per channel, 2 channel float format
    FFX_SURFACE_FORMAT_R8_UINT,                ///< 8 bit per channel, 1 channel float format
    FFX_SURFACE_FORMAT_R32_UINT,               ///< 32 bit per channel, 1 channel float format
    FFX_SURFACE_FORMAT_R10G10B10A2_UNORM,      ///< 10 bit per RGB channel with 2 bit alpha, 4 channel unsigned normalized format
    FFX_SURFACE_FORMAT_R8G8B8A8_TYPELESS,      ///<  8 bit per channel, 4 channel float format
    FFX_SURFACE_FORMAT_R8G8B8A8_UNORM,         ///<  8 bit per channel, 4 channel unsigned normalized format
    FFX_SURFACE_FORMAT_R8G8B8A8_SNORM,         ///<  8 bit per channel, 4 channel signed normalized format
    FFX_SURFACE_FORMAT_R8G8B8A8_SRGB,          ///<  8 bit per channel, 4 channel srgb normalized
    FFX_SURFACE_FORMAT_R11G11B10_FLOAT,        ///< 32 bit 3 channel float format
    FFX_SURFACE_FORMAT_R16G16_FLOAT,           ///< 16 bit per channel, 2 channel float format
    FFX_SURFACE_FORMAT_R16G16_UINT,            ///< 16 bit per channel, 2 channel unsigned int format
    FFX_SURFACE_FORMAT_R16G16_SINT,            ///< 16 bit per channel, 2 channel signed int format
    FFX_SURFACE_FORMAT_R16_FLOAT,              ///< 16 bit per channel, 1 channel float format
    FFX_SURFACE_FORMAT_R16_UINT,               ///< 16 bit per channel, 1 channel unsigned int format
    FFX_SURFACE_FORMAT_R16_UNORM,              ///< 16 bit per channel, 1 channel unsigned normalized format
    FFX_SURFACE_FORMAT_R16_SNORM,              ///< 16 bit per channel, 1 channel signed normalized format
    FFX_SURFACE_FORMAT_R8_UNORM,               ///<  8 bit per channel, 1 channel unsigned normalized format
    FFX_SURFACE_FORMAT_R8G8_UNORM,             ///<  8 bit per channel, 2 channel unsigned normalized format
    FFX_SURFACE_FORMAT_R8G8_UINT,              ///<  8 bit per channel, 2 channel unsigned integer format
    FFX_SURFACE_FORMAT_R32_FLOAT               ///< 32 bit per channel, 1 channel float format
} FfxSurfaceFormat;

/// An enumeration of resource usage.
///
/// @ingroup SDKTypes
typedef enum FfxResourceUsage {

    FFX_RESOURCE_USAGE_READ_ONLY = 0,               ///< No usage flags indicate a resource is read only.
    FFX_RESOURCE_USAGE_RENDERTARGET = (1<<0),       ///< Indicates a resource will be used as render target.
    FFX_RESOURCE_USAGE_UAV = (1<<1),                ///< Indicates a resource will be used as UAV.
    FFX_RESOURCE_USAGE_DEPTHTARGET = (1<<2),        ///< Indicates a resource will be used as depth target.
    FFX_RESOURCE_USAGE_INDIRECT = (1<<3),           ///< Indicates a resource will be used as indirect argument buffer
    FFX_RESOURCE_USAGE_ARRAYVIEW = (1<<4),          ///< Indicates a resource that will generate array views. Works on 2D and cubemap textures
} FfxResourceUsage;

/// An enumeration of resource states.
///
/// @ingroup SDKTypes
typedef enum FfxResourceStates {

    FFX_RESOURCE_STATE_COMMON               = (1 << 0),
    FFX_RESOURCE_STATE_UNORDERED_ACCESS     = (1 << 1), ///< Indicates a resource is in the state to be used as UAV.
    FFX_RESOURCE_STATE_COMPUTE_READ         = (1 << 2), ///< Indicates a resource is in the state to be read by compute shaders.
    FFX_RESOURCE_STATE_PIXEL_READ           = (1 << 3), ///< Indicates a resource is in the state to be read by pixel shaders.
    FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ   = (FFX_RESOURCE_STATE_PIXEL_READ | FFX_RESOURCE_STATE_COMPUTE_READ), ///< Indicates a resource is in the state to be read by pixel or compute shaders.
    FFX_RESOURCE_STATE_COPY_SRC             = (1 << 4), ///< Indicates a resource is in the state to be used as source in a copy command.
    FFX_RESOURCE_STATE_COPY_DEST            = (1 << 5), ///< Indicates a resource is in the state to be used as destination in a copy command.
    FFX_RESOURCE_STATE_GENERIC_READ         = (FFX_RESOURCE_STATE_COPY_SRC | FFX_RESOURCE_STATE_COMPUTE_READ),  ///< Indicates a resource is in generic (slow) read state.
    FFX_RESOURCE_STATE_INDIRECT_ARGUMENT    = (1 << 6), ///< Indicates a resource is in the state to be used as an indirect command argument
    FFX_RESOURCE_STATE_PRESENT              = (1 << 7), ///< Indicates a resource is in the state to be used to present to the swap chain
    FFX_RESOURCE_STATE_RENDER_TARGET        = (1 << 8), ///< Indicates a resource is in the state to be used as render target
} FfxResourceStates;

/// An enumeration of surface dimensions.
///
/// @ingroup SDKTypes
typedef enum FfxResourceDimension {

    FFX_RESOURCE_DIMENSION_TEXTURE_1D,              ///< A resource with a single dimension.
    FFX_RESOURCE_DIMENSION_TEXTURE_2D,              ///< A resource with two dimensions.
} FfxResourceDimension;

/// An enumeration of resource view dimensions.
///
/// @ingroup SDKTypes
typedef enum FfxResourceViewDimension
{
    FFX_RESOURCE_VIEW_DIMENSION_BUFFER,             ///< A resource view on a buffer.
    FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_1D,         ///< A resource view on a single dimension.
    FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_1D_ARRAY,   ///< A resource view on a single dimensional array.
    FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_2D,         ///< A resource view on two dimensions.
    FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_2D_ARRAY,  ///< A resource view on two dimensional array.
    FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_3D,         ///< A resource view on three dimensions.
} FfxResourceViewDimension;

/// An enumeration of surface dimensions.
///
/// @ingroup SDKTypes
typedef enum FfxResourceFlags {

    FFX_RESOURCE_FLAGS_NONE             = 0,            ///< No flags.
    FFX_RESOURCE_FLAGS_ALIASABLE        = (1 << 0),     ///< A bit indicating a resource does not need to persist across frames.
    FFX_RESOURCE_FLAGS_UNDEFINED        = (1 << 1),     ///< Special case flag used internally when importing resources that require additional setup
} FfxResourceFlags;

/// An enumeration of all resource view types.
///
/// @ingroup SDKTypes
typedef enum FfxResourceViewType {

    FFX_RESOURCE_VIEW_UNORDERED_ACCESS,             ///< The resource view is an unordered access view (UAV).
    FFX_RESOURCE_VIEW_SHADER_READ,                  ///< The resource view is a shader resource view (SRV).
} FfxResourceViewType;

/// The type of filtering to perform when reading a texture.
///
/// @ingroup SDKTypes
typedef enum FfxFilterType {

    FFX_FILTER_TYPE_MINMAGMIP_POINT,        ///< Point sampling.
    FFX_FILTER_TYPE_MINMAGMIP_LINEAR,       ///< Sampling with interpolation.
    FFX_FILTER_TYPE_MINMAGLINEARMIP_POINT,  ///< Use linear interpolation for minification and magnification; use point sampling for mip-level sampling.
} FfxFilterType;

/// The address mode used when reading a texture.
///
/// @ingroup SDKTypes
typedef enum FfxAddressMode {

    FFX_ADDRESS_MODE_WRAP,                  ///< Wrap when reading texture.
    FFX_ADDRESS_MODE_MIRROR,                ///< Mirror when reading texture.
    FFX_ADDRESS_MODE_CLAMP,                 ///< Clamp when reading texture.
    FFX_ADDRESS_MODE_BORDER,                ///< Border color when reading texture.
    FFX_ADDRESS_MODE_MIRROR_ONCE,           ///< Mirror once when reading texture.
} FfxAddressMode;

/// An enumeration of all supported shader models.
///
/// @ingroup SDKTypes
typedef enum FfxShaderModel {

    FFX_SHADER_MODEL_5_1,                           ///< Shader model 5.1.
    FFX_SHADER_MODEL_6_0,                           ///< Shader model 6.0.
    FFX_SHADER_MODEL_6_1,                           ///< Shader model 6.1.
    FFX_SHADER_MODEL_6_2,                           ///< Shader model 6.2.
    FFX_SHADER_MODEL_6_3,                           ///< Shader model 6.3.
    FFX_SHADER_MODEL_6_4,                           ///< Shader model 6.4.
    FFX_SHADER_MODEL_6_5,                           ///< Shader model 6.5.
    FFX_SHADER_MODEL_6_6,                           ///< Shader model 6.6.
    FFX_SHADER_MODEL_6_7,                           ///< Shader model 6.7.
} FfxShaderModel;

// An enumeration for different resource types
///
/// @ingroup SDKTypes
typedef enum FfxResourceType {

    FFX_RESOURCE_TYPE_BUFFER,                       ///< The resource is a buffer.
    FFX_RESOURCE_TYPE_TEXTURE1D,                    ///< The resource is a 1-dimensional texture.
    FFX_RESOURCE_TYPE_TEXTURE2D,                    ///< The resource is a 2-dimensional texture.
    FFX_RESOURCE_TYPE_TEXTURE_CUBE,                 ///< The resource is a cube map.
    FFX_RESOURCE_TYPE_TEXTURE3D,                    ///< The resource is a 3-dimensional texture.
} FfxResourceType;

/// An enumeration for different heap types
///
/// @ingroup SDKTypes
typedef enum FfxHeapType {

    FFX_HEAP_TYPE_DEFAULT = 0,                      ///< Local memory.
    FFX_HEAP_TYPE_UPLOAD                            ///< Heap used for uploading resources.
} FfxHeapType;

/// An enumeration for different render job types
///
/// @ingroup SDKTypes
typedef enum FfxGpuJobType {

    FFX_GPU_JOB_CLEAR_FLOAT = 0,                 ///< The GPU job is performing a floating-point clear.
    FFX_GPU_JOB_COPY = 1,                        ///< The GPU job is performing a copy.
    FFX_GPU_JOB_COMPUTE = 2,                     ///< The GPU job is performing a compute dispatch.
    FFX_GPU_JOB_BARRIER = 3,                     ///< The GPU job is performing a barrier.

} FfxGpuJobType;

/// An enumeration for various descriptor types
///
/// @ingroup SDKTypes
typedef enum FfxDescriptorType {

    //FFX_DESCRIPTOR_CBV = 0,   // All CBVs currently mapped to root signature
    //FFX_DESCRIPTOR_SAMPLER,   // All samplers currently static
    FFX_DESCRIPTOR_TEXTURE_SRV = 0,
    FFX_DESCRIPTOR_BUFFER_SRV,
    FFX_DESCRIPTOR_TEXTURE_UAV,
    FFX_DESCRIPTOR_BUFFER_UAV,
} FfxDescriptiorType;

/// An enumeration for view binding stages
///
/// @ingroup SDKTypes
typedef enum FfxBindStage {

    FFX_BIND_PIXEL_SHADER_STAGE = 1 << 0,
    FFX_BIND_VERTEX_SHADER_STAGE = 1 << 1,
    FFX_BIND_COMPUTE_SHADER_STAGE = 1 << 2,

} FfxBindStage;

/// An enumeration for barrier types
///
/// @ingroup SDKTypes
typedef enum FfxBarrierType
{
    FFX_BARRIER_TYPE_TRANSITION = 0,
    FFX_BARRIER_TYPE_UAV,
} FfxBarrierType;

/// An enumeration for message types that can be passed
///
/// @ingroup SDKTypes
typedef enum FfxMsgType {
    FFX_MESSAGE_TYPE_ERROR      = 0,
    FFX_MESSAGE_TYPE_WARNING    = 1,
    FFX_MESSAGE_TYPE_COUNT
} FfxMsgType;

/// An enumeration of all the effects which constitute the FidelityFX SDK.
///
/// Dictates what effect shader blobs to fetch for pipeline creation
///
/// @ingroup FfxInterface
typedef enum FfxEffect
{

    FFX_EFFECT_FSR2 = 0,               ///< FidelityFX Super Resolution v2
    FFX_EFFECT_FSR1,                   ///< FidelityFX Super Resolution
    FFX_EFFECT_SPD,                    ///< FidelityFX Single Pass Downsampler
    FFX_EFFECT_BLUR,                   ///< FidelityFX Blur
    FFX_EFFECT_CACAO,                  ///< FidelityFX Combined Adaptive Compute Ambient Occlusion
    FFX_EFFECT_CAS,                    ///< FidelityFX Contrast Adaptive Sharpening
    FFX_EFFECT_DENOISER,               ///< FidelityFX Denoiser
    FFX_EFFECT_LENS,                   ///< FidelityFX Lens
    FFX_EFFECT_PARALLEL_SORT,          ///< FidelityFX Parallel Sort
    FFX_EFFECT_SSSR,                   ///< FidelityFX Stochastic Screen Space Reflections
    FFX_EFFECT_VARIABLE_SHADING,       ///< FidelityFX Variable Shading
    FFX_EFFECT_LPM,                    ///< FidelityFX Luma Preserving Mapper
    FFX_EFFECT_DOF,                    ///< FidelityFX Depth of Field
    FFX_EFFECT_CLASSIFIER,             ///< FidelityFX Classifier
    FFX_EFFECT_FSR3UPSCALER,           ///< FidelityFX Super Resolution v3
    FFX_EFFECT_FRAMEINTERPOLATION,     ///< FidelityFX Frame Interpolation, part of FidelityFX Super Resolution v3
    FFX_EFFECT_OPTICALFLOW,            ///< FidelityFX Optical Flow, part of FidelityFX Super Resolution v3

} FfxEffect;

typedef enum FfxBackbufferTransferFunction {
    FFX_BACKBUFFER_TRANSFER_FUNCTION_SRGB,
    FFX_BACKBUFFER_TRANSFER_FUNCTION_PQ,
    FFX_BACKBUFFER_TRANSFER_FUNCTION_SCRGB
} FfxBackbufferTransferFunction;

/// A typedef representing the graphics device.
///
/// @ingroup SDKTypes
typedef void* FfxDevice;

typedef void* FfxCommandQueue;

typedef void* FfxSwapchain;

/// A typedef representing a command list or command buffer.
///
/// @ingroup SDKTypes
typedef void* FfxCommandList;

/// A typedef for a root signature.
///
/// @ingroup SDKTypes
typedef void* FfxRootSignature;

/// A typedef for a command signature, used for indirect workloads
///
/// @ingroup SDKTypes
typedef void* FfxCommandSignature;

/// A typedef for a pipeline state object.
///
/// @ingroup SDKTypes
typedef void* FfxPipeline;

/// A structure encapsulating a collection of device capabilities.
///
/// @ingroup SDKTypes
typedef struct FfxDeviceCapabilities {

    FfxShaderModel                  minimumSupportedShaderModel;            ///< The minimum shader model supported by the device.
    uint32_t                        waveLaneCountMin;                       ///< The minimum supported wavefront width.
    uint32_t                        waveLaneCountMax;                       ///< The maximum supported wavefront width.
    bool                            fp16Supported;                          ///< The device supports FP16 in hardware.
    bool                            raytracingSupported;                    ///< The device supports ray tracing.
} FfxDeviceCapabilities;

/// A structure encapsulating a 2-dimensional point, using 32bit unsigned integers.
///
/// @ingroup SDKTypes
typedef struct FfxDimensions2D {

    uint32_t                        width;                                  ///< The width of a 2-dimensional range.
    uint32_t                        height;                                 ///< The height of a 2-dimensional range.
} FfxDimensions2D;

/// A structure encapsulating a 2-dimensional point.
///
/// @ingroup SDKTypes
typedef struct FfxIntCoords2D {

    int32_t                         x;                                      ///< The x coordinate of a 2-dimensional point.
    int32_t                         y;                                      ///< The y coordinate of a 2-dimensional point.
} FfxIntCoords2D;

/// A structure encapsulating a 2-dimensional rect.
///
/// @ingroup SDKTypes
typedef struct FfxRect2D
{
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
} FfxRect2D;

/// A structure encapsulating a 2-dimensional set of floating point coordinates.
///
/// @ingroup SDKTypes
typedef struct FfxFloatCoords2D {

    float                           x;                                      ///< The x coordinate of a 2-dimensional point.
    float                           y;                                      ///< The y coordinate of a 2-dimensional point.
} FfxFloatCoords2D;

/// A structure describing a resource.
///
/// @ingroup SDKTypes
typedef struct FfxResourceDescription {

    FfxResourceType                 type;                                   ///< The type of the resource.
    FfxSurfaceFormat                format;                                 ///< The surface format.
    union {
        uint32_t                    width;                                  ///< The width of the texture resource.
        uint32_t                    size;                                   ///< The size of the buffer resource.
    };

    union {
        uint32_t                    height;                                 ///< The height of the texture resource.
        uint32_t                    stride;                                 ///< The stride of the buffer resource.
    };

    union {
        uint32_t                    depth;                                  ///< The depth of the texture resource.
        uint32_t                    alignment;                              ///< The alignment of the buffer resource.
    };

    uint32_t                        mipCount;                               ///< Number of mips (or 0 for full mipchain).
    FfxResourceFlags                flags;                                  ///< A set of <c><i>FfxResourceFlags</i></c> flags.
    FfxResourceUsage                usage;                                  ///< Resource usage flags.
} FfxResourceDescription;

/// An outward facing structure containing a resource
///
/// @ingroup SDKTypes
typedef struct FfxResource {
    void*                           resource;                               ///< pointer to the resource.
    FfxResourceDescription          description;
    FfxResourceStates               state;
    wchar_t                         name[FFX_RESOURCE_NAME_SIZE];           ///< (optional) Resource name.
} FfxResource;

/// An internal structure containing a handle to a resource and resource views
///
/// @ingroup SDKTypes
typedef struct FfxResourceInternal {
    int32_t                         internalIndex;                          ///< The index of the resource.
} FfxResourceInternal;

/// An internal structure housing all that is needed for backend resource descriptions
///
/// @ingroup SDKTypes
typedef struct FfxInternalResourceDescription {

    uint32_t                    id;
    const wchar_t* name;
    FfxResourceType             type;
    FfxResourceUsage            usage;
    FfxSurfaceFormat            format;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    mipCount;
    FfxResourceFlags            flags;
    uint32_t                    initDataSize;
    void* initData;
} FfxInternalResourceDescription;

/// A structure defining the view to create
///
/// @ingroup SDKTypes
typedef struct FfxViewDescription
{
    bool                        uavView;                    ///< Indicates that the view is a UAV.
    FfxResourceViewDimension    viewDimension;              ///< The view dimension to map
    union {
        int32_t mipLevel;                                   ///< The mip level of the view, (-1) for default
        int32_t firstElement;                               ///< The first element of a buffer view, (-1) for default
    };

    union {
        int32_t arraySize;                                  ///< The array size of the view, (-1) for full depth/array size
        int32_t elementCount;                               ///< The number of elements in a buffer view, (-1) for full depth/array size
    };

    int32_t                     firstSlice;                 ///< The first slice to map to, (-1) for default first slice
    wchar_t name[64];
} FfxViewDescription;

static FfxViewDescription s_FfxViewDescInit = { false, FFX_RESOURCE_VIEW_DIMENSION_TEXTURE_2D, -1, -1, -1, L"" };

/// A structure defining a resource bind point
///
/// @ingroup SDKTypes
typedef struct FfxResourceBinding
{
    uint32_t    slotIndex;
    uint32_t    resourceIdentifier;
    uint32_t    bindCount;
    wchar_t     name[64];
}FfxResourceBinding;

/// A structure encapsulating a single pass of an algorithm.
///
/// @ingroup SDKTypes
typedef struct FfxPipelineState {

    FfxRootSignature                rootSignature;                                      ///< The pipelines rootSignature
    FfxCommandSignature             cmdSignature;                                       ///< The command signature used for indirect workloads
    FfxPipeline                     pipeline;                                           ///< The pipeline object
    uint32_t                        uavTextureCount;                                    ///< Count of Texture UAVs used in this pipeline
    uint32_t                        srvTextureCount;                                    ///< Count of Texture SRVs used in this pipeline
    uint32_t                        srvBufferCount;                                     ///< Count of Buffer SRV used in this pipeline
    uint32_t                        uavBufferCount;                                     ///< Count of Buffer UAVs used in this pipeline
    uint32_t                        constCount;                                         ///< Count of constant buffers used in this pipeline

    FfxResourceBinding              uavTextureBindings[FFX_MAX_NUM_UAVS];               ///< Array of ResourceIdentifiers bound as texture UAVs
    FfxResourceBinding              srvTextureBindings[FFX_MAX_NUM_SRVS];               ///< Array of ResourceIdentifiers bound as texture SRVs
    FfxResourceBinding              srvBufferBindings[FFX_MAX_NUM_SRVS];                ///< Array of ResourceIdentifiers bound as buffer SRVs
    FfxResourceBinding              uavBufferBindings[FFX_MAX_NUM_UAVS];                ///< Array of ResourceIdentifiers bound as buffer UAVs
    FfxResourceBinding              constantBufferBindings[FFX_MAX_NUM_CONST_BUFFERS];  ///< Array of ResourceIdentifiers bound as CBs
} FfxPipelineState;

/// A structure containing the data required to create a resource.
///
/// @ingroup SDKTypes
typedef struct FfxCreateResourceDescription {

    FfxHeapType                     heapType;                               ///< The heap type to hold the resource, typically <c><i>FFX_HEAP_TYPE_DEFAULT</i></c>.
    FfxResourceDescription          resourceDescription;                    ///< A resource description.
    FfxResourceStates               initalState;                            ///< The initial resource state.
    uint32_t                        initDataSize;                           ///< Size of initial data buffer.
    void*                           initData;                               ///< Buffer containing data to fill the resource.
    const wchar_t*                  name;                                   ///< Name of the resource.
    uint32_t                        id;                                     ///< Internal resource ID.
} FfxCreateResourceDescription;

/// A structure containing the data required to create sampler mappings
///
/// @ingroup SDKTypes
typedef struct FfxSamplerDescription {

    FfxFilterType   filter;
    FfxAddressMode  addressModeU;
    FfxAddressMode  addressModeV;
    FfxAddressMode  addressModeW;
    FfxBindStage    stage;
} FfxSamplerDescription;

/// A structure containing the data required to create root constant buffer mappings
///
/// @ingroup SDKTypes
typedef struct FfxRootConstantDescription
{
    uint32_t      size;
    FfxBindStage  stage;
} FfxRootConstantDescription;

/// A structure containing the description used to create a
/// <c><i>FfxPipeline</i></c> structure.
///
/// A pipeline is the name given to a shader and the collection of state that
/// is required to dispatch it. In the context of the FidelityFX SDK and its architecture
/// this means that a <c><i>FfxPipelineDescription</i></c> will map to either a
/// monolithic object in an explicit API (such as a
/// <c><i>PipelineStateObject</i></c> in DirectX 12). Or a shader and some
/// ancillary API objects (in something like DirectX 11).
///
/// The <c><i>contextFlags</i></c> field contains a copy of the flags passed
/// to <c><i>ffxContextCreate</i></c> via the <c><i>flags</i></c> field of
/// the <c><i>Ffx<Effect>InitializationParams</i></c> structure. These flags are
/// used to determine which permutation of a pipeline for a specific
/// <c><i>Ffx<Effect>Pass</i></c> should be used to implement the features required
/// by each application, as well as to achieve the best performance on specific
/// target hardware configurations.
///
/// When using one of the provided backends for FidelityFX SDK (such as DirectX 12 or
/// Vulkan) the data required to create a pipeline is compiled off line and
/// included into the backend library that you are using. For cases where the
/// backend interface is overridden by providing custom callback function
/// implementations care should be taken to respect the contents of the
/// <c><i>contextFlags</i></c> field in order to correctly support the options
/// provided by the FidelityFX SDK, and achieve best performance.
/// ///
/// @ingroup SDKTypes
typedef struct FfxPipelineDescription {

    uint32_t                            contextFlags;                   ///< A collection of <c><i>FfxInitializationFlagBits</i></c> which were passed to the context.
    const FfxSamplerDescription*        samplers;                       ///< A collection of samplers to use when building the root signature for the pipeline
    size_t                              samplerCount;                   ///< Number of samplers to create for the pipeline
    const FfxRootConstantDescription*   rootConstants;                  ///< A collection of root constant descriptions to use when building the root signature for the pipeline
    uint32_t                            rootConstantBufferCount;        ///< Number of root constant buffers to create for the pipeline
    wchar_t                             name[64];                       ///< Pipeline name with which to name the pipeline object
    FfxBindStage                        stage;                          ///< The stage(s) for which this pipeline is being built
    uint32_t                            indirectWorkload;               ///< Whether this pipeline has an indirect workload
    FfxSurfaceFormat                    backbufferFormat;               ///< For raster pipelines this contains the backbuffer format
} FfxPipelineDescription;

/// A structure containing the data required to create a barrier
///
/// @ingroup SDKTypes
typedef struct FfxBarrierDescription
{
    FfxResourceInternal resource;
    FfxBarrierType    barrierType;
    FfxResourceStates currentState;
    FfxResourceStates newState;
    uint32_t          subResourceID;
} FfxBarrierDescription;


/// A structure containing a constant buffer.
///
/// @ingroup SDKTypes
typedef struct FfxConstantBuffer {

    uint32_t                        num32BitEntries;                        ///< The size (expressed in 32-bit chunks) stored in data.
    uint32_t                        data[FFX_MAX_CONST_SIZE];               ///< Constant buffer data
}FfxConstantBuffer;

/// A structure describing a clear render job.
///
/// @ingroup SDKTypes
typedef struct FfxClearFloatJobDescription {

    float                           color[4];                               ///< The clear color of the resource.
    FfxResourceInternal             target;                                 ///< The resource to be cleared.
} FfxClearFloatJobDescription;

/// A structure describing a compute render job.
///
/// @ingroup SDKTypes
typedef struct FfxComputeJobDescription {

    FfxPipelineState                pipeline;                               ///< Compute pipeline for the render job.
    uint32_t                        dimensions[3];                          ///< Dispatch dimensions.
    FfxResourceInternal             cmdArgument;                            ///< Dispatch indirect cmd argument buffer
    uint32_t                        cmdArgumentOffset;                      ///< Dispatch indirect offset within the cmd argument buffer
    FfxResourceInternal             srvTextures[FFX_MAX_NUM_SRVS];          ///< SRV texture resources to be bound in the compute job.
    wchar_t                         srvTextureNames[FFX_MAX_NUM_SRVS][64];
    FfxResourceInternal             uavTextures[FFX_MAX_NUM_UAVS];          ///< UAV texture resources to be bound in the compute job.
    uint32_t                        uavTextureMips[FFX_MAX_NUM_UAVS];       ///< Mip level of UAV texture resources to be bound in the compute job.
    wchar_t                         uavTextureNames[FFX_MAX_NUM_UAVS][64];
    FfxResourceInternal             srvBuffers[FFX_MAX_NUM_SRVS];           ///< SRV buffer resources to be bound in the compute job.
    wchar_t                         srvBufferNames[FFX_MAX_NUM_SRVS][64];
    FfxResourceInternal             uavBuffers[FFX_MAX_NUM_UAVS];           ///< UAV buffer resources to be bound in the compute job.
    wchar_t                         uavBufferNames[FFX_MAX_NUM_UAVS][64];
    FfxConstantBuffer               cbs[FFX_MAX_NUM_CONST_BUFFERS];         ///< Constant buffers to be bound in the compute job.
    wchar_t                         cbNames[FFX_MAX_NUM_CONST_BUFFERS][64];
} FfxComputeJobDescription;

typedef struct FfxRasterJobDescription
{
    FfxPipelineState                pipeline;                               ///< Raster pipeline for the render job.
    uint32_t                        numVertices;
    FfxResourceInternal             renderTarget;

    FfxResourceInternal             srvTextures[FFX_MAX_NUM_SRVS];                 ///< SRV resources to be bound in the compute job.
    wchar_t                         srvTextureNames[FFX_MAX_NUM_SRVS][64];
    FfxResourceInternal             uavTextures[FFX_MAX_NUM_UAVS];                 ///< UAV resources to be bound in the compute job.
    uint32_t                        uavTextureMips[FFX_MAX_NUM_UAVS];               ///< Mip level of UAV resources to be bound in the compute job.
    wchar_t                         uavTextureNames[FFX_MAX_NUM_UAVS][64];
    FfxConstantBuffer               cbs[FFX_MAX_NUM_CONST_BUFFERS];         ///< Constant buffers to be bound in the compute job.
    wchar_t                         cbNames[FFX_MAX_NUM_CONST_BUFFERS][64];
} FfxRasterJobDescription;

/// A structure describing a copy render job.
///
/// @ingroup SDKTypes
typedef struct FfxCopyJobDescription
{
    FfxResourceInternal                     src;                                    ///< Source resource for the copy.
    FfxResourceInternal                     dst;                                    ///< Destination resource for the copy.
} FfxCopyJobDescription;

/// A structure describing a single render job.
///
/// @ingroup SDKTypes
typedef struct FfxGpuJobDescription{

    FfxGpuJobType                jobType;                                    ///< Type of the job.

    union {
        FfxClearFloatJobDescription clearJobDescriptor;                     ///< Clear job descriptor. Valid when <c><i>jobType</i></c> is <c><i>FFX_RENDER_JOB_CLEAR_FLOAT</i></c>.
        FfxCopyJobDescription       copyJobDescriptor;                      ///< Copy job descriptor. Valid when <c><i>jobType</i></c> is <c><i>FFX_RENDER_JOB_COPY</i></c>.
        FfxComputeJobDescription    computeJobDescriptor;                   ///< Compute job descriptor. Valid when <c><i>jobType</i></c> is <c><i>FFX_RENDER_JOB_COMPUTE</i></c>.
        FfxRasterJobDescription     rasterJobDescriptor;
        FfxBarrierDescription       barrierDescriptor;
    };
} FfxGpuJobDescription;

#if defined(POPULATE_SHADER_BLOB_FFX)
#undef POPULATE_SHADER_BLOB_FFX
#endif // #if defined(POPULATE_SHADER_BLOB_FFX)

/// Macro definition to copy header shader blob information into its SDK structural representation
///
/// @ingroup SDKTypes
#define POPULATE_SHADER_BLOB_FFX(info, index)                                                                                                              \
    {                                                                                                                                                      \
        info[index].blobData, info[index].blobSize, info[index].numConstantBuffers, info[index].numSRVTextures, info[index].numUAVTextures,                \
            info[index].numSRVBuffers, info[index].numUAVBuffers, info[index].numSamplers, info[index].numRTAccelerationStructures,                           \
            info[index].constantBufferNames,                  \
            info[index].constantBufferBindings, info[index].constantBufferCounts, info[index].srvTextureNames, info[index].srvTextureBindings,             \
            info[index].srvTextureCounts, info[index].uavTextureNames, info[index].uavTextureBindings, info[index].uavTextureCounts,                       \
            info[index].srvBufferNames, info[index].srvBufferBindings, info[index].srvBufferCounts, \
            info[index].uavBufferNames, info[index].uavBufferBindings, info[index].uavBufferCounts, info[index].samplerNames,  \
            info[index].samplerBindings, info[index].samplerCounts, info[index].rtAccelerationStructureNames, info[index].rtAccelerationStructureBindings, \
            info[index].rtAccelerationStructureCounts                                                                                                      \
    }

// A single shader blob and a description of its resources.
///
/// @ingroup SDKTypes
typedef struct FfxShaderBlob {

    const uint8_t* data;                                // A pointer to the blob
    const uint32_t  size;                               // Size in bytes.

    const uint32_t  cbvCount;                           // Number of CBs.
    const uint32_t  srvTextureCount;                    // Number of SRV Textures.
    const uint32_t  uavTextureCount;                    // Number of UAV Textures.
    const uint32_t  srvBufferCount;                     // Number of SRV Buffers.
    const uint32_t  uavBufferCount;                     // Number of UAV Buffers.
    const uint32_t  samplerCount;                       // Number of Samplers.
    const uint32_t  rtAccelStructCount;                 // Number of RT Acceleration structures.

    // constant buffers
    const char** boundConstantBufferNames;
    const uint32_t* boundConstantBuffers;               // Pointer to an array of bound ConstantBuffers.
    const uint32_t* boundConstantBufferCounts;          // Pointer to an array of bound ConstantBuffer resource counts

    // srv textures
    const char** boundSRVTextureNames;
    const uint32_t* boundSRVTextures;                   // Pointer to an array of bound SRV resources.
    const uint32_t* boundSRVTextureCounts;              // Pointer to an array of bound SRV resource counts

    // uav textures
    const char** boundUAVTextureNames;
    const uint32_t* boundUAVTextures;                   // Pointer to an array of bound UAV texture resources.
    const uint32_t* boundUAVTextureCounts;              // Pointer to an array of bound UAV texture resource counts

    // srv buffers
    const char** boundSRVBufferNames;
    const uint32_t* boundSRVBuffers;                    // Pointer to an array of bound SRV buffer resources.
    const uint32_t* boundSRVBufferCounts;               // Pointer to an array of bound SRV buffer resource counts

    // uav buffers
    const char** boundUAVBufferNames;
    const uint32_t* boundUAVBuffers;                    // Pointer to an array of bound UAV buffer resources.
    const uint32_t* boundUAVBufferCounts;               // Pointer to an array of bound UAV buffer resource counts

    // samplers
    const char** boundSamplerNames;
    const uint32_t* boundSamplers;                      // Pointer to an array of bound sampler resources.
    const uint32_t* boundSamplerCounts;                 // Pointer to an array of bound sampler resource counts

    // rt acceleration structures
    const char** boundRTAccelerationStructureNames;
    const uint32_t* boundRTAccelerationStructures;      // Pointer to an array of bound UAV buffer resources.
    const uint32_t* boundRTAccelerationStructureCounts; // Pointer to an array of bound UAV buffer resource counts

} FfxShaderBlob;

typedef struct FfxPresentCallbackDescription
{
    FfxDevice        device;
    FfxCommandList   commandList;
    FfxResource      currentBackBuffer;
    FfxResource      currentUI;
    FfxResource      outputSwapChainBuffer;
    bool             isInterpolatedFrame;
} FfxPresentCallbackDescription;

typedef struct FfxFrameGenerationDispatchDescription {
    FfxCommandList              commandList;
    FfxResource                 presentColor;
    FfxResource                 outputs[4];
    uint32_t                    numInterpolatedFrames;
    bool                        reset;
    FfxBackbufferTransferFunction backBufferTransferFunction;
    float                       minMaxLuminance[2];
} FfxFrameGenerationDispatchDescription;

#ifdef __cplusplus
}
#endif  // #ifdef __cplusplus
