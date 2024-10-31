#pragma once

#include <pch.h>

#include <d3dcompiler.h>

static std::string biasShader = R"(
cbuffer Params : register(b0)
{
    float Bias;
};

Texture2D<float3> Source : register(t0);
RWTexture2D<float3> Dest : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
  float3 src = Source.Load(int3(DTid.x, DTid.y, 0)).rgb;
  src.r *= Bias;
  Dest[DTid.xy] = src;
}
)";

static ID3DBlob* Bias_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
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