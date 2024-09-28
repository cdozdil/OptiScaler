#pragma once
#include "../pch.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

struct alignas(256) DUConstants
{
    XMFLOAT2 sourceResolution;
    XMFLOAT2 inverseResolution;
    float depthThreshold;
};

inline static std::string shaderCode = R"(
// Input texture (source depth texture)
Texture2D<float> SourceDepthTexture : register(t0);

// Sampler for texture sampling (using linear interpolation)
SamplerState SamplerLinear : register(s0);

// Output texture (UAV for writing the upscaled depth values)
RWTexture2D<float> OutputDepthTexture : register(u0);

// Compute shader thread group size
[numthreads(16, 16, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    // Calculate texture coordinates (normalized UV)
    float2 uv = (threadID.xy * inverseResolution); // Map threadID to UV

    // Read the depth value 
    float depthCenter = SourceDepthTexture.SampleLevel(SamplerLinear, uv, 0);

    // Return 
    OutputDepthTexture[threadID.xy] = depthCenter;
}
)";

inline static ID3DBlob* DU_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        LOG_ERROR("error while compiling shader");

        if (errorBlob)
        {
            LOG_ERROR("error while compiling shader : {0}", (char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return nullptr;
    }

    if (errorBlob)
        errorBlob->Release();

    return shaderBlob;
}