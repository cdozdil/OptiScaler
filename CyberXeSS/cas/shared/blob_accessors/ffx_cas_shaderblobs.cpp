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


#include <FidelityFX/host/ffx_util.h>
#include "ffx_cas_shaderblobs.h"
#include "../../ffx_cas_private.h"

#include "ffx_cas_sharpen_pass_permutations.h"

#include "ffx_cas_sharpen_pass_wave64_permutations.h"

#include "ffx_cas_sharpen_pass_16bit_permutations.h"

#include "ffx_cas_sharpen_pass_wave64_16bit_permutations.h"

#include <string.h>  // for memset

#if defined(POPULATE_PERMUTATION_KEY)
#undef POPULATE_PERMUTATION_KEY
#endif  // #if defined(POPULATE_PERMUTATION_KEY)
#define POPULATE_PERMUTATION_KEY(options, key)                                                     \
key.index = 0;                                                                                     \
key.FFX_CAS_OPTION_SHARPEN_ONLY = FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_SHARPEN_ONLY); \
if (FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_COLOR_SPACE_LINEAR))                         \
    key.FFX_CAS_COLOR_SPACE_CONVERSION = 0;                                                        \
else if (FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA20))                   \
    key.FFX_CAS_COLOR_SPACE_CONVERSION = 1;                                                        \
else if (FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA22))                   \
    key.FFX_CAS_COLOR_SPACE_CONVERSION = 2;                                                        \
else if (FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_OUTPUT))               \
    key.FFX_CAS_COLOR_SPACE_CONVERSION = 3;                                                        \
else if (FFX_CONTAINS_FLAG(options, CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_INPUT_OUTPUT))         \
    key.FFX_CAS_COLOR_SPACE_CONVERSION = 4;

static FfxShaderBlob casGetSharpenPassPermutationBlobByIndex(uint32_t permutationOptions, bool isWave64, bool is16bit)
{
    ffx_cas_sharpen_pass_PermutationKey key;
    POPULATE_PERMUTATION_KEY(permutationOptions, key);

    if (isWave64)
    {
        if (is16bit)
        {
            const int32_t tableIndex = g_ffx_cas_sharpen_pass_wave64_16bit_IndirectionTable[key.index];
            return POPULATE_SHADER_BLOB_FFX(g_ffx_cas_sharpen_pass_wave64_16bit_PermutationInfo, tableIndex);
        }
        else
        {
            const int32_t tableIndex = g_ffx_cas_sharpen_pass_wave64_IndirectionTable[key.index];
            return POPULATE_SHADER_BLOB_FFX(g_ffx_cas_sharpen_pass_wave64_PermutationInfo, tableIndex);
        }
    }
    else
    {
        if (is16bit)
        {
            const int32_t tableIndex = g_ffx_cas_sharpen_pass_16bit_IndirectionTable[key.index];
            return POPULATE_SHADER_BLOB_FFX(g_ffx_cas_sharpen_pass_16bit_PermutationInfo, tableIndex);
        }
        else
        {
            const int32_t tableIndex = g_ffx_cas_sharpen_pass_IndirectionTable[key.index];
            return POPULATE_SHADER_BLOB_FFX(g_ffx_cas_sharpen_pass_PermutationInfo, tableIndex);
        }
    }
}

FfxErrorCode casGetPermutationBlobByIndex(FfxCasPass passId, uint32_t permutationOptions, FfxShaderBlob* outBlob)
{
    bool isWave64 = FFX_CONTAINS_FLAG(permutationOptions, CAS_SHADER_PERMUTATION_FORCE_WAVE64);
    bool is16bit  = FFX_CONTAINS_FLAG(permutationOptions, CAS_SHADER_PERMUTATION_ALLOW_FP16);

    switch (passId)
    {
    case FFX_CAS_PASS_SHARPEN:
    {
        FfxShaderBlob blob = casGetSharpenPassPermutationBlobByIndex(permutationOptions, isWave64, is16bit);
        memcpy(outBlob, &blob, sizeof(FfxShaderBlob));
        return FFX_OK;
    }

    default:
        FFX_ASSERT_FAIL("Should never reach here.");
        break;
    }

    // return an empty blob
    memset(outBlob, 0, sizeof(FfxShaderBlob));
    return FFX_OK;
}

FfxErrorCode casIsWave64(uint32_t permutationOptions, bool& isWave64)
{
    isWave64 = FFX_CONTAINS_FLAG(permutationOptions, CAS_SHADER_PERMUTATION_FORCE_WAVE64);
    return FFX_OK;
}
