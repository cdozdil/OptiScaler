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


#include "ffx_shader_blobs.h"

#if defined(FFX_FSR) || defined(FFX_ALL)
#include "blob_accessors/ffx_fsr3upscaler_shaderblobs.h"
#include "blob_accessors/ffx_fsr2_shaderblobs.h"
#include "blob_accessors/ffx_fsr1_shaderblobs.h"
#include "blob_accessors/ffx_frameinterpolation_shaderblobs.h"
#include "blob_accessors/ffx_opticalflow_shaderblobs.h"
#endif // #if defined(FFX_FSR) || defined(FFX_ALL)

#if defined(FFX_FSR1) || defined(FFX_ALL)
#include "blob_accessors/ffx_fsr1_shaderblobs.h"
#endif // #if defined(FFX_FSR1) || defined(FFX_ALL)

#if defined(FFX_FSR2) || defined(FFX_ALL)
#include "blob_accessors/ffx_fsr2_shaderblobs.h"
#endif // #if defined(FFX_FSR2) || defined(FFX_ALL)

#if defined(FFX_FSR3) || defined(FFX_ALL)
#include "blob_accessors/ffx_fsr3upscaler_shaderblobs.h"
#include "blob_accessors/ffx_frameinterpolation_shaderblobs.h"
#include "blob_accessors/ffx_opticalflow_shaderblobs.h"
#endif // #if defined(FFX_FSR3) || defined(FFX_ALL)

#if defined(FFX_FSR3UPSCALER) || defined(FFX_ALL)
#include "blob_accessors/ffx_fsr3upscaler_shaderblobs.h"
#endif // #if defined(FFX_FSR3UPSCALER) || defined(FFX_ALL)

#if defined(FFX_FRAMEINTERPOLATION) || defined(FFX_ALL)
#include "blob_accessors/ffx_frameinterpolation_shaderblobs.h"
#endif // #if defined(FFX_FRAMEINTERPOLATION) || defined(FFX_ALL)

#if defined(FFX_OPTICALFLOW) || defined(FFX_ALL)
#include "blob_accessors/ffx_opticalflow_shaderblobs.h"
#endif // #if defined(FFX_OPTICALFLOW) || defined(FFX_ALL)

#if defined(FFX_SPD) || defined(FFX_ALL)
#include "blob_accessors/ffx_spd_shaderblobs.h"
#endif // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_CACAO) || defined(FFX_ALL)
#include "blob_accessors/ffx_cacao_shaderblobs.h"
#endif // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_LPM) || defined(FFX_ALL)
#include "blob_accessors/ffx_lpm_shaderblobs.h"
#endif  // #if defined(FFX_LPM) || defined(FFX_ALL)

#if defined(FFX_BLUR)|| defined(FFX_ALL)
#include "blob_accessors/ffx_blur_shaderblobs.h"
#endif // #if defined(FFX_BLUR) || defined(FFX_ALL)

#if defined(FFX_VRS) || defined(FFX_ALL)
#include "blob_accessors/ffx_vrs_shaderblobs.h"
#endif // #if defined(FFX_VRS) || defined(FFX_ALL)

#if defined(FFX_CAS) || defined(FFX_ALL)
#include "blob_accessors/ffx_cas_shaderblobs.h"
#endif // #if defined(FFX_CAS) || defined(FFX_ALL)

#if defined(FFX_DOF) || defined(FFX_ALL)
#include "blob_accessors/ffx_dof_shaderblobs.h"
#endif // #if defined(FFX_DOF) || defined(FFX_ALL)

#if defined(FFX_LENS) || defined(FFX_ALL)
#include "blob_accessors/ffx_lens_shaderblobs.h"
#endif  // #if defined(FFX_LENS) || defined(FFX_ALL)

#if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)
#include "blob_accessors/ffx_parallelsort_shaderblobs.h"
#endif  // #if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)

#if defined(FFX_SSSR) || defined(FFX_ALL)
#include "blob_accessors/ffx_sssr_shaderblobs.h"
#endif // #if defined(FFX_SSSR) || defined(FFX_ALL)

#if defined(FFX_DENOISER) || defined(FFX_ALL)
#include "blob_accessors/ffx_denoiser_shaderblobs.h"
#endif // #if defined(FFX_DENOISER) || defined(FFX_ALL)

#if defined(FFX_CLASSIFIER) || defined(FFX_ALL)
#include "blob_accessors/ffx_classifier_shaderblobs.h"
#endif  // #if defined(FFX_CLASSIFIER) || defined(FFX_ALL)

#include <string.h> // for memset

FfxErrorCode ffxGetPermutationBlobByIndex(
    FfxEffect effectId,
    FfxPass passId, 
    FfxBindStage stageId,
    uint32_t permutationOptions,
    FfxShaderBlob* outBlob)
{

    switch (effectId)
    {
#if defined(FFX_FSR) || defined(FFX_ALL)
    case FFX_EFFECT_FSR2:
        return fsr2GetPermutationBlobByIndex((FfxFsr2Pass)passId, permutationOptions, outBlob);
    case FFX_EFFECT_FSR1:
        return fsr1GetPermutationBlobByIndex((FfxFsr1Pass)passId, permutationOptions, outBlob);
    #if defined(FFX_FSR3)
        case FFX_EFFECT_FSR3UPSCALER:
            return fsr3UpscalerGetPermutationBlobByIndex((FfxFsr3UpscalerPass)passId, permutationOptions, outBlob);
        case FFX_EFFECT_FRAMEINTERPOLATION:
            return frameInterpolationGetPermutationBlobByIndex((FfxFrameInterpolationPass)passId, stageId, permutationOptions, outBlob);
        case FFX_EFFECT_OPTICALFLOW:
            return opticalflowGetPermutationBlobByIndex((FfxOpticalflowPass)passId, permutationOptions, outBlob);
    #endif
#endif // #if defined(FFX_FSR) || defined(FFX_ALL)

#if defined(FFX_FSR1) || defined(FFX_ALL)
    case FFX_EFFECT_FSR1:
        return fsr1GetPermutationBlobByIndex((FfxFsr1Pass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_FSR1) || defined(FFX_ALL)

#if defined(FFX_FSR2) || defined(FFX_ALL)
    case FFX_EFFECT_FSR2:
        return fsr2GetPermutationBlobByIndex((FfxFsr2Pass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_FSR2) || defined(FFX_ALL)

#if defined(FFX_FSR3) || defined(FFX_ALL)
    case FFX_EFFECT_FSR3UPSCALER:
        return fsr3UpscalerGetPermutationBlobByIndex((FfxFsr3UpscalerPass)passId, permutationOptions, outBlob);
    case FFX_EFFECT_FRAMEINTERPOLATION:
        return frameInterpolationGetPermutationBlobByIndex((FfxFrameInterpolationPass)passId, stageId, permutationOptions, outBlob);
    case FFX_EFFECT_OPTICALFLOW:
        return opticalflowGetPermutationBlobByIndex((FfxOpticalflowPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_FSR3) || defined(FFX_ALL)

#if defined(FFX_FSR3UPSCALER) || defined(FFX_ALL)
    case FFX_EFFECT_FSR3UPSCALER:
        return fsr3UpscalerGetPermutationBlobByIndex((FfxFsr3UpscalerPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_FSR3UPSCALER) || defined(FFX_ALL)

#if defined(FFX_FRAMEINTERPOLATION) || defined(FFX_ALL)
    case FFX_EFFECT_FRAMEINTERPOLATION:
        return frameInterpolationGetPermutationBlobByIndex((FfxFrameInterpolationPass)passId, stageId, permutationOptions, outBlob);
#endif // #if defined(FFX_FRAMEINTERPOLATION) || defined(FFX_ALL)

#if defined(FFX_OPTICALFLOW) || defined(FFX_ALL)
    case FFX_EFFECT_OPTICALFLOW:
        return opticalflowGetPermutationBlobByIndex((FfxOpticalflowPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_OPTICALFLOW) || defined(FFX_ALL)

#if defined(FFX_SPD) || defined(FFX_ALL)
    case FFX_EFFECT_SPD:
        return spdGetPermutationBlobByIndex((FfxSpdPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_LPM) || defined(FFX_ALL)
    case FFX_EFFECT_LPM:
        return lpmGetPermutationBlobByIndex((FfxLpmPass)passId, permutationOptions, outBlob);
#endif  // #if defined(FFX_LPM) || defined(FFX_ALL)

#if defined(FFX_DOF) || defined(FFX_ALL)
    case FFX_EFFECT_DOF:
        return dofGetPermutationBlobByIndex((FfxDofPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_DOF) || defined(FFX_ALL)

#if defined(FFX_BLUR) || defined(FFX_ALL)
    case FFX_EFFECT_BLUR:
        return blurGetPermutationBlobByIndex((FfxBlurPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_BLUR) || defined(FFX_ALL)

#if defined(FFX_CACAO) || defined(FFX_ALL)
    case FFX_EFFECT_CACAO:
        return cacaoGetPermutationBlobByIndex((FfxCacaoPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_CACAO) || defined(FFX_ALL)

#if defined(FFX_CAS) || defined(FFX_ALL)
    case FFX_EFFECT_CAS:
        return casGetPermutationBlobByIndex((FfxCasPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_CAS)

#if defined(FFX_LENS) || defined(FFX_ALL)
    case FFX_EFFECT_LENS:
        return lensGetPermutationBlobByIndex((FfxLensPass)passId, permutationOptions, outBlob);
#endif  // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)
    case FFX_EFFECT_PARALLEL_SORT:
        return parallelSortGetPermutationBlobByIndex((FfxParallelSortPass)passId, permutationOptions, outBlob);
#endif  // #if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)

#if defined(FFX_VRS) || defined(FFX_ALL)
    case FFX_EFFECT_VARIABLE_SHADING:
        return vrsGetPermutationBlobByIndex((FfxVrsPass)passId, permutationOptions, outBlob);
#endif  // #if defined(FFX_VRS) || defined(FFX_ALL)

#if defined(FFX_DENOISER) || defined(FFX_ALL)
    case FFX_EFFECT_DENOISER:
        return denoiserGetPermutationBlobByIndex((FfxDenoiserPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_DENOISER) || defined(FFX_ALL)

#if defined(FFX_SSSR) || defined(FFX_ALL)
    case FFX_EFFECT_SSSR:
        return sssrGetPermutationBlobByIndex((FfxSssrPass)passId, permutationOptions, outBlob);
#endif // #if defined(FFX_SSSR) || defined(FFX_ALL)

#if defined(FFX_CLASSIFIER) || defined(FFX_ALL)
    case FFX_EFFECT_CLASSIFIER:
        return classifierGetPermutationBlobByIndex((FfxClassifierPass)passId, permutationOptions, outBlob);
#endif  // #if defined(FFX_HYBRIDSHADOWS) || defined(FFX_ALL)
    default:
        FFX_ASSERT_MESSAGE(false, "Not implemented");
        break;
    }

    // return an empty blob
    memset(outBlob, 0, sizeof(FfxShaderBlob));
    return FFX_OK;
}

FfxErrorCode ffxIsWave64(FfxEffect effectId, uint32_t permutationOptions, bool& isWave64)
{
switch (effectId)
    {
#if defined(FFX_FSR) || defined(FFX_ALL)
    case FFX_EFFECT_FSR2:
        return fsr2IsWave64(permutationOptions, isWave64);

    case FFX_EFFECT_FSR1:
        return fsr1IsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_FSR) || defined(FFX_ALL)

#if defined(FFX_FSR1) || defined(FFX_ALL)
    case FFX_EFFECT_FSR1:
        return fsr1IsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_FSR1) || defined(FFX_ALL)

#if defined(FFX_FSR2) || defined(FFX_ALL)
    case FFX_EFFECT_FSR2:
        return fsr2IsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_FSR2) || defined(FFX_ALL)

#if defined(FFX_SPD) || defined(FFX_ALL)
    case FFX_EFFECT_SPD:
        return spdIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_LPM) || defined(FFX_ALL)
    case FFX_EFFECT_LPM:
        return lpmIsWave64(permutationOptions, isWave64);
#endif  // #if defined(FFX_LPM) || defined(FFX_ALL)

#if defined(FFX_DOF) || defined(FFX_ALL)
    case FFX_EFFECT_DOF:
        return dofIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_DOF) || defined(FFX_ALL)

#if defined(FFX_BLUR) || defined(FFX_ALL)
    case FFX_EFFECT_BLUR:
        return blurIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_BLUR) || defined(FFX_ALL)

#if defined(FFX_CACAO) || defined(FFX_ALL)
    case FFX_EFFECT_CACAO:
        return cacaoIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_CACAO) || defined(FFX_ALL)

#if defined(FFX_CAS) || defined(FFX_ALL)
    case FFX_EFFECT_CAS:
        return casIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_CAS)

#if defined(FFX_LENS) || defined(FFX_ALL)
    case FFX_EFFECT_LENS:
        return lensIsWave64(permutationOptions, isWave64);
#endif  // #if defined(FFX_SPD) || defined(FFX_ALL)

#if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)
    case FFX_EFFECT_PARALLEL_SORT:
        return parallelSortIsWave64(permutationOptions, isWave64);
#endif  // #if defined(FFX_PARALLEL_SORT) || defined(FFX_ALL)

#if defined(FFX_VRS) || defined(FFX_ALL)
    case FFX_EFFECT_VARIABLE_SHADING:
        return vrsIsWave64(permutationOptions, isWave64);
#endif  // #if defined(FFX_VRS) || defined(FFX_ALL)

#if defined(FFX_DENOISER) || defined(FFX_ALL)
    case FFX_EFFECT_DENOISER:
        return denoiserIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_DENOISER) || defined(FFX_ALL)

#if defined(FFX_SSSR) || defined(FFX_ALL)
    case FFX_EFFECT_SSSR:
        return sssrIsWave64(permutationOptions, isWave64);
#endif // #if defined(FFX_SSSR) || defined(FFX_ALL)

#if defined(FFX_CLASSIFIER) || defined(FFX_ALL)
    case FFX_EFFECT_CLASSIFIER:
        return classifierIsWave64(permutationOptions, isWave64);
#endif  // #if defined(FFX_HYBRIDSHADOWS) || defined(FFX_ALL)
    default:
        FFX_ASSERT_MESSAGE(false, "Not implemented");
        isWave64 = false;
        break;
    }
    
    return FFX_ERROR_BACKEND_API_ERROR;
}
