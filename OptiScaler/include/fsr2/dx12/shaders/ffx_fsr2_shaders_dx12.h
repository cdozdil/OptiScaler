// This file is part of the FidelityFX SDK.
//
// Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All rights reserved.
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
#include "../../ffx_fsr2_interface.h"

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)

// A single shader blob and a description of its resources.
typedef struct Fsr2ShaderBlobDX12 {

    const uint8_t*  data;               // A pointer to the blob 
    uint32_t        size;               // Size in bytes.
    uint32_t        uavCount;           // Number of UAV.
    uint32_t        srvCount;           // Number of SRV.
    uint32_t        cbvCount;           // Number of CBs.
    const char**    boundUAVResourceNames;
    const uint32_t* boundUAVResources;  // Pointer to an array of bound UAV resources.
    const char**    boundSRVResourceNames;
    const uint32_t* boundSRVResources;  // Pointer to an array of bound SRV resources.
    const char**    boundCBVResourceNames;
    const uint32_t* boundCBVResources;   // Pointer to an array of bound ConstantBuffers.
} Fsr2ShaderBlobDX12;

// The different options which contribute to permutations.
typedef enum Fs2ShaderPermutationOptionsDX12 {

    FSR2_SHADER_PERMUTATION_USE_LANCZOS_TYPE        = (1<<0),    // FFX_FSR2_OPTION_REPROJECT_USE_LANCZOS_TYPE. Off means reference, On means LUT
    FSR2_SHADER_PERMUTATION_HDR_COLOR_INPUT         = (1<<1),    // FFX_FSR2_OPTION_HDR_COLOR_INPUT
    FSR2_SHADER_PERMUTATION_LOW_RES_MOTION_VECTORS  = (1<<2),    // FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
    FSR2_SHADER_PERMUTATION_JITTER_MOTION_VECTORS   = (1<<3),    // FFX_FSR2_OPTION_JITTERED_MOTION_VECTORS
    FSR2_SHADER_PERMUTATION_DEPTH_INVERTED          = (1<<4),    // FFX_FSR2_OPTION_INVERTED_DEPTH
    FSR2_SHADER_PERMUTATION_ENABLE_SHARPENING       = (1<<5),    // FFX_FSR2_OPTION_APPLY_SHARPENING
    FSR2_SHADER_PERMUTATION_FORCE_WAVE64            = (1<<6),    // doesn't map to a define, selects different table
    FSR2_SHADER_PERMUTATION_ALLOW_FP16              = (1<<7),    // FFX_USE_16BIT
} Fs2ShaderPermutationOptionsDX12;

// Get a DX12 shader blob for the specified pass and permutation index.
Fsr2ShaderBlobDX12 fsr2GetPermutationBlobByIndexDX12(FfxFsr2Pass passId, uint32_t permutationOptions);

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)
