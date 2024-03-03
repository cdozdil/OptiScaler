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
#include "ffx_core.h"
#endif  // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#ifndef FFX_PREFER_WAVE64
#define FFX_PREFER_WAVE64
#endif  // #ifndef FFX_PREFER_WAVE64

#if defined(CAS_BIND_CB_CAS)
layout(set = 0, binding = CAS_BIND_CB_CAS, std140) uniform cbCAS_t
{
    FfxUInt32x4 const0;
    FfxUInt32x4 const1;
}
cbCAS;
#endif

FfxUInt32x4 Const0()
{
#if defined(CAS_BIND_CB_CAS)
    return cbCAS.const0;
#else
    return 0.f;
#endif
}

FfxUInt32x4 Const1()
{
#if defined(CAS_BIND_CB_CAS)
    return cbCAS.const1;
#else
    return 0.f;
#endif
}

layout(set = 0, binding = 1000) uniform sampler s_LinearClamp;

    // SRVs
    #if defined CAS_BIND_SRV_INPUT_COLOR
        layout(set = 0, binding = CAS_BIND_SRV_INPUT_COLOR) uniform texture2D r_input_color;
    #endif

    // UAV declarations
    #if defined CAS_BIND_UAV_OUTPUT_COLOR
        layout(set = 0, binding = CAS_BIND_UAV_OUTPUT_COLOR, rgba16) uniform image2D rw_output_color;
    #endif

#if FFX_HALF

    FfxFloat16x3 casLoadHalf(FFX_PARAMETER_IN FfxInt16x2 position)
    {
    #if defined(CAS_BIND_SRV_INPUT_COLOR) 
        return FfxFloat16x3(texelFetch(r_input_color, FfxInt32x2(position), 0).rgb);
    #else
        return FfxFloat16x3(0.f);
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
        return texelFetch(r_input_color, position, 0).rgb;
    #else
        return FfxFloat32x3(0.f);
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
        imageStore(rw_output_color, iPxPos, fColor);
    #endif
    }

#endif  // #if defined(FFX_GPU)
