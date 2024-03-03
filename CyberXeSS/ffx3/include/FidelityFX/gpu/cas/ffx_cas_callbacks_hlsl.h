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


#include "ffx_cas_resources.h"

#if defined(FFX_GPU)
#ifdef __hlsl_dx_compiler
#pragma dxc diagnostic push
#pragma dxc diagnostic ignored "-Wambig-lit-shift"
#endif //__hlsl_dx_compiler
#include "ffx_core.h"
#ifdef __hlsl_dx_compiler
#pragma dxc diagnostic pop
#endif //__hlsl_dx_compiler
#endif // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#ifndef FFX_PREFER_WAVE64
#define FFX_PREFER_WAVE64
#endif // #ifndef FFX_PREFER_WAVE64

#if defined(FFX_GPU)
#pragma warning(disable: 3205)  // conversion from larger type to smaller
#endif // #if defined(FFX_GPU)

#define DECLARE_SRV_REGISTER(regIndex)  t##regIndex
#define DECLARE_UAV_REGISTER(regIndex)  u##regIndex
#define DECLARE_CB_REGISTER(regIndex)   b##regIndex
#define FFX_CAS_DECLARE_SRV(regIndex)  register(DECLARE_SRV_REGISTER(regIndex))
#define FFX_CAS_DECLARE_UAV(regIndex)  register(DECLARE_UAV_REGISTER(regIndex))
#define FFX_CAS_DECLARE_CB(regIndex)   register(DECLARE_CB_REGISTER(regIndex))

#if defined(CAS_BIND_CB_CAS)
    cbuffer cbCAS : FFX_CAS_DECLARE_CB(CAS_BIND_CB_CAS)
    {
        FfxUInt32x4 const0;
        FfxUInt32x4 const1;
       #define FFX_CAS_CONSTANT_BUFFER_1_SIZE 8  // Number of 32-bit values. This must be kept in sync with the cbCAS size.
    };
#else
    #define const0 0
    #define const1 0
#endif

#if defined(FFX_GPU)
#define FFX_CAS_ROOTSIG_STRINGIFY(p) FFX_CAS_ROOTSIG_STR(p)
#define FFX_CAS_ROOTSIG_STR(p) #p
#define FFX_CAS_ROOTSIG [RootSignature( "DescriptorTable(UAV(u0, numDescriptors = " FFX_CAS_ROOTSIG_STRINGIFY(FFX_CAS_RESOURCE_IDENTIFIER_COUNT) ")), " \
                                    "DescriptorTable(SRV(t0, numDescriptors = " FFX_CAS_ROOTSIG_STRINGIFY(FFX_CAS_RESOURCE_IDENTIFIER_COUNT) ")), " \
                                    "RootConstants(num32BitConstants=" FFX_CAS_ROOTSIG_STRINGIFY(FFX_CAS_CONSTANT_BUFFER_1_SIZE) ", b0), " \
                                    "StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_LINEAR, " \
                                                      "addressU = TEXTURE_ADDRESS_CLAMP, " \
                                                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                                                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                                                      "comparisonFunc = COMPARISON_NEVER, " \
                                                      "borderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK)" )]

#if defined(FFX_CAS_EMBED_ROOTSIG)
#define FFX_CAS_EMBED_ROOTSIG_CONTENT FFX_CAS_ROOTSIG
#else
#define FFX_CAS_EMBED_ROOTSIG_CONTENT
#endif // #if FFX_CAS_EMBED_ROOTSIG
#endif // #if defined(FFX_GPU)


FfxUInt32x4 Const0()
{
#if defined(CAS_BIND_CB_CAS)
    return const0;
#else
    return 0.f;
#endif
}

FfxUInt32x4 Const1()
{
#if defined(CAS_BIND_CB_CAS)
    return const1;
#else
    return 0.f;
#endif
}

SamplerState s_LinearClamp : register(s0);

    // SRVs
    #if defined(CAS_BIND_SRV_INPUT_COLOR)
        Texture2D<FfxFloat32x4>                   r_input_color                 : FFX_CAS_DECLARE_SRV(CAS_BIND_SRV_INPUT_COLOR);
    #endif

    // UAV declarations
    #if defined(CAS_BIND_UAV_OUTPUT_COLOR)
        RWTexture2D<FfxFloat32x4>                 rw_output_color               : FFX_CAS_DECLARE_UAV(CAS_BIND_UAV_OUTPUT_COLOR);
    #endif

#if FFX_HALF

FfxFloat16x3 casLoadHalf(FFX_PARAMETER_IN FfxInt16x2 position)
{
#if defined(CAS_BIND_SRV_INPUT_COLOR) 
    return FfxFloat16x3(r_input_color.Load(FfxInt32x3(position, 0)).rgb);
#else
    return 0.f;
#endif
}

// Transform input from the load into a linear color space between 0 and 1.
void casInputHalf(FFX_PARAMETER_INOUT FfxFloat16x2 red, FFX_PARAMETER_INOUT FfxFloat16x2 green, FFX_PARAMETER_INOUT FfxFloat16x2 blue)
{
#if FFX_CAS_COLOR_SPACE_CONVERSION == 1    // gamma 2.0
    red   *= red;
    green *= green;
    blue  *= blue;
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 2  // gamma 2.2
    red   = ffxLinearFromGammaHalf(red, FfxFloat16(2.2f));
    green = ffxLinearFromGammaHalf(green, FfxFloat16(2.2f));
    blue  = ffxLinearFromGammaHalf(blue, FfxFloat16(2.2f));
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 3  // sRGB output (auto-degamma'd on sampler read)

#elif FFX_CAS_COLOR_SPACE_CONVERSION == 4  // sRGB input/output
    red   = ffxLinearFromSrgbHalf(red);
    green = ffxLinearFromSrgbHalf(green);
    blue  = ffxLinearFromSrgbHalf(blue);
#endif
}

void casOutputHalf(FFX_PARAMETER_INOUT FfxFloat16x2 red, FFX_PARAMETER_INOUT FfxFloat16x2 green, FFX_PARAMETER_INOUT FfxFloat16x2 blue)
{
#if FFX_CAS_COLOR_SPACE_CONVERSION == 1    // gamma 2.0
    red   = ffxSqrt(red);
    green = ffxSqrt(green);
    blue  = ffxSqrt(blue);
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 2  // gamma 2.2
    red   = ffxGammaFromLinearHalf(red, FfxFloat16(1/2.2f));
    green = ffxGammaFromLinearHalf(green, FfxFloat16(1/2.2f));
    blue  = ffxGammaFromLinearHalf(blue, FfxFloat16(1/2.2f));
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 3  // sRGB output (auto-degamma'd on sampler read)
    red   = ffxSrgbFromLinearHalf(red);
    green = ffxSrgbFromLinearHalf(green);
    blue  = ffxSrgbFromLinearHalf(blue);
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 4  // sRGB input/output
    red   = ffxSrgbFromLinearHalf(red);
    green = ffxSrgbFromLinearHalf(green);
    blue  = ffxSrgbFromLinearHalf(blue);
#endif
}

#else

FfxFloat32x3 casLoad(FFX_PARAMETER_IN FfxInt32x2 position)
{
#if defined(CAS_BIND_SRV_INPUT_COLOR) 
    return r_input_color.Load(FfxInt32x3(position, 0)).rgb;
#else
    return 0.f;
#endif
}

// Transform input from the load into a linear color space between 0 and 1.
void casInput(FFX_PARAMETER_INOUT FfxFloat32 red, FFX_PARAMETER_INOUT FfxFloat32 green, FFX_PARAMETER_INOUT FfxFloat32 blue)
{
#if FFX_CAS_COLOR_SPACE_CONVERSION == 1    // gamma 2.0
    red   *= red;
    green *= green;
    blue  *= blue;
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 2  // gamma 2.2
    red   = ffxLinearFromGamma(red, FfxFloat32(2.2f));
    green = ffxLinearFromGamma(green, FfxFloat32(2.2f));
    blue  = ffxLinearFromGamma(blue, FfxFloat32(2.2f));
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 3  // sRGB output (auto-degamma'd on sampler read)

#elif FFX_CAS_COLOR_SPACE_CONVERSION == 4  // sRGB input/output
    red   = ffxLinearFromSrgb(red);
    green = ffxLinearFromSrgb(green);
    blue  = ffxLinearFromSrgb(blue);
#endif
}

void casOutput(FFX_PARAMETER_INOUT FfxFloat32 red, FFX_PARAMETER_INOUT FfxFloat32 green, FFX_PARAMETER_INOUT FfxFloat32 blue)
{
#if FFX_CAS_COLOR_SPACE_CONVERSION == 1    // gamma 2.0
    red   = ffxSqrt(red);
    green = ffxSqrt(green);
    blue  = ffxSqrt(blue);
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 2  // gamma 2.2
    red   = ffxGammaFromLinear(red, FfxFloat32(1/2.2f));
    green = ffxGammaFromLinear(green, FfxFloat32(1/2.2f));
    blue  = ffxGammaFromLinear(blue, FfxFloat32(1/2.2f));
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 3  // sRGB output (auto-degamma'd on sampler read)
    red   = ffxSrgbToLinear(red);
    green = ffxSrgbToLinear(green);
    blue  = ffxSrgbToLinear(blue);
#elif FFX_CAS_COLOR_SPACE_CONVERSION == 4  // sRGB input/output
    red   = ffxSrgbToLinear(red);
    green = ffxSrgbToLinear(green);
    blue  = ffxSrgbToLinear(blue);
#endif
}

#endif  // FFX_HALF

void casStoreOutput(FfxInt32x2 iPxPos, FfxFloat32x4 fColor)
{
#if defined(CAS_BIND_UAV_OUTPUT_COLOR) 
    rw_output_color[iPxPos] = fColor;
#endif
}

#endif // #if defined(FFX_GPU)
