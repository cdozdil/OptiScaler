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


#include <string.h>  // for memset
#include <stdlib.h>  // for _countof
#include <cmath>     // for fabs, abs, sinf, sqrt, etc.

#include <FidelityFX/host/ffx_cas.h>
#include <FidelityFX/gpu/ffx_core.h>
#include <FidelityFX/gpu/cas/ffx_cas.h>
#include "ffx_object_management.h"

#include "ffx_cas_private.h"

// lists to map shader resource bindpoint name to resource identifier
typedef struct ResourceBinding
{
    uint32_t index;
    wchar_t  name[64];
} ResourceBinding;

static const ResourceBinding s_SrvResourceBindingTable[] = {
    {FFX_CAS_RESOURCE_IDENTIFIER_INPUT_COLOR, L"r_input_color"},
};

static const ResourceBinding s_UavResourceBindingTable[] = {
    {FFX_CAS_RESOURCE_IDENTIFIER_OUTPUT_COLOR, L"rw_output_color"},
};

static const ResourceBinding s_CbResourceBindingTable[] = {
    {FFX_CAS_CONSTANTBUFFER_IDENTIFIER_CAS, L"cbCAS"},
};

static FfxErrorCode patchResourceBindings(FfxPipelineState* inoutPipeline)
{
    for (uint32_t srvIndex = 0; srvIndex < inoutPipeline->srvTextureCount; ++srvIndex)
    {
        int32_t mapIndex = 0;
        for (mapIndex = 0; mapIndex < _countof(s_SrvResourceBindingTable); ++mapIndex)
        {
            if (0 == wcscmp(s_SrvResourceBindingTable[mapIndex].name, inoutPipeline->srvTextureBindings[srvIndex].name))
                break;
        }
        if (mapIndex == _countof(s_SrvResourceBindingTable))
            return FFX_ERROR_INVALID_ARGUMENT;

        inoutPipeline->srvTextureBindings[srvIndex].resourceIdentifier = s_SrvResourceBindingTable[mapIndex].index;
    }

    for (uint32_t uavIndex = 0; uavIndex < inoutPipeline->uavTextureCount; ++uavIndex)
    {
        int32_t mapIndex = 0;
        for (mapIndex = 0; mapIndex < _countof(s_UavResourceBindingTable); ++mapIndex)
        {
            if (0 == wcscmp(s_UavResourceBindingTable[mapIndex].name, inoutPipeline->uavTextureBindings[uavIndex].name))
                break;
        }
        if (mapIndex == _countof(s_UavResourceBindingTable))
            return FFX_ERROR_INVALID_ARGUMENT;

        inoutPipeline->uavTextureBindings[uavIndex].resourceIdentifier = s_UavResourceBindingTable[mapIndex].index;
    }

    for (uint32_t cbIndex = 0; cbIndex < inoutPipeline->constCount; ++cbIndex)
    {
        int32_t mapIndex = 0;
        for (mapIndex = 0; mapIndex < _countof(s_CbResourceBindingTable); ++mapIndex)
        {
            if (0 == wcscmp(s_CbResourceBindingTable[mapIndex].name, inoutPipeline->constantBufferBindings[cbIndex].name))
                break;
        }
        if (mapIndex == _countof(s_CbResourceBindingTable))
            return FFX_ERROR_INVALID_ARGUMENT;

        inoutPipeline->constantBufferBindings[cbIndex].resourceIdentifier = s_CbResourceBindingTable[mapIndex].index;
    }

    return FFX_OK;
}

static uint32_t getPipelinePermutationFlags(uint32_t contextFlags, FfxCasPass passId, FfxCasColorSpaceConversion colorSpaceConversion, bool fp16, bool force64)
{
    // work out what permutation to load.
    uint32_t flags = 0;
    flags |= (contextFlags & FFX_CAS_SHARPEN_ONLY) ? CAS_SHADER_PERMUTATION_SHARPEN_ONLY : 0;
    flags |= (force64) ? CAS_SHADER_PERMUTATION_FORCE_WAVE64 : 0;
    flags |= (fp16) ? CAS_SHADER_PERMUTATION_ALLOW_FP16 : 0;
    switch (colorSpaceConversion)
    {
    case FFX_CAS_COLOR_SPACE_LINEAR:
        flags |= CAS_SHADER_PERMUTATION_COLOR_SPACE_LINEAR;
        break;
    case FFX_CAS_COLOR_SPACE_GAMMA20:
        flags |= CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA20;
        break;
    case FFX_CAS_COLOR_SPACE_GAMMA22:
        flags |= CAS_SHADER_PERMUTATION_COLOR_SPACE_GAMMA22;
        break;
    case FFX_CAS_COLOR_SPACE_SRGB_OUTPUT:
        flags |= CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_OUTPUT;
        break;
    case FFX_CAS_COLOR_SPACE_SRGB_INPUT_OUTPUT:
        flags |= CAS_SHADER_PERMUTATION_COLOR_SPACE_SRGB_INPUT_OUTPUT;
        break;
    default:
        break;
    }

    return flags;
}

static FfxErrorCode createPipelineStates(FfxCasContext_Private* context)
{
    FFX_ASSERT(context);

    FfxPipelineDescription pipelineDescription = {};
    pipelineDescription.contextFlags = context->contextDescription.flags;

    pipelineDescription.samplerCount  = 1;
    FfxSamplerDescription samplers[1] = {{FFX_FILTER_TYPE_MINMAGMIP_POINT, FFX_ADDRESS_MODE_CLAMP, FFX_ADDRESS_MODE_CLAMP, FFX_ADDRESS_MODE_CLAMP, FFX_BIND_COMPUTE_SHADER_STAGE}};
    pipelineDescription.samplers      = samplers;

    pipelineDescription.rootConstantBufferCount     = 1;
    FfxRootConstantDescription rootConstantsDecs[1] = {{sizeof(CasConstants) / sizeof(uint32_t), FFX_BIND_COMPUTE_SHADER_STAGE}};
    pipelineDescription.rootConstants               = rootConstantsDecs;

    // Query device capabilities
    FfxDeviceCapabilities capabilities;
    context->contextDescription.backendInterface.fpGetDeviceCapabilities(&context->contextDescription.backendInterface, &capabilities);

    // Setup a few options used to determine permutation flags
    bool haveShaderModel66 = capabilities.minimumSupportedShaderModel >= FFX_SHADER_MODEL_6_6;
    bool supportedFP16     = capabilities.fp16Supported;
    bool canForceWave64    = false;

    const uint32_t waveLaneCountMin = capabilities.waveLaneCountMin;
    const uint32_t waveLaneCountMax = capabilities.waveLaneCountMax;
    if (waveLaneCountMin == 32 && waveLaneCountMax == 64)
        canForceWave64 = haveShaderModel66;
    else
        canForceWave64 = false;

    // Work out what permutation to load.
    uint32_t                   contextFlags         = context->contextDescription.flags;
    FfxCasColorSpaceConversion colorSpaceConversion = context->contextDescription.colorSpaceConversion;

    // Set up pipeline descriptors (basically RootSignature and binding)
    wcscpy_s(pipelineDescription.name, L"CAS_SHARPEN");
    FFX_VALIDATE(context->contextDescription.backendInterface.fpCreatePipeline(
        &context->contextDescription.backendInterface,
        FFX_EFFECT_CAS,
        FFX_CAS_PASS_SHARPEN,
        getPipelinePermutationFlags(contextFlags, FFX_CAS_PASS_SHARPEN, colorSpaceConversion, supportedFP16, canForceWave64),
        &pipelineDescription, context->effectContextId,
        &context->pipelineSharpen));

    // For each pipeline: re-route/fix-up IDs based on names
    patchResourceBindings(&context->pipelineSharpen);

    return FFX_OK;
}

static void scheduleDispatch(
    FfxCasContext_Private* context, const FfxCasDispatchDescription* params, const FfxPipelineState* pipeline, uint32_t dispatchX, uint32_t dispatchY)
{
    FfxGpuJobDescription dispatchJob = {FFX_GPU_JOB_COMPUTE};

    for (uint32_t currentShaderResourceViewIndex = 0; currentShaderResourceViewIndex < pipeline->srvTextureCount; ++currentShaderResourceViewIndex)
    {
        const uint32_t            currentResourceId               = pipeline->srvTextureBindings[currentShaderResourceViewIndex].resourceIdentifier;
        const FfxResourceInternal currentResource                 = context->srvResources[currentResourceId];
        dispatchJob.computeJobDescriptor.srvTextures[currentShaderResourceViewIndex] = currentResource;
        wcscpy_s(dispatchJob.computeJobDescriptor.srvTextureNames[currentShaderResourceViewIndex],
                 pipeline->srvTextureBindings[currentShaderResourceViewIndex].name);
    }

    for (uint32_t currentUnorderedAccessViewIndex = 0; currentUnorderedAccessViewIndex < pipeline->uavTextureCount; ++currentUnorderedAccessViewIndex)
    {
        const uint32_t currentResourceId = pipeline->uavTextureBindings[currentUnorderedAccessViewIndex].resourceIdentifier;
        wcscpy_s(dispatchJob.computeJobDescriptor.uavTextureNames[currentUnorderedAccessViewIndex],
                 pipeline->uavTextureBindings[currentUnorderedAccessViewIndex].name);

        const FfxResourceInternal currentResource                     = context->uavResources[currentResourceId];
        dispatchJob.computeJobDescriptor.uavTextures[currentUnorderedAccessViewIndex] = currentResource;
        dispatchJob.computeJobDescriptor.uavTextureMips[currentUnorderedAccessViewIndex] = 0;
    }

    dispatchJob.computeJobDescriptor.dimensions[0] = dispatchX;
    dispatchJob.computeJobDescriptor.dimensions[1] = dispatchY;
    dispatchJob.computeJobDescriptor.dimensions[2] = 1;
    dispatchJob.computeJobDescriptor.pipeline      = *pipeline;

    wcscpy_s(dispatchJob.computeJobDescriptor.cbNames[0], pipeline->constantBufferBindings[0].name);
    dispatchJob.computeJobDescriptor.cbs[0] = context->constantBuffer;

    
    context->contextDescription.backendInterface.fpScheduleGpuJob(&context->contextDescription.backendInterface, &dispatchJob);
}

static FfxErrorCode casDispatch(FfxCasContext_Private* context, const FfxCasDispatchDescription* params)
{
    // take a short cut to the command list
    FfxCommandList commandList = params->commandList;

    // Register resources for frame
    context->contextDescription.backendInterface.fpRegisterResource(
        &context->contextDescription.backendInterface, &params->color, context->effectContextId, &context->srvResources[FFX_CAS_RESOURCE_IDENTIFIER_INPUT_COLOR]);

    context->contextDescription.backendInterface.fpRegisterResource(
        &context->contextDescription.backendInterface, &params->output, context->effectContextId, &context->uavResources[FFX_CAS_RESOURCE_IDENTIFIER_OUTPUT_COLOR]);

    // This value is the image region dimension that each thread group of the CAS shader operates on
    static const int threadGroupWorkRegionDim = 16;
    int              dispatchX                = FFX_DIVIDE_ROUNDING_UP(context->contextDescription.displaySize.width, threadGroupWorkRegionDim);
    int              dispatchY                = FFX_DIVIDE_ROUNDING_UP(context->contextDescription.displaySize.height, threadGroupWorkRegionDim);

    // Cas constants
    ffxCasSetup(reinterpret_cast<FfxUInt32*>(&context->constants.const0),
                reinterpret_cast<FfxUInt32*>(&context->constants.const1),
                params->sharpness,
                static_cast<FfxFloat32>(params->renderSize.width),
                static_cast<FfxFloat32>(params->renderSize.height),
                static_cast<FfxFloat32>(context->contextDescription.displaySize.width),
                static_cast<FfxFloat32>(context->contextDescription.displaySize.height));
    memcpy(&context->constantBuffer.data, &context->constants, sizeof(CasConstants));
    scheduleDispatch(context, params, &context->pipelineSharpen, dispatchX, dispatchY);

    // Execute all the work for the frame
    context->contextDescription.backendInterface.fpExecuteGpuJobs(&context->contextDescription.backendInterface, commandList);

    // Release dynamic resources
    context->contextDescription.backendInterface.fpUnregisterResources(&context->contextDescription.backendInterface, commandList, context->effectContextId);

    return FFX_OK;
}

static FfxErrorCode casCreate(FfxCasContext_Private* context, const FfxCasContextDescription* contextDescription)
{
    FFX_ASSERT(context);
    FFX_ASSERT(contextDescription);

    // Setup the data for implementation.
    memset(context, 0, sizeof(FfxCasContext_Private));
    context->device = contextDescription->backendInterface.device;

    memcpy(&context->contextDescription, contextDescription, sizeof(FfxCasContextDescription));

    context->constantBuffer.num32BitEntries = sizeof(CasConstants) / sizeof(uint32_t);

    // Create the context.
    FfxErrorCode errorCode =
        context->contextDescription.backendInterface.fpCreateBackendContext(&context->contextDescription.backendInterface, &context->effectContextId);
    FFX_RETURN_ON_ERROR(errorCode == FFX_OK, errorCode);

    // Call out for device caps.
    errorCode = context->contextDescription.backendInterface.fpGetDeviceCapabilities(
        &context->contextDescription.backendInterface, &context->deviceCapabilities);
    FFX_RETURN_ON_ERROR(errorCode == FFX_OK, errorCode);

    // Clear the SRV resources to NULL.
    memset(context->srvResources, 0, sizeof(context->srvResources));

    // And copy resources to uavResrouces list
    memcpy(context->uavResources, context->srvResources, sizeof(context->srvResources));

    // Create shaders on initialize.
    errorCode = createPipelineStates(context);
    FFX_RETURN_ON_ERROR(errorCode == FFX_OK, errorCode);

    return FFX_OK;
}

static FfxErrorCode casRelease(FfxCasContext_Private* context)
{
    FFX_ASSERT(context);

    // Release all pipelines
    ffxSafeReleasePipeline(&context->contextDescription.backendInterface, &context->pipelineSharpen, context->effectContextId);

    // Unregister resources not created internally
    context->srvResources[FFX_CAS_RESOURCE_IDENTIFIER_INPUT_COLOR] = {FFX_CAS_RESOURCE_IDENTIFIER_NULL};

    // Destroy the context
    context->contextDescription.backendInterface.fpDestroyBackendContext(&context->contextDescription.backendInterface, context->effectContextId);

    return FFX_OK;
}

FfxErrorCode ffxCasContextCreate(FfxCasContext* context, const FfxCasContextDescription* contextDescription)
{
    // Zero context memory
    memset(context, 0, sizeof(FfxCasContext));

    // Check pointers are valid.
    FFX_RETURN_ON_ERROR(context, FFX_ERROR_INVALID_POINTER);
    FFX_RETURN_ON_ERROR(contextDescription, FFX_ERROR_INVALID_POINTER);

    // Validate that all callbacks are set for the interface
    FFX_RETURN_ON_ERROR(contextDescription->backendInterface.fpGetDeviceCapabilities, FFX_ERROR_INCOMPLETE_INTERFACE);
    FFX_RETURN_ON_ERROR(contextDescription->backendInterface.fpCreateBackendContext, FFX_ERROR_INCOMPLETE_INTERFACE);
    FFX_RETURN_ON_ERROR(contextDescription->backendInterface.fpDestroyBackendContext, FFX_ERROR_INCOMPLETE_INTERFACE);

    // If a scratch buffer is declared, then we must have a size
    if (contextDescription->backendInterface.scratchBuffer)
    {
        FFX_RETURN_ON_ERROR(contextDescription->backendInterface.scratchBufferSize, FFX_ERROR_INCOMPLETE_INTERFACE);
    }

    // Ensure the context is large enough for the internal context.
    FFX_STATIC_ASSERT(sizeof(FfxCasContext) >= sizeof(FfxCasContext_Private));

    // create the context.
    FfxCasContext_Private* contextPrivate = (FfxCasContext_Private*)(context);
    const FfxErrorCode     errorCode      = casCreate(contextPrivate, contextDescription);

    return errorCode;
}

FfxErrorCode ffxCasContextDestroy(FfxCasContext* context)
{
    FFX_RETURN_ON_ERROR(context, FFX_ERROR_INVALID_POINTER);

    // Destroy the context.
    FfxCasContext_Private* contextPrivate = (FfxCasContext_Private*)(context);
    const FfxErrorCode     errorCode      = casRelease(contextPrivate);
    return errorCode;
}

FfxErrorCode ffxCasContextDispatch(FfxCasContext* context, const FfxCasDispatchDescription* dispatchDescription)
{
    // check pointers are valid
    FFX_RETURN_ON_ERROR(context, FFX_ERROR_INVALID_POINTER);
    FFX_RETURN_ON_ERROR(dispatchDescription, FFX_ERROR_INVALID_POINTER);

    FfxCasContext_Private* contextPrivate = (FfxCasContext_Private*)(context);

    // validate that renderSize is within the maximum.
    FFX_RETURN_ON_ERROR(dispatchDescription->renderSize.width <= contextPrivate->contextDescription.maxRenderSize.width, FFX_ERROR_OUT_OF_RANGE);
    FFX_RETURN_ON_ERROR(dispatchDescription->renderSize.height <= contextPrivate->contextDescription.maxRenderSize.height, FFX_ERROR_OUT_OF_RANGE);
    FFX_RETURN_ON_ERROR(contextPrivate->device, FFX_ERROR_NULL_DEVICE);

    // dispatch the CAS passes.
    const FfxErrorCode errorCode = casDispatch(contextPrivate, dispatchDescription);
    return errorCode;
}
