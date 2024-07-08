#pragma once

#include "../pch.h"
#include <d3dcompiler.h>

struct RcasConstants
{
    float Sharpness;

    // Motion Vector Stuff
    bool DynamicSharpenEnabled;
    bool DisplaySizeMV;
    bool Debug;

    float MotionSharpness;
    float MotionTextureScale;
    float MvScaleX;
    float MvScaleY;
    float Threshold;
    float ScaleLimit;

    int RenderWidth;
    int RenderHeight;
    int DisplayWidth;
    int DisplayHeight;
};

// Based on this Reshade shader
// https://github.com/RdenBlaauwen/RCAS-for-ReShade

static std::string rcasCode = R"(
cbuffer Params : register(b0)
{
    float Sharpness;

    // Motion Vector Stuff
    int DynamicSharpenEnabled;
    int DisplaySizeMV;
    int Debug;
    
    float MotionSharpness;
    float MotionTextureScale;
    float MvScaleX;
    float MvScaleY;
    float Threshold;
    float ScaleLimit;
    int DisplayWidth;
    int DisplayHeight;
};

Texture2D<float3> Source : register(t0);
Texture2D<float2> Motion : register(t1);
RWTexture2D<float3> Dest : register(u0);

float getRCASLuma(float3 rgb)
{  
    return dot(rgb, float3(0.598, 1.174, 0.228));
}

[numthreads(32, 32, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
  float setSharpness = Sharpness;
  
  if(DynamicSharpenEnabled > 0)
  {
    float2 mv;
    float motion;
    float add = 0.0f;

    if(DisplaySizeMV > 0)
      mv = Motion.Load(int3(DTid.x, DTid.y, 0)).rg;
    else
      mv = Motion.Load(int3(DTid.x * MotionTextureScale, DTid.y * MotionTextureScale, 0)).rg;

    motion = max(abs(mv.r * MvScaleX), abs(mv.g * MvScaleY));

    if(motion > Threshold)
        add = (motion / (ScaleLimit - Threshold)) * MotionSharpness;
    
    if((add > MotionSharpness && MotionSharpness > 0.0f) || (add < MotionSharpness && MotionSharpness < 0.0f))
        add = MotionSharpness;
    
    setSharpness += add;

    if(setSharpness > 1.0f)
      setSharpness = 1.0f;
    else if (setSharpness < 0.0f)
      setSharpness = 0.0f;
  }

  float3 e = Source.Load(int3(DTid.x, DTid.y, 0)).rgb;
  
  // skip sharpening if set value == 0
  if(setSharpness == 0.0f)
  {
    if(Debug > 0 && DynamicSharpenEnabled > 0 && Sharpness > 0)
      e.g *= 1 + (12.0f * Sharpness);

    Dest[DTid.xy] = e;
    return;
  }

  float3 b = Source.Load(int3(DTid.x, DTid.y - 1, 0)).rgb;
  float3 d = Source.Load(int3(DTid.x - 1, DTid.y, 0)).rgb;
  float3 f = Source.Load(int3(DTid.x + 1, DTid.y, 0)).rgb;
  float3 h = Source.Load(int3(DTid.x, DTid.y + 1, 0)).rgb;
  
  // Get lumas times 2. Should use luma weights that are twice as large as normal.
  float bL = getRCASLuma(b);
  float dL = getRCASLuma(d);
  float eL = getRCASLuma(e);
  float fL = getRCASLuma(f);
  float hL = getRCASLuma(h);
  
  // Min and max of ring.
  float3 minRGB = min(min(b, d), min(f, h));
  float3 maxRGB = max(max(b, d), max(f, h));
  
  // Immediate constants for peak range.
  float2 peakC = float2(1.0, -4.0);
  
  // Limiters, these need to use high precision reciprocal operations.
  // Decided to use standard rcp for now in hopes of optimizing it
  float3 hitMin = minRGB * rcp(4.0 * maxRGB);
  float3 hitMax = (peakC.xxx - maxRGB) * rcp(4.0 * minRGB + peakC.yyy);
  float3 lobeRGB = max(-hitMin, hitMax);
  float lobe = max(-0.1875, min(max(lobeRGB.r, max(lobeRGB.g, lobeRGB.b)), 0.0)) * setSharpness;
  
  // Resolve, which needs medium precision rcp approximation to avoid visible tonality changes.
  float rcpL = rcp(4.0 * lobe + 1.0);
  float3 output = ((b + d + f + h) * lobe + e) * rcpL;
  
  if(Debug > 0 && DynamicSharpenEnabled > 0)
  {
    if(Sharpness < setSharpness)
      output.r *= 1 + (12.0f * (setSharpness - Sharpness));
    else
      output.g *= 1 + (12.0f * (Sharpness - setSharpness));
  }
  
  Dest[DTid.xy] = output;
}
)";

static ID3DBlob* CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        spdlog::error("CompileShader error while compiling shader");

        if (errorBlob)
        {
            spdlog::error("CompileShader error while compiling shader : {0}", (char*)errorBlob->GetBufferPointer());
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