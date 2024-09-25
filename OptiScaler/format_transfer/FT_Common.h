#pragma once
#include "../pch.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

inline static std::string ftR10G10B10A2Code = R"(
Texture2D<float4> SourceTexture : register(t0); 
RWTexture2D<uint> DestinationTexture : register(u0); // R10G10B10A2_UNORM destination texture

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Read from the source texture
    float4 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));

    // Clamp color channels to [0, 1] range, as R10G10B10A2_UNORM is normalized
    srcColor = saturate(srcColor); // Ensures all values are between 0 and 1

    // Convert each channel to its corresponding bit range
    uint R = (uint)(srcColor.r * 1023.0f); // 10 bits for Red
    uint G = (uint)(srcColor.g * 1023.0f); // 10 bits for Green
    uint B = (uint)(srcColor.b * 1023.0f); // 10 bits for Blue
    uint A = (uint)(srcColor.a * 3.0f);    // 2 bits for Alpha

    // Pack the values into a single 32-bit unsigned int
    uint packedColor = R | G << 10 | B << 20 | A << 30;

    // Write the packed color to the destination texture
    DestinationTexture[dispatchThreadID.xy] = packedColor;
}
)";

inline static std::string ftR8G8B8A8Code = R"(
Texture2D<float4> SourceTexture : register(t0); 
RWTexture2D<uint> DestinationTexture : register(u0); // R8G8B8A8_UNORM destination texture

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Read from the source texture
    float4 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));

    // Clamp color channels to [0, 1] range, as R8G8B8A8_UNORM is normalized
    srcColor = saturate(srcColor); // Ensures all values are between 0 and 1

    // Convert each channel to its corresponding bit range
    uint R = (uint)(srcColor.r * 255.0f); // 8 bits for Red
    uint G = (uint)(srcColor.g * 255.0f); // 8 bits for Green
    uint B = (uint)(srcColor.b * 255.0f); // 8 bits for Blue
    uint A = (uint)(srcColor.a * 255.0f); // 8 bits for Alpha

    // Pack the values into a single 32-bit unsigned int
    uint packedColor = R | G << 8 | B << 16 | A << 24;

    // Write the packed color to the destination texture
    DestinationTexture[dispatchThreadID.xy] = packedColor;
}
)";

inline static std::string ftB8G8R8A8Code = R"(
Texture2D<float4> SourceTexture : register(t0); 
RWTexture2D<uint> DestinationTexture : register(u0); // R8G8B8A8_UNORM destination texture

// Shader to perform the conversion
[numthreads(512, 1, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Read from the source texture
    float4 srcColor = SourceTexture.Load(int3(dispatchThreadID.xy, 0));

    // Clamp color channels to [0, 1] range, as R8G8B8A8_UNORM is normalized
    srcColor = saturate(srcColor); // Ensures all values are between 0 and 1

    // Convert each channel to its corresponding bit range
    uint R = (uint)(srcColor.r * 255.0f); // 8 bits for Red
    uint G = (uint)(srcColor.g * 255.0f); // 8 bits for Green
    uint B = (uint)(srcColor.b * 255.0f); // 8 bits for Blue
    uint A = (uint)(srcColor.a * 255.0f); // 8 bits for Alpha

    // Pack the values into a single 32-bit unsigned int
    uint packedColor = B | R << 8 | G << 16 | A << 24;

    // Write the packed color to the destination texture
    DestinationTexture[dispatchThreadID.xy] = packedColor;
}
)";


inline static ID3DBlob* FT_CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
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