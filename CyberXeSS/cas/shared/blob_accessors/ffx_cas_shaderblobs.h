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

#include "../ffx_shader_blobs.h"
#include <FidelityFX/host/ffx_cas.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif  // #if defined(__cplusplus)

// Get a DX12 shader blob for the specified pass and permutation index.
FfxErrorCode casGetPermutationBlobByIndex(
    FfxCasPass passId, 
    uint32_t permutationOptions, 
    FfxShaderBlob* outBlob);

// Check is Wave64 is requested on this permutation
FfxErrorCode casIsWave64(uint32_t permutationOptions, bool& isWave64);

#if defined(__cplusplus)
}
#endif  // #if defined(__cplusplus)
