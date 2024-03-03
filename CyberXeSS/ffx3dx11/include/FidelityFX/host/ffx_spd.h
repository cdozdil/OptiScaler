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


// @defgroup FSR2

#pragma once

// Include the interface for the backend of the FSR2 API.
#include <FidelityFX/host/ffx_interface.h>

/// @defgroup FfxSpd FidelityFX SPD
/// FidelityFX Single Pass Downsampler runtime library
/// 
/// @ingroup SDKComponents

/// FidelityFX SPD major version.
///
/// @ingroup FfxSpd
#define FFX_SPD_VERSION_MAJOR      (2)

/// FidelityFX SPD minor version.
///
/// @ingroup FfxSpd
#define FFX_SPD_VERSION_MINOR (1)

/// FidelityFX SPD patch version.
///
/// @ingroup FfxSpd
#define FFX_SPD_VERSION_PATCH (0)

/// FidelityFX SPD context count
/// 
/// Defines the number of internal effect contexts required by SPD
///
/// @ingroup FfxSpd
#define FFX_SPD_CONTEXT_COUNT   1

/// The size of the context specified in 32bit values.
///
/// @ingroup FfxSpd
#define FFX_SPD_CONTEXT_SIZE       (16536)

/// If this ever changes, need to also reflect a change in number
/// of resources in ffx_spd_resources.h
/// 
/// @ingroup FfxSpd
#define SPD_MAX_MIP_LEVELS 12

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)

/// An enumeration of the pass which constitutes the SPD algorithm.
///
/// SPD is implemented as a single pass algorithm. Each call to the
/// <c><i>FfxSPDScheduleGpuJobFunc</i></c> callback function will
/// correspond to a single downsample job. For a
/// more comprehensive description of SPD's inner workings, please
/// refer to the SPD reference documentation.
///
/// @ingroup FfxSpd
typedef enum FfxSpdPass
{

    FFX_SPD_PASS_DOWNSAMPLE = 0,    ///< A pass which which downsamples all mips

    FFX_SPD_PASS_COUNT              ///< The number of passes in SPD
} FfxSpdPass;

typedef enum FfxSpdDownsampleFilter
{
    FFX_SPD_DOWNSAMPLE_FILTER_MEAN = 0,
    FFX_SPD_DOWNSAMPLE_FILTER_MIN,
    FFX_SPD_DOWNSAMPLE_FILTER_MAX,
    FFX_SPD_DOWNSAMPLE_FILTER_COUNT
} FfxSpdDownsampleFilter;

/// An enumeration of bit flags used when creating a
/// <c><i>FfxSpdContext</i></c>. See <c><i>FfxSpdContextDescription</i></c>.
///
/// @ingroup FfxSpd
typedef enum FfxSpdInitializationFlagBits {

    FFX_SPD_SAMPLER_LOAD            = (1 << 0),     ///< A bit indicating if we should use resource loads (favor loads over sampler)
    FFX_SPD_SAMPLER_LINEAR          = (1 << 1),     ///< A bit indicating if we should use sampler to load resources.
    FFX_SPD_WAVE_INTEROP_LDS        = (1 << 2),     ///< A bit indicating if we should use LDS
    FFX_SPD_WAVE_INTEROP_WAVE_OPS   = (1 << 3),     ///< A bit indicating if we should use WAVE OPS (favor wave ops over LDS)
    FFX_SPD_MATH_NONPACKED          = (1 << 4),     ///< A bit indicating if we should use floating point math
    FFX_SPD_MATH_PACKED             = (1 << 5)      ///< A bit indicating if we should use 16-bit half precision floating point math (favored)

} FfxSpdInitializationFlagBits;

/// A structure encapsulating the parameters required to initialize FidelityFX
/// Single Pass Downsampler.
///
/// @ingroup FfxSpd
typedef struct FfxSpdContextDescription {

    uint32_t                    flags;                              ///< A collection of <c><i>FfxSpdInitializationFlagBits</i></c>
    FfxSpdDownsampleFilter      downsampleFilter;
    FfxInterface                backendInterface;                   ///< A set of pointers to the backend implementation for FidelityFX.

} FfxSpdContextDescription;

/// A structure encapsulating the parameters for dispatching
/// of FidelityFX Single Pass Downsampler
///
/// @ingroup FfxSpd
typedef struct FfxSpdDispatchDescription {

    FfxCommandList              commandList;        ///< The <c><i>FfxCommandList</i></c> to record rendering commands into.
    FfxResource                 resource;           ///< The <c><i>FfxResource</i></c> to downsample

} FfxSpdDispatchDescription;

/// A structure encapsulating the FidelityFX single pass downsampler context.
///
/// This sets up an object which contains all persistent internal data and
/// resources that are required by SPD.
///
/// The <c><i>FfxSpdContext</i></c> object should have a lifetime matching
/// your use of FSR1. Before destroying the SPD context care should be taken
/// to ensure the GPU is not accessing the resources created or used by SPD.
/// It is therefore recommended that the GPU is idle before destroying the
/// SPD context.
///
/// @ingroup FfxSpd
typedef struct FfxSpdContext {

    uint32_t                    data[FFX_SPD_CONTEXT_SIZE];  ///< An opaque set of <c>uint32_t</c> which contain the data for the context.
} FfxSpdContext;

/// Create a FidelityFX Single Pass Downsampler context from the parameters
/// programmed to the <c><i>FfxSpdContextDescription</i></c> structure.
///
/// The context structure is the main object used to interact with the Single
/// Pass Downsampler API, and is responsible for the management of the internal resources
/// used by the SPD algorithm. When this API is called, multiple calls
/// will be made via the pointers contained in the <c><i>backendInterface</i></c>
/// structure. This backend will attempt to retrieve the device capabilities,
/// and create the internal resources, and pipelines required by SPD to function.
/// Depending on the precise configuration used when
/// creating the <c><i>FfxSpdContext</i></c> a different set of resources and
/// pipelines might be requested via the callback functions.
///
/// The <c><i>FfxSpdContext</i></c> should be destroyed when use of it is
/// completed, typically when an application is unloaded or SPD
/// upscaling is disabled by a user. To destroy the SPD context you
/// should call <c><i>ffxSpdContextDestroy</i></c>.
///
/// @param [out] pContext                A pointer to a <c><i>FfxSpdContext</i></c> structure to populate.
/// @param [in]  pContextDescription     A pointer to a <c><i>FfxSpdContextDescription</i></c> structure.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>contextDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFX_ERROR_INCOMPLETE_INTERFACE      The operation failed because the <c><i>FfxSpdContextDescription.callbacks</i></c>  was not fully specified.
/// @retval
/// FFX_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup FfxSpd
FFX_API FfxErrorCode ffxSpdContextCreate(FfxSpdContext* pContext, const FfxSpdContextDescription* pContextDescription);

/// Dispatches work to the FidelityFX SPD context
///
/// @param [out] pContext                A pointer to a <c><i>FfxSpdContext</i></c> structure to populate.
/// @param [in]  pDispatchDescription    A pointer to a <c><i>FfxSpdDispatchDescription</i></c> structure.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>dispatchDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFX_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup FfxSpd
FFX_API FfxErrorCode ffxSpdContextDispatch(FfxSpdContext* pContext, const FfxSpdDispatchDescription* pDispatchDescription);

/// Destroy the FidelityFX SPD context.
///
/// @param [out] pContext                A pointer to a <c><i>FfxSpdContext</i></c> structure to destroy.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> was <c><i>NULL</i></c>.
///
/// @ingroup FfxSpd
FFX_API FfxErrorCode ffxSpdContextDestroy(FfxSpdContext* pContext);

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)
