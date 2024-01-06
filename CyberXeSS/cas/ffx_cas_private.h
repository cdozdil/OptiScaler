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
#include <FidelityFX/gpu/cas/ffx_cas_resources.h>

typedef enum CasShaderPermutationOptions
{
    CAS_SHADER_PERMUTATION_SHARPEN_ONLY                  = (1 << 0),  ///< Sharpen only, otherwise will upscale the color buffer.
    CAS_SHADER_PERMUTATION_FORCE_WAVE64                  = (1 << 1),  ///< doesn't map to a define, selects different table
    CAS_SHADER_PERMUTATION_ALLOW_FP16                    = (1 << 2),  ///< Enables fast math computations where possible
    CAS_SHADER_PERMUTATION_COLOR_SPACE_LINEAR            = (1 << 3),  ///< Linear color space, will do nothing.
    CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA20           = (1 << 4),  ///< Convert gamma 2.0 to linear for input and linear to gamma 2.0 for output.
    CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA22           = (1 << 5),  ///< Convert gamma 2.2 to linear for input and linear to gamma 2.2 for output.
    CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_OUTPUT       = (1 << 6),  ///< Only do sRGB conversion for output (input conversion will be done automatically).
    CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_INPUT_OUTPUT = (1 << 7),  ///< Convert sRGB to linear for input and linear to sRGB for output.
} Fs1ShaderPermutationOptions;

typedef struct CasConstants
{
    FfxUInt32x4 const0;
    FfxUInt32x4 const1;
} CasConstants;

struct FfxCasContextDescription;
struct FfxDeviceCapabilities;
struct FfxPipelineState;
struct FfxResource;

// FfxCASContext_Private
// The private implementation of the CAS context.
typedef struct FfxCasContext_Private
{
    FfxCasContextDescription contextDescription;
    FfxUInt32                effectContextId;
    CasConstants             constants;
    FfxDevice                device;
    FfxDeviceCapabilities    deviceCapabilities;
    FfxConstantBuffer        constantBuffer;

    FfxPipelineState pipelineSharpen;

    FfxResourceInternal srvResources[FFX_CAS_RESOURCE_IDENTIFIER_COUNT];
    FfxResourceInternal uavResources[FFX_CAS_RESOURCE_IDENTIFIER_COUNT];

} FfxCasContext_Private;
