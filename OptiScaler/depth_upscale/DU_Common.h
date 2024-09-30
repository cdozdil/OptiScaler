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
Texture2D<float> SourceTexture : register(t0);

// Output texture (UAV for writing the upscaled depth values)
RWTexture2D<float> DestinationTexture : register(u0);

// Compute shader thread group size
[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // The dispatchThreadID.xy are the pixel coordinates
    uint2 pixelCoord = dispatchThreadID.xy;

    // Load the pixel value from the source texture
    float4 srcColor = SourceTexture.Load(int3(pixelCoord, 0));

    // Write the pixel value to the destination texture
    DestinationTexture[pixelCoord] = srcColor;
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