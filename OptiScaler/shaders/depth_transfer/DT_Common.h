#pragma once
#include <pch.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

inline static std::string shaderCode = R"(
Texture2D<float> SourceTexture : register(t0);
RWTexture2D<float> DestinationTexture : register(u0);

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));
    DestinationTexture[dispatchThreadID.xy] = 1.0f - srcColor;
})";

inline static ID3DBlob* DT_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
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
