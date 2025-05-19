#pragma once
#include <pch.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

struct alignas(256) RFConstants
{
    UINT width;
    UINT height;
};

inline static std::string rfCode = R"(
cbuffer Params : register(b0)
{
    uint width;
    uint height;
};

// Input texture
Texture2D<float3> SourceTexture : register(t0);

// Output texture
RWTexture2D<float3> DestinationTexture : register(u0);

// Compute shader thread group size
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixelCoord = uint2(dispatchThreadID.x, height - dispatchThreadID.y);
    float3 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));
    DestinationTexture[pixelCoord] = srcColor;
}
)";

inline static ID3DBlob* RF_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target,
                            D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        LOG_ERROR("error while compiling shader");

        if (errorBlob)
        {
            LOG_ERROR("error while compiling shader : {0}", (char*) errorBlob->GetBufferPointer());
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
