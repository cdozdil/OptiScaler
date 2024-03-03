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


// @defgroup CAS

#pragma once

// Include the interface for the backend of the CAS API.
#include <FidelityFX/host/ffx_interface.h>

/// @defgroup ffxCas FidelityFX Cas
/// FidelityFX Contrast Adaptive Sharpening runtime library
/// 
/// @ingroup SDKComponents

/// Contrast Adaptive Sharpening major version.
///
/// @ingroup ffxCas
#define FFX_CAS_VERSION_MAJOR (1)

/// Contrast Adaptive Sharpening minor version.
///
/// @ingroup ffxCas
#define FFX_CAS_VERSION_MINOR (1)

/// Contrast Adaptive Sharpening patch version.
///
/// @ingroup ffxCas
#define FFX_CAS_VERSION_PATCH (0)

/// FidelityFX CAS context count
/// 
/// Defines the number of internal effect contexts required by CAS
///
/// @ingroup ffxCas
#define FFX_CAS_CONTEXT_COUNT   1

/// The size of the context specified in 32bit values.
///
/// @ingroup ffxCas
#define FFX_CAS_CONTEXT_SIZE (16536)

#if defined(__cplusplus)
extern "C" {
#endif  // #if defined(__cplusplus)

/// An enumeration of all the passes which constitute the CAS algorithm.
///
/// CAS has only one pass. For a more comprehensive description 
/// of this pass, please refer to the CAS reference documentation.
///
/// @ingroup ffxCas
typedef enum FfxCasPass
{
    FFX_CAS_PASS_SHARPEN = 0,  ///< A pass which sharpens only or upscales the color buffer.

    FFX_CAS_PASS_COUNT  ///< The number of passes performed by CAS.
} FfxCasPass;

/// An enumeration of color space conversions used when creating a
/// <c><i>FfxCASContext</i></c>. See <c><i>FfxCASContextDescription</i></c>.
///
/// @ingroup ffxCas
typedef enum FfxCasColorSpaceConversion
{
    FFX_CAS_COLOR_SPACE_LINEAR            = 0,  ///< Linear color space, will do nothing.
    FFX_CAS_COLOR_SPACE_GAMMA20           = 1,  ///< Convert gamma 2.0 to linear for input and linear to gamma 2.0 for output.
    FFX_CAS_COLOR_SPACE_GAMMA22           = 2,  ///< Convert gamma 2.2 to linear for input and linear to gamma 2.2 for output.
    FFX_CAS_COLOR_SPACE_SRGB_OUTPUT       = 3,  ///< Only do sRGB conversion for output (input conversion will be done automatically).
    FFX_CAS_COLOR_SPACE_SRGB_INPUT_OUTPUT = 4   ///< Convert sRGB to linear for input and linear to sRGB for output.
} FfxCasColorSpaceConversion;

/// An enumeration of bit flags used when creating a
/// <c><i>FfxCASContext</i></c>. See <c><i>FfxCASContextDescription</i></c>.
///
/// @ingroup ffxCas
typedef enum FfxCasInitializationFlagBits
{
    FFX_CAS_SHARPEN_ONLY = (1 << 0)  ///< A bit indicating if we sharpen only.
} FfxCasInitializationFlagBits;

/// A structure encapsulating the parameters required to initialize FidelityFX CAS
///
/// @ingroup ffxCas
typedef struct FfxCasContextDescription
{
    uint32_t                   flags;                 ///< A collection of <c><i>FfxCasInitializationFlagBits</i></c>.
    FfxCasColorSpaceConversion colorSpaceConversion;  ///< An enumeration indicates which color space conversion is used.
    FfxDimensions2D            maxRenderSize;         ///< The maximum size that rendering will be performed at.
    FfxDimensions2D            displaySize;           ///< The size of the presentation resolution targeted by the upscaling process.
    FfxInterface               backendInterface;      ///< A set of pointers to the backend implementation for CAS.
} FfxCasContextDescription;

/// A structure encapsulating the parameters for dispatching the various passes
/// of FidelityFX CAS
///
/// @ingroup ffxCas
typedef struct FfxCasDispatchDescription
{
    FfxCommandList  commandList;  ///< The <c><i>FfxCommandList</i></c> to record CAS rendering commands into.
    FfxResource     color;        ///< A <c><i>FfxResource</i></c> containing the color buffer for the current frame (at render resolution).
    FfxResource     output;       ///< A <c><i>FfxResource</i></c> containing the output color buffer for the current frame (at presentation resolution).
    FfxDimensions2D renderSize;   ///< The resolution that was used for rendering the input resource.
    float           sharpness;    ///< The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness.
} FfxCasDispatchDescription;

/// A structure encapsulating the FidelityFX CAS context.
///
/// This sets up an object which contains all persistent internal data and
/// resources that are required by CAS.
///
/// The <c><i>FfxCasContext</i></c> object should have a lifetime matching
/// your use of CAS. Before destroying the CAS context care should be taken
/// to ensure the GPU is not accessing the resources created or used by CAS.
/// It is therefore recommended that the GPU is idle before destroying the
/// CAS context.
///
/// @ingroup ffxCas
typedef struct FfxCasContext
{
    uint32_t data[FFX_CAS_CONTEXT_SIZE];  ///< An opaque set of <c>uint32_t</c> which contain the data for the context.
} FfxCasContext;

/// Create a FidelityFX CAS context from the parameters
/// programmed to the <c><i>FfxCasContextDescription</i></c> structure.
///
/// The context structure is the main object used to interact with the CAS, 
/// and is responsible for the management of the internal resources
/// used by the CAS algorithm. When this API is called, multiple calls
/// will be made via the pointers contained in the <c><i>callbacks</i></c>
/// structure. These callbacks will attempt to retreive the device capabilities,
/// , and pipelines required by CAS frame-to-frame function. Depending on configuration 
/// used when creating the <c><i>FfxCasContext</i></c> a different set of pipelines 
/// might be requested via the callback functions.
///
/// The <c><i>FfxCasContext</i></c> should be destroyed when use of it is
/// completed, typically when an application is unloaded or CAS is disabled by a user. 
/// To destroy the CAS context you should call <c><i>ffxCasContextDestroy</i></c>.
///
/// @param [out] pContext                A pointer to a <c><i>FfxCasContext</i></c> structure to populate.
/// @param [in]  pContextDescription     A pointer to a <c><i>FfxCasContextDescription</i></c> structure.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>contextDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFX_ERROR_INCOMPLETE_INTERFACE      The operation failed because the <c><i>FfxCasContextDescription.callbacks</i></c>  was not fully specified.
/// @retval
/// FFX_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup ffxCas
FFX_API FfxErrorCode ffxCasContextCreate(FfxCasContext* pContext, const FfxCasContextDescription* pContextDescription);

/// @param [out] pContext                A pointer to a <c><i>FfxCasContext</i></c> structure to populate.
/// @param [in]  pDispatchDescription    A pointer to a <c><i>FfxCasDispatchDescription</i></c> structure.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>dispatchDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFX_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup ffxCas
FFX_API FfxErrorCode ffxCasContextDispatch(FfxCasContext* pContext, const FfxCasDispatchDescription* pDispatchDescription);

/// Destroy the FidelityFX CAS context.
///
/// @param [out] pContext                A pointer to a <c><i>FfxCasContext</i></c> structure to destroy.
///
/// @retval
/// FFX_OK                              The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> was <c><i>NULL</i></c>.
///
/// @ingroup ffxCas
FFX_API FfxErrorCode ffxCasContextDestroy(FfxCasContext* pContext);

#if defined(__cplusplus)
}
#endif  // #if defined(__cplusplus)
