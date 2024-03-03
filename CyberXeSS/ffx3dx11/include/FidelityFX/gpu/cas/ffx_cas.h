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


/// @defgroup FfxGPUCas FidelityFX CAS
/// FidelityFX Contrast Adaptive Sharpening GPU documentation
/// 
/// @ingroup FfxGPUEffects

/// The maximum scaling ratio that CAS can support.
///
/// @ingroup FfxGPUCas
#define FFX_CAS_AREA_LIMIT      (4.0)

/// A function to check if the scaling ratio is supported by CAS.
/// 
/// Contrast Adaptive Sharpening (CAS) supports a maximum scaling ratio expressed in <c><i>FFX_CAS_AREA_LIMIT</i></c>.
/// 
/// @param [in] outX                The width of the target output, expressed in pixels.
/// @param [in] outY                The height of the target output, expressed in pixels.
/// @param [in] inX                 The width of the input surface, expressed in pixels.
/// @param [in] inY                 The height of the input surface, expressed in pixels.
///
/// @returns
/// True if CAS supports scaling in the given configuration.
/// 
/// @ingroup FfxGPUCas
FfxUInt32 ffxCasSupportScaling(
    FFX_PARAMETER_IN FfxFloat32 outX,
    FFX_PARAMETER_IN FfxFloat32 outY,
    FFX_PARAMETER_IN FfxFloat32 inX,
    FFX_PARAMETER_IN FfxFloat32 inY)
{
    return FfxUInt32(((outX * outY) * ffxReciprocal(inX * inY)) <= FFX_CAS_AREA_LIMIT);
}

/// Call to setup required constant values (works on CPU or GPU).
///
/// @param [out] const0                 The first 4 32-bit values of the constant buffer which is populated by this function.
/// @param [out] const1                 The second 4 32-bit values of the constant buffer which is populated by this function.
/// @param [in] sharpness               Set to 0 for the default (lower ringing), 1 for maximum (higest ringing).
/// @param [in] inputSizeInPixelsX      The size of the input resolution in the X dimension.
/// @param [in] inputSizeInPixelsY      The size of the input resolution in the Y dimension.
/// @param [in] outputSizeInPixelsX     The size of the output resolution in the X dimension.
/// @param [in] outputSizeInPixelsY     The size of the output resolution in the Y dimension.
/// 
/// @ingroup FfxGPUCas
FFX_STATIC void ffxCasSetup(
    FFX_PARAMETER_INOUT FfxUInt32x4 const0,
    FFX_PARAMETER_INOUT FfxUInt32x4 const1,
    FFX_PARAMETER_IN FfxFloat32 sharpness,
    FFX_PARAMETER_IN FfxFloat32 inputSizeInPixelsX,
    FFX_PARAMETER_IN FfxFloat32 inputSizeInPixelsY,
    FFX_PARAMETER_IN FfxFloat32 outputSizeInPixelsX,
    FFX_PARAMETER_IN FfxFloat32 outputSizeInPixelsY)
{
    // Scaling terms.
    const0[0] = ffxAsUInt32(inputSizeInPixelsX * ffxReciprocal(outputSizeInPixelsX));
    const0[1] = ffxAsUInt32(inputSizeInPixelsY * ffxReciprocal(outputSizeInPixelsY));
    const0[2] = ffxAsUInt32(FfxFloat32(0.5) * inputSizeInPixelsX * ffxReciprocal(outputSizeInPixelsX) - FfxFloat32(0.5));
    const0[3] = ffxAsUInt32(FfxFloat32(0.5) * inputSizeInPixelsY * ffxReciprocal(outputSizeInPixelsY) - FfxFloat32(0.5));

    // Sharpness value.
    FfxFloat32   sharp  = -ffxReciprocal(ffxLerp(8.0, 5.0, ffxSaturate(sharpness)));
    FfxFloat32x2 hSharp = {sharp, 0.0};
    const1[0] = ffxAsUInt32(sharp);
    const1[1] = packHalf2x16(hSharp);
    const1[2] = ffxAsUInt32(FfxFloat32(8.0) * inputSizeInPixelsX * ffxReciprocal(outputSizeInPixelsX));
    const1[3] = 0;
}

#if defined(FFX_GPU)
#if defined(FFX_CAS_PACKED_ONLY)
// Avoid compiler errors by including default implementations of these callbacks.
FfxFloat32x3 casLoad(FFX_PARAMETER_IN FfxInt32x2 position)
{
    return FfxFloat32x3(0.0, 0.0, 0.0);
}

void casInput(
    FFX_PARAMETER_INOUT FfxFloat32 red,
    FFX_PARAMETER_INOUT FfxFloat32 green,
    FFX_PARAMETER_INOUT FfxFloat32 blue)
{
}
#endif // #if defined(FFX_CAS_PACKED_ONLY)

// No scaling algorithm uses minimal 3x3 pixel neighborhood.
void casFilterNoScaling(
    FFX_PARAMETER_OUT FfxFloat32 outPixelRed,
    FFX_PARAMETER_OUT FfxFloat32 outPixelGreen,
    FFX_PARAMETER_OUT FfxFloat32 outPixelBlue,
    FFX_PARAMETER_IN FfxUInt32x2 samplePosition,
    FFX_PARAMETER_IN FfxUInt32x4 const0,
    FFX_PARAMETER_IN FfxUInt32x4 const1)
{
    // Load a collection of samples in a 3x3 neighorhood, where e is the current pixel.
    // a b c
    // d e f
    // g h i
    FfxFloat32x3 sampleA = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(-1, -1));
    FfxFloat32x3 sampleB = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(0, -1));
    FfxFloat32x3 sampleC = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(1, -1));
    FfxFloat32x3 sampleD = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(-1, 0));
    FfxFloat32x3 sampleE = casLoad(FfxInt32x2(samplePosition));
    FfxFloat32x3 sampleF = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(1, 0));
    FfxFloat32x3 sampleG = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(-1, 1));
    FfxFloat32x3 sampleH = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(0, 1));
    FfxFloat32x3 sampleI = casLoad(FfxInt32x2(samplePosition) + FfxInt32x2(1, 1));

    // Run optional input transform.
    casInput(sampleA.r, sampleA.g, sampleA.b);
    casInput(sampleB.r, sampleB.g, sampleB.b);
    casInput(sampleC.r, sampleC.g, sampleC.b);
    casInput(sampleD.r, sampleD.g, sampleD.b);
    casInput(sampleE.r, sampleE.g, sampleE.b);
    casInput(sampleF.r, sampleF.g, sampleF.b);
    casInput(sampleG.r, sampleG.g, sampleG.b);
    casInput(sampleH.r, sampleH.g, sampleH.b);
    casInput(sampleI.r, sampleI.g, sampleI.b);

    // Soft min and max.
    //  a b c             b
    //  d e f * 0.5  +  d e f * 0.5
    //  g h i             h
    // These are 2.0x bigger (factored out the extra multiply).
    FfxFloat32 minimumRed   = ffxMin3(ffxMin3(sampleD.r, sampleE.r, sampleF.r), sampleB.r, sampleH.r);
    FfxFloat32 minimumGreen = ffxMin3(ffxMin3(sampleD.g, sampleE.g, sampleF.g), sampleB.g, sampleH.g);
    FfxFloat32 minimumBlue  = ffxMin3(ffxMin3(sampleD.b, sampleE.b, sampleF.b), sampleB.b, sampleH.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 minimumRed2   = ffxMin3(ffxMin3(minimumRed, sampleA.r, sampleC.r), sampleG.r, sampleI.r);
    FfxFloat32 minimumGreen2 = ffxMin3(ffxMin3(minimumGreen, sampleA.g, sampleC.g), sampleG.g, sampleI.g);
    FfxFloat32 minimumBlue2  = ffxMin3(ffxMin3(minimumBlue, sampleA.b, sampleC.b), sampleG.b, sampleI.b);
    minimumRed               = minimumRed + minimumRed2;
    minimumGreen             = minimumGreen + minimumGreen2;
    minimumBlue              = minimumBlue + minimumBlue2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat32 maximumRed   = ffxMax3(ffxMax3(sampleD.r, sampleE.r, sampleF.r), sampleB.r, sampleH.r);
    FfxFloat32 maximumGreen = ffxMax3(ffxMax3(sampleD.g, sampleE.g, sampleF.g), sampleB.g, sampleH.g);
    FfxFloat32 maximumBlue  = ffxMax3(ffxMax3(sampleD.b, sampleE.b, sampleF.b), sampleB.b, sampleH.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 maximumRed2   = ffxMax3(ffxMax3(maximumRed, sampleA.r, sampleC.r), sampleG.r, sampleI.r);
    FfxFloat32 maximumGreen2 = ffxMax3(ffxMax3(maximumGreen, sampleA.g, sampleC.g), sampleG.g, sampleI.g);
    FfxFloat32 maximumBlue2  = ffxMax3(ffxMax3(maximumBlue, sampleA.b, sampleC.b), sampleG.b, sampleI.b);
    maximumRed               = maximumRed + maximumRed2;
    maximumGreen             = maximumGreen + maximumGreen2;
    maximumBlue              = maximumBlue + maximumBlue2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    // Smooth minimum distance to signal limit divided by smooth max.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat32 reciprocalMaximumRed   = ffxReciprocal(maximumRed);
    FfxFloat32 reciprocalMaximumGreen = ffxReciprocal(maximumGreen);
    FfxFloat32 reciprocalMaximumBlue  = ffxReciprocal(maximumBlue);
#else
    FfxFloat32 reciprocalMaximumRed   = ffxApproximateReciprocal(maximumRed);
    FfxFloat32 reciprocalMaximumGreen = ffxApproximateReciprocal(maximumGreen);
    FfxFloat32 reciprocalMaximumBlue  = ffxApproximateReciprocal(maximumBlue);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat32 amplifyRed   = ffxSaturate(ffxMin(minimumRed, FfxFloat32(2.0) - maximumRed) * reciprocalMaximumRed);
    FfxFloat32 amplifyGreen = ffxSaturate(ffxMin(minimumGreen, FfxFloat32(2.0) - maximumGreen) * reciprocalMaximumGreen);
    FfxFloat32 amplifyBlue  = ffxSaturate(ffxMin(minimumBlue, FfxFloat32(2.0) - maximumBlue) * reciprocalMaximumBlue);
#else
    FfxFloat32 amplifyRed   = ffxSaturate(ffxMin(minimumRed, FfxFloat32(1.0) - maximumRed) * reciprocalMaximumRed);
    FfxFloat32 amplifyGreen = ffxSaturate(ffxMin(minimumGreen, FfxFloat32(1.0) - maximumGreen) * reciprocalMaximumGreen);
    FfxFloat32 amplifyBlue  = ffxSaturate(ffxMin(minimumBlue, FfxFloat32(1.0) - maximumBlue) * reciprocalMaximumBlue);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Shaping amount of sharpening.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    amplifyRed   = ffxSqrt(amplifyRed);
    amplifyGreen = ffxSqrt(amplifyGreen);
    amplifyBlue  = ffxSqrt(amplifyBlue);
#else
    amplifyRed   = ffxApproximateSqrt(amplifyRed);
    amplifyGreen = ffxApproximateSqrt(amplifyGreen);
    amplifyBlue  = ffxApproximateSqrt(amplifyBlue);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Filter shape.
    //  0 w 0
    //  w 1 w
    //  0 w 0
    FfxFloat32 peak = ffxAsFloat(const1.x);
    FfxFloat32x3 weight = FfxFloat32x3(amplifyRed * peak, amplifyGreen * peak, amplifyBlue * peak);

    // Filter using green coef only, depending on dead code removal to strip out the extra overhead.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat32 reciprocalWeight = ffxReciprocal(FfxFloat32(1.0) + FfxFloat32(4.0) * weight.g);
#else
    FfxFloat32 reciprocalWeight = ffxApproximateReciprocalMedium(FfxFloat32(1.0) + FfxFloat32(4.0) * weight.g);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    outPixelRed   = ffxSaturate((sampleB.r * weight.g + sampleD.r * weight.g + sampleF.r * weight.g + sampleH.r * weight.g + sampleE.r) * reciprocalWeight);
    outPixelGreen = ffxSaturate((sampleB.g * weight.g + sampleD.g * weight.g + sampleF.g * weight.g + sampleH.g * weight.g + sampleE.g) * reciprocalWeight);
    outPixelBlue  = ffxSaturate((sampleB.b * weight.g + sampleD.b * weight.g + sampleF.b * weight.g + sampleH.b * weight.g + sampleE.b) * reciprocalWeight);
}

#if FFX_HALF == 1
// Half precision version algorithm with no scaling and filters 2 tiles in one run.
void casFilterNoScalingHalf(
    FFX_PARAMETER_OUT FfxFloat16x2 outPixelRed,
    FFX_PARAMETER_OUT FfxFloat16x2 outPixelGreen,
    FFX_PARAMETER_OUT FfxFloat16x2 outPixelBlue,
    FFX_PARAMETER_IN FfxUInt32x2 samplePosition,
    FFX_PARAMETER_IN FfxUInt32x4 const0,
    FFX_PARAMETER_IN FfxUInt32x4 const1)
{
    FfxInt16x2   samplePosition0 = FfxInt16x2(samplePosition);
    FfxFloat16x3 sampleA0 = casLoadHalf(samplePosition0 + FfxInt16x2(-1, -1));
    FfxFloat16x3 sampleB0 = casLoadHalf(samplePosition0 + FfxInt16x2(0, -1));
    FfxFloat16x3 sampleC0 = casLoadHalf(samplePosition0 + FfxInt16x2(1, -1));
    FfxFloat16x3 sampleD0 = casLoadHalf(samplePosition0 + FfxInt16x2(-1, 0));
    FfxFloat16x3 sampleE0 = casLoadHalf(samplePosition0);
    FfxFloat16x3 sampleF0 = casLoadHalf(samplePosition0 + FfxInt16x2(1, 0));
    FfxFloat16x3 sampleG0 = casLoadHalf(samplePosition0 + FfxInt16x2(-1, 1));
    FfxFloat16x3 sampleH0 = casLoadHalf(samplePosition0 + FfxInt16x2(0, 1));
    FfxFloat16x3 sampleI0 = casLoadHalf(samplePosition0 + FfxInt16x2(1, 1));
    FfxInt16x2   samplePosition1 = samplePosition0 + FfxInt16x2(8, 0);
    FfxFloat16x3 sampleA1  = casLoadHalf(samplePosition1 + FfxInt16x2(-1, -1));
    FfxFloat16x3 sampleB1  = casLoadHalf(samplePosition1 + FfxInt16x2(0, -1));
    FfxFloat16x3 sampleC1  = casLoadHalf(samplePosition1 + FfxInt16x2(1, -1));
    FfxFloat16x3 sampleD1  = casLoadHalf(samplePosition1 + FfxInt16x2(-1, 0));
    FfxFloat16x3 sampleE1  = casLoadHalf(samplePosition1);
    FfxFloat16x3 sampleF1  = casLoadHalf(samplePosition1 + FfxInt16x2(1, 0));
    FfxFloat16x3 sampleG1  = casLoadHalf(samplePosition1 + FfxInt16x2(-1, 1));
    FfxFloat16x3 sampleH1  = casLoadHalf(samplePosition1 + FfxInt16x2(0, 1));
    FfxFloat16x3 sampleI1  = casLoadHalf(samplePosition1 + FfxInt16x2(1, 1));

    // AOS to SOA conversion.
    FfxFloat16x2 aR = FfxFloat16x2(sampleA0.r, sampleA1.r);
    FfxFloat16x2 aG = FfxFloat16x2(sampleA0.g, sampleA1.g);
    FfxFloat16x2 aB = FfxFloat16x2(sampleA0.b, sampleA1.b);
    FfxFloat16x2 bR = FfxFloat16x2(sampleB0.r, sampleB1.r);
    FfxFloat16x2 bG = FfxFloat16x2(sampleB0.g, sampleB1.g);
    FfxFloat16x2 bB = FfxFloat16x2(sampleB0.b, sampleB1.b);
    FfxFloat16x2 cR = FfxFloat16x2(sampleC0.r, sampleC1.r);
    FfxFloat16x2 cG = FfxFloat16x2(sampleC0.g, sampleC1.g);
    FfxFloat16x2 cB = FfxFloat16x2(sampleC0.b, sampleC1.b);
    FfxFloat16x2 dR = FfxFloat16x2(sampleD0.r, sampleD1.r);
    FfxFloat16x2 dG = FfxFloat16x2(sampleD0.g, sampleD1.g);
    FfxFloat16x2 dB = FfxFloat16x2(sampleD0.b, sampleD1.b);
    FfxFloat16x2 eR = FfxFloat16x2(sampleE0.r, sampleE1.r);
    FfxFloat16x2 eG = FfxFloat16x2(sampleE0.g, sampleE1.g);
    FfxFloat16x2 eB = FfxFloat16x2(sampleE0.b, sampleE1.b);
    FfxFloat16x2 fR = FfxFloat16x2(sampleF0.r, sampleF1.r);
    FfxFloat16x2 fG = FfxFloat16x2(sampleF0.g, sampleF1.g);
    FfxFloat16x2 fB = FfxFloat16x2(sampleF0.b, sampleF1.b);
    FfxFloat16x2 gR = FfxFloat16x2(sampleG0.r, sampleG1.r);
    FfxFloat16x2 gG = FfxFloat16x2(sampleG0.g, sampleG1.g);
    FfxFloat16x2 gB = FfxFloat16x2(sampleG0.b, sampleG1.b);
    FfxFloat16x2 hR = FfxFloat16x2(sampleH0.r, sampleH1.r);
    FfxFloat16x2 hG = FfxFloat16x2(sampleH0.g, sampleH1.g);
    FfxFloat16x2 hB = FfxFloat16x2(sampleH0.b, sampleH1.b);
    FfxFloat16x2 iR = FfxFloat16x2(sampleI0.r, sampleI1.r);
    FfxFloat16x2 iG = FfxFloat16x2(sampleI0.g, sampleI1.g);
    FfxFloat16x2 iB = FfxFloat16x2(sampleI0.b, sampleI1.b);

    // Run optional input transform.
    casInputHalf(aR, aG, aB);
    casInputHalf(bR, bG, bB);
    casInputHalf(cR, cG, cB);
    casInputHalf(dR, dG, dB);
    casInputHalf(eR, eG, eB);
    casInputHalf(fR, fG, fB);
    casInputHalf(gR, gG, gB);
    casInputHalf(hR, hG, hB);
    casInputHalf(iR, iG, iB);

    // Soft min and max.
    FfxFloat16x2 minimumRed   = ffxMin(ffxMin(fR, hR), ffxMin(ffxMin(bR, dR), eR));
    FfxFloat16x2 minimumGreen = ffxMin(ffxMin(fG, hG), ffxMin(ffxMin(bG, dG), eG));
    FfxFloat16x2 minimumBlue  = ffxMin(ffxMin(fB, hB), ffxMin(ffxMin(bB, dB), eB));

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat16x2 minimumRed2   = ffxMin(ffxMin(gR, iR), ffxMin(ffxMin(aR, cR), minimumRed));
    FfxFloat16x2 minimumGreen2 = ffxMin(ffxMin(gG, iG), ffxMin(ffxMin(aG, cG), minimumGreen));
    FfxFloat16x2 minimumBlue2  = ffxMin(ffxMin(gB, iB), ffxMin(ffxMin(aB, cB), minimumBlue));
    minimumRed                 = minimumRed + minimumRed2;
    minimumGreen               = minimumGreen + minimumGreen2;
    minimumBlue                = minimumBlue + minimumBlue2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat16x2 maximumRed   = max(max(fR, hR), max(max(bR, dR), eR));
    FfxFloat16x2 maximumGreen = max(max(fG, hG), max(max(bG, dG), eG));
    FfxFloat16x2 maximumBlue  = max(max(fB, hB), max(max(bB, dB), eB));

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat16x2 maximumRed2   = max(max(gR, iR), max(max(aR, cR), maximumRed));
    FfxFloat16x2 maximumGreen2 = max(max(gG, iG), max(max(aG, cG), maximumGreen));
    FfxFloat16x2 maximumBlue2  = max(max(gB, iB), max(max(aB, cB), maximumBlue));
    maximumRed                 = maximumRed + maximumRed2;
    maximumGreen               = maximumGreen + maximumGreen2;
    maximumBlue                = maximumBlue + maximumBlue2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    // Smooth minimum distance to signal limit divided by smooth max.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat16x2 reciprocalMaximumRed   = ffxReciprocalHalf(maximumRed);
    FfxFloat16x2 reciprocalMaximumGreen = ffxReciprocalHalf(maximumGreen);
    FfxFloat16x2 reciprocalMaximumBlue  = ffxReciprocalHalf(maximumBlue);
#else
    FfxFloat16x2 reciprocalMaximumRed   = ffxApproximateReciprocalHalf(maximumRed);
    FfxFloat16x2 reciprocalMaximumGreen = ffxApproximateReciprocalHalf(maximumGreen);
    FfxFloat16x2 reciprocalMaximumBlue  = ffxApproximateReciprocalHalf(maximumBlue);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat16x2 amplifyRed   = ffxSaturate(min(minimumRed, FFX_BROADCAST_FLOAT16X2(2.0) - maximumRed) * reciprocalMaximumRed);
    FfxFloat16x2 amplifyGreen = ffxSaturate(min(minimumGreen, FFX_BROADCAST_FLOAT16X2(2.0) - maximumGreen) * reciprocalMaximumGreen);
    FfxFloat16x2 amplifyBlue  = ffxSaturate(min(minimumBlue, FFX_BROADCAST_FLOAT16X2(2.0) - maximumBlue) * reciprocalMaximumBlue);
#else
    FfxFloat16x2 amplifyRed   = FfxFloat16x2(ffxSaturate(min(minimumRed, FFX_BROADCAST_FLOAT16X2(1.0) - maximumRed) * reciprocalMaximumRed));
    FfxFloat16x2 amplifyGreen = FfxFloat16x2(ffxSaturate(min(minimumGreen, FFX_BROADCAST_FLOAT16X2(1.0) - maximumGreen) * reciprocalMaximumGreen));
    FfxFloat16x2 amplifyBlue  = FfxFloat16x2(ffxSaturate(min(minimumBlue, FFX_BROADCAST_FLOAT16X2(1.0) - maximumBlue) * reciprocalMaximumBlue));
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    // Shaping amount of sharpening.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    amplifyRed   = ffxSqrt(amplifyRed);
    amplifyGreen = ffxSqrt(amplifyGreen);
    amplifyBlue  = ffxSqrt(amplifyBlue);
#else
    amplifyRed   = ffxApproximateSqrtHalf(amplifyRed);
    amplifyGreen = ffxApproximateSqrtHalf(amplifyGreen);
    amplifyBlue  = ffxApproximateSqrtHalf(amplifyBlue);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Filter shape.
    FfxFloat16   peak        = FFX_UINT32_TO_FLOAT16X2(const1.y).x;
    FfxFloat16x2 weightRed   = amplifyRed * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 weightGreen = amplifyGreen * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 weightBlue  = amplifyBlue * FFX_BROADCAST_FLOAT16X2(peak);
    // Filter.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat16x2 reciprocalWeight = ffxReciprocalHalf(FFX_BROADCAST_FLOAT16X2(1.0) + FFX_BROADCAST_FLOAT16X2(4.0) * weightGreen);
#else
    FfxFloat16x2 reciprocalWeight = ffxApproximateReciprocalMediumHalf(FFX_BROADCAST_FLOAT16X2(1.0) + FFX_BROADCAST_FLOAT16X2(4.0) * weightGreen);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    outPixelRed   = FfxFloat16x2(ffxSaturate((bR * weightGreen + dR * weightGreen + fR * weightGreen + hR * weightGreen + eR) * reciprocalWeight));
    outPixelGreen = FfxFloat16x2(ffxSaturate((bG * weightGreen + dG * weightGreen + fG * weightGreen + hG * weightGreen + eG) * reciprocalWeight));
    outPixelBlue  = FfxFloat16x2(ffxSaturate((bB * weightGreen + dB * weightGreen + fB * weightGreen + hB * weightGreen + eB) * reciprocalWeight));
}
#endif // #if FFX_HALF == 1

// Scaling algorithm adaptively interpolates between nearest 4 results of the non-scaling algorithm.
void casFilterWithScaling(
    FFX_PARAMETER_OUT FfxFloat32 pixR,
    FFX_PARAMETER_OUT FfxFloat32 pixG,
    FFX_PARAMETER_OUT FfxFloat32 pixB,
    FFX_PARAMETER_IN FfxUInt32x2 samplePosition,
    FFX_PARAMETER_IN FfxUInt32x4 const0,
    FFX_PARAMETER_IN FfxUInt32x4 const1)
{
    //  a b c d
    //  e f g h
    //  i j k l
    //  m n o p
    // Working these 4 results.
    //  +-----+-----+
    //  |     |     |
    //  |  f..|..g  |
    //  |  .  |  .  |
    //  +-----+-----+
    //  |  .  |  .  |
    //  |  j..|..k  |
    //  |     |     |
    //  +-----+-----+
    FfxFloat32x2 pixelPosition = FfxFloat32x2(samplePosition) * ffxAsFloat(const0.xy) + ffxAsFloat(const0.zw);
    FfxFloat32x2 floorPixelPosition = floor(pixelPosition);
    pixelPosition -= floorPixelPosition;
    FfxInt32x2   finalSamplePosition = FfxInt32x2(floorPixelPosition);
    FfxFloat32x3 a  = casLoad(finalSamplePosition + FfxInt32x2(-1, -1));
    FfxFloat32x3 b  = casLoad(finalSamplePosition + FfxInt32x2(0, -1));
    FfxFloat32x3 e  = casLoad(finalSamplePosition + FfxInt32x2(-1, 0));
    FfxFloat32x3 f  = casLoad(finalSamplePosition);
    FfxFloat32x3 c  = casLoad(finalSamplePosition + FfxInt32x2(1, -1));
    FfxFloat32x3 d  = casLoad(finalSamplePosition + FfxInt32x2(2, -1));
    FfxFloat32x3 g  = casLoad(finalSamplePosition + FfxInt32x2(1, 0));
    FfxFloat32x3 h  = casLoad(finalSamplePosition + FfxInt32x2(2, 0));
    FfxFloat32x3 i  = casLoad(finalSamplePosition + FfxInt32x2(-1, 1));
    FfxFloat32x3 j  = casLoad(finalSamplePosition + FfxInt32x2(0, 1));
    FfxFloat32x3 m  = casLoad(finalSamplePosition + FfxInt32x2(-1, 2));
    FfxFloat32x3 n  = casLoad(finalSamplePosition + FfxInt32x2(0, 2));
    FfxFloat32x3 k  = casLoad(finalSamplePosition + FfxInt32x2(1, 1));
    FfxFloat32x3 l  = casLoad(finalSamplePosition + FfxInt32x2(2, 1));
    FfxFloat32x3 o  = casLoad(finalSamplePosition + FfxInt32x2(1, 2));
    FfxFloat32x3 p  = casLoad(finalSamplePosition + FfxInt32x2(2, 2));

    // Run optional input transform.
    casInput(a.r, a.g, a.b);
    casInput(b.r, b.g, b.b);
    casInput(c.r, c.g, c.b);
    casInput(d.r, d.g, d.b);
    casInput(e.r, e.g, e.b);
    casInput(f.r, f.g, f.b);
    casInput(g.r, g.g, g.b);
    casInput(h.r, h.g, h.b);
    casInput(i.r, i.g, i.b);
    casInput(j.r, j.g, j.b);
    casInput(k.r, k.g, k.b);
    casInput(l.r, l.g, l.b);
    casInput(m.r, m.g, m.b);
    casInput(n.r, n.g, n.b);
    casInput(o.r, o.g, o.b);
    casInput(p.r, p.g, p.b);

    // Soft min and max.
    // These are 2.0x bigger (factored out the extra multiply).
    //  a b c             b
    //  e f g * 0.5  +  e f g * 0.5  [F]
    //  i j k             j
    FfxFloat32 minimumRed = ffxMin3(ffxMin3(b.r, e.r, f.r), g.r, j.r);
    FfxFloat32 minimumGreen = ffxMin3(ffxMin3(b.g, e.g, f.g), g.g, j.g);
    FfxFloat32 minimumBlue = ffxMin3(ffxMin3(b.b, e.b, f.b), g.b, j.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mnfR2 = ffxMin3(ffxMin3(minimumRed, a.r, c.r), i.r, k.r);
    FfxFloat32 mnfG2 = ffxMin3(ffxMin3(minimumGreen, a.g, c.g), i.g, k.g);
    FfxFloat32 mnfB2 = ffxMin3(ffxMin3(minimumBlue, a.b, c.b), i.b, k.b);
    minimumRed       = minimumRed + mnfR2;
    minimumGreen     = minimumGreen + mnfG2;
    minimumBlue      = minimumBlue + mnfB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat32 mxfR = ffxMax3(ffxMax3(b.r, e.r, f.r), g.r, j.r);
    FfxFloat32 mxfG = ffxMax3(ffxMax3(b.g, e.g, f.g), g.g, j.g);
    FfxFloat32 mxfB = ffxMax3(ffxMax3(b.b, e.b, f.b), g.b, j.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mxfR2 = ffxMax3(ffxMax3(mxfR, a.r, c.r), i.r, k.r);
    FfxFloat32 mxfG2 = ffxMax3(ffxMax3(mxfG, a.g, c.g), i.g, k.g);
    FfxFloat32 mxfB2 = ffxMax3(ffxMax3(mxfB, a.b, c.b), i.b, k.b);
    mxfR             = mxfR + mxfR2;
    mxfG             = mxfG + mxfG2;
    mxfB             = mxfB + mxfB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    //  b c d             c
    //  f g h * 0.5  +  f g h * 0.5  [G]
    //  j k l             k
    FfxFloat32 mngR = ffxMin3(ffxMin3(c.r, f.r, g.r), h.r, k.r);
    FfxFloat32 mngG = ffxMin3(ffxMin3(c.g, f.g, g.g), h.g, k.g);
    FfxFloat32 mngB = ffxMin3(ffxMin3(c.b, f.b, g.b), h.b, k.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mngR2 = ffxMin3(ffxMin3(mngR, b.r, d.r), j.r, l.r);
    FfxFloat32 mngG2 = ffxMin3(ffxMin3(mngG, b.g, d.g), j.g, l.g);
    FfxFloat32 mngB2 = ffxMin3(ffxMin3(mngB, b.b, d.b), j.b, l.b);
    mngR             = mngR + mngR2;
    mngG             = mngG + mngG2;
    mngB             = mngB + mngB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat32 mxgR = ffxMax3(ffxMax3(c.r, f.r, g.r), h.r, k.r);
    FfxFloat32 mxgG = ffxMax3(ffxMax3(c.g, f.g, g.g), h.g, k.g);
    FfxFloat32 mxgB = ffxMax3(ffxMax3(c.b, f.b, g.b), h.b, k.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mxgR2 = ffxMax3(ffxMax3(mxgR, b.r, d.r), j.r, l.r);
    FfxFloat32 mxgG2 = ffxMax3(ffxMax3(mxgG, b.g, d.g), j.g, l.g);
    FfxFloat32 mxgB2 = ffxMax3(ffxMax3(mxgB, b.b, d.b), j.b, l.b);
    mxgR             = mxgR + mxgR2;
    mxgG             = mxgG + mxgG2;
    mxgB             = mxgB + mxgB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    //  e f g             f
    //  i j k * 0.5  +  i j k * 0.5  [J]
    //  m n o             n
    FfxFloat32 mnjR = ffxMin3(ffxMin3(f.r, i.r, j.r), k.r, n.r);
    FfxFloat32 mnjG = ffxMin3(ffxMin3(f.g, i.g, j.g), k.g, n.g);
    FfxFloat32 mnjB = ffxMin3(ffxMin3(f.b, i.b, j.b), k.b, n.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mnjR2 = ffxMin3(ffxMin3(mnjR, e.r, g.r), m.r, o.r);
    FfxFloat32 mnjG2 = ffxMin3(ffxMin3(mnjG, e.g, g.g), m.g, o.g);
    FfxFloat32 mnjB2 = ffxMin3(ffxMin3(mnjB, e.b, g.b), m.b, o.b);
    mnjR             = mnjR + mnjR2;
    mnjG             = mnjG + mnjG2;
    mnjB             = mnjB + mnjB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat32 mxjR = ffxMax3(ffxMax3(f.r, i.r, j.r), k.r, n.r);
    FfxFloat32 mxjG = ffxMax3(ffxMax3(f.g, i.g, j.g), k.g, n.g);
    FfxFloat32 mxjB = ffxMax3(ffxMax3(f.b, i.b, j.b), k.b, n.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mxjR2 = ffxMax3(ffxMax3(mxjR, e.r, g.r), m.r, o.r);
    FfxFloat32 mxjG2 = ffxMax3(ffxMax3(mxjG, e.g, g.g), m.g, o.g);
    FfxFloat32 mxjB2 = ffxMax3(ffxMax3(mxjB, e.b, g.b), m.b, o.b);
    mxjR             = mxjR + mxjR2;
    mxjG             = mxjG + mxjG2;
    mxjB             = mxjB + mxjB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    //  f g h             g
    //  j k l * 0.5  +  j k l * 0.5  [K]
    //  n o p             o
    FfxFloat32 mnkR = ffxMin3(ffxMin3(g.r, j.r, k.r), l.r, o.r);
    FfxFloat32 mnkG = ffxMin3(ffxMin3(g.g, j.g, k.g), l.g, o.g);
    FfxFloat32 mnkB = ffxMin3(ffxMin3(g.b, j.b, k.b), l.b, o.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mnkR2 = ffxMin3(ffxMin3(mnkR, f.r, h.r), n.r, p.r);
    FfxFloat32 mnkG2 = ffxMin3(ffxMin3(mnkG, f.g, h.g), n.g, p.g);
    FfxFloat32 mnkB2 = ffxMin3(ffxMin3(mnkB, f.b, h.b), n.b, p.b);
    mnkR             = mnkR + mnkR2;
    mnkG             = mnkG + mnkG2;
    mnkB             = mnkB + mnkB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

    FfxFloat32 mxkR = ffxMax3(ffxMax3(g.r, j.r, k.r), l.r, o.r);
    FfxFloat32 mxkG = ffxMax3(ffxMax3(g.g, j.g, k.g), l.g, o.g);
    FfxFloat32 mxkB = ffxMax3(ffxMax3(g.b, j.b, k.b), l.b, o.b);

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 mxkR2 = ffxMax3(ffxMax3(mxkR, f.r, h.r), n.r, p.r);
    FfxFloat32 mxkG2 = ffxMax3(ffxMax3(mxkG, f.g, h.g), n.g, p.g);
    FfxFloat32 mxkB2 = ffxMax3(ffxMax3(mxkB, f.b, h.b), n.b, p.b);
    mxkR             = mxkR + mxkR2;
    mxkG             = mxkG + mxkG2;
    mxkB             = mxkB + mxkB2;
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

#if defined(FFX_CAS_USE_PRECISE_MATH)
    // Smooth minimum distance to signal limit divided by smooth max.
    FfxFloat32 rcpMfR = ffxReciprocal(mxfR);
    FfxFloat32 rcpMfG = ffxReciprocal(mxfG);
    FfxFloat32 rcpMfB = ffxReciprocal(mxfB);
    FfxFloat32 rcpMgR = ffxReciprocal(mxgR);
    FfxFloat32 rcpMgG = ffxReciprocal(mxgG);
    FfxFloat32 rcpMgB = ffxReciprocal(mxgB);
    FfxFloat32 rcpMjR = ffxReciprocal(mxjR);
    FfxFloat32 rcpMjG = ffxReciprocal(mxjG);
    FfxFloat32 rcpMjB = ffxReciprocal(mxjB);
    FfxFloat32 rcpMkR = ffxReciprocal(mxkR);
    FfxFloat32 rcpMkG = ffxReciprocal(mxkG);
    FfxFloat32 rcpMkB = ffxReciprocal(mxkB);
#else
    // Smooth minimum distance to signal limit divided by smooth max.
    FfxFloat32 rcpMfR = ffxApproximateReciprocal(mxfR);
    FfxFloat32 rcpMfG = ffxApproximateReciprocal(mxfG);
    FfxFloat32 rcpMfB = ffxApproximateReciprocal(mxfB);
    FfxFloat32 rcpMgR = ffxApproximateReciprocal(mxgR);
    FfxFloat32 rcpMgG = ffxApproximateReciprocal(mxgG);
    FfxFloat32 rcpMgB = ffxApproximateReciprocal(mxgB);
    FfxFloat32 rcpMjR = ffxApproximateReciprocal(mxjR);
    FfxFloat32 rcpMjG = ffxApproximateReciprocal(mxjG);
    FfxFloat32 rcpMjB = ffxApproximateReciprocal(mxjB);
    FfxFloat32 rcpMkR = ffxApproximateReciprocal(mxkR);
    FfxFloat32 rcpMkG = ffxApproximateReciprocal(mxkG);
    FfxFloat32 rcpMkB = ffxApproximateReciprocal(mxkB);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

#if defined(FFX_CAS_BETTER_DIAGONALS)
    FfxFloat32 ampfR = ffxSaturate(ffxMin(minimumRed, FfxFloat32(2.0) - mxfR) * rcpMfR);
    FfxFloat32 ampfG = ffxSaturate(ffxMin(minimumGreen, FfxFloat32(2.0) - mxfG) * rcpMfG);
    FfxFloat32 ampfB = ffxSaturate(ffxMin(minimumBlue, FfxFloat32(2.0) - mxfB) * rcpMfB);
    FfxFloat32 ampgR = ffxSaturate(ffxMin(mngR, FfxFloat32(2.0) - mxgR) * rcpMgR);
    FfxFloat32 ampgG = ffxSaturate(ffxMin(mngG, FfxFloat32(2.0) - mxgG) * rcpMgG);
    FfxFloat32 ampgB = ffxSaturate(ffxMin(mngB, FfxFloat32(2.0) - mxgB) * rcpMgB);
    FfxFloat32 ampjR = ffxSaturate(ffxMin(mnjR, FfxFloat32(2.0) - mxjR) * rcpMjR);
    FfxFloat32 ampjG = ffxSaturate(ffxMin(mnjG, FfxFloat32(2.0) - mxjG) * rcpMjG);
    FfxFloat32 ampjB = ffxSaturate(ffxMin(mnjB, FfxFloat32(2.0) - mxjB) * rcpMjB);
    FfxFloat32 ampkR = ffxSaturate(ffxMin(mnkR, FfxFloat32(2.0) - mxkR) * rcpMkR);
    FfxFloat32 ampkG = ffxSaturate(ffxMin(mnkG, FfxFloat32(2.0) - mxkG) * rcpMkG);
    FfxFloat32 ampkB = ffxSaturate(ffxMin(mnkB, FfxFloat32(2.0) - mxkB) * rcpMkB);
#else
    FfxFloat32 ampfR = ffxSaturate(ffxMin(minimumRed, FfxFloat32(1.0) - mxfR) * rcpMfR);
    FfxFloat32 ampfG = ffxSaturate(ffxMin(minimumGreen, FfxFloat32(1.0) - mxfG) * rcpMfG);
    FfxFloat32 ampfB = ffxSaturate(ffxMin(minimumBlue, FfxFloat32(1.0) - mxfB) * rcpMfB);
    FfxFloat32 ampgR = ffxSaturate(ffxMin(mngR, FfxFloat32(1.0) - mxgR) * rcpMgR);
    FfxFloat32 ampgG = ffxSaturate(ffxMin(mngG, FfxFloat32(1.0) - mxgG) * rcpMgG);
    FfxFloat32 ampgB = ffxSaturate(ffxMin(mngB, FfxFloat32(1.0) - mxgB) * rcpMgB);
    FfxFloat32 ampjR = ffxSaturate(ffxMin(mnjR, FfxFloat32(1.0) - mxjR) * rcpMjR);
    FfxFloat32 ampjG = ffxSaturate(ffxMin(mnjG, FfxFloat32(1.0) - mxjG) * rcpMjG);
    FfxFloat32 ampjB = ffxSaturate(ffxMin(mnjB, FfxFloat32(1.0) - mxjB) * rcpMjB);
    FfxFloat32 ampkR = ffxSaturate(ffxMin(mnkR, FfxFloat32(1.0) - mxkR) * rcpMkR);
    FfxFloat32 ampkG = ffxSaturate(ffxMin(mnkG, FfxFloat32(1.0) - mxkG) * rcpMkG);
    FfxFloat32 ampkB = ffxSaturate(ffxMin(mnkB, FfxFloat32(1.0) - mxkB) * rcpMkB);
#endif  // #if defined(FFX_CAS_BETTER_DIAGONALS)

#if defined(FFX_CAS_USE_PRECISE_MATH)
    // Shaping amount of sharpening.
    ampfR = ffxSqrt(ampfR);
    ampfG = ffxSqrt(ampfG);
    ampfB = ffxSqrt(ampfB);
    ampgR = ffxSqrt(ampgR);
    ampgG = ffxSqrt(ampgG);
    ampgB = ffxSqrt(ampgB);
    ampjR = ffxSqrt(ampjR);
    ampjG = ffxSqrt(ampjG);
    ampjB = ffxSqrt(ampjB);
    ampkR = ffxSqrt(ampkR);
    ampkG = ffxSqrt(ampkG);
    ampkB = ffxSqrt(ampkB);
#else
    // Shaping amount of sharpening.
    ampfR = ffxApproximateSqrt(ampfR);
    ampfG = ffxApproximateSqrt(ampfG);
    ampfB = ffxApproximateSqrt(ampfB);
    ampgR = ffxApproximateSqrt(ampgR);
    ampgG = ffxApproximateSqrt(ampgG);
    ampgB = ffxApproximateSqrt(ampgB);
    ampjR = ffxApproximateSqrt(ampjR);
    ampjG = ffxApproximateSqrt(ampjG);
    ampjB = ffxApproximateSqrt(ampjB);
    ampkR = ffxApproximateSqrt(ampkR);
    ampkG = ffxApproximateSqrt(ampkG);
    ampkB = ffxApproximateSqrt(ampkB);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Filter shape.
    //  0 w 0
    //  w 1 w
    //  0 w 0
    FfxFloat32 peak = ffxAsFloat(const1.x);
    FfxFloat32 wfR  = ampfR * peak;
    FfxFloat32 wfG  = ampfG * peak;
    FfxFloat32 wfB  = ampfB * peak;
    FfxFloat32 wgR  = ampgR * peak;
    FfxFloat32 wgG  = ampgG * peak;
    FfxFloat32 wgB  = ampgB * peak;
    FfxFloat32 wjR  = ampjR * peak;
    FfxFloat32 wjG  = ampjG * peak;
    FfxFloat32 wjB  = ampjB * peak;
    FfxFloat32 wkR  = ampkR * peak;
    FfxFloat32 wkG  = ampkG * peak;
    FfxFloat32 wkB  = ampkB * peak;

    // Blend between 4 results.
    //  s t
    //  u v
    FfxFloat32 s = (FfxFloat32(1.0) - pixelPosition.x) * (FfxFloat32(1.0) - pixelPosition.y);
    FfxFloat32 t = pixelPosition.x * (FfxFloat32(1.0) - pixelPosition.y);
    FfxFloat32 u = (FfxFloat32(1.0) - pixelPosition.x) * pixelPosition.y;
    FfxFloat32 v = pixelPosition.x * pixelPosition.y;

    // Thin edges to hide bilinear interpolation (helps diagonals).
    FfxFloat32 thinB = 1.0 / 32.0;

#if defined(FFX_CAS_USE_PRECISE_MATH)
    s *= ffxReciprocal(thinB + (mxfG - minimumGreen));
    t *= ffxReciprocal(thinB + (mxgG - mngG));
    u *= ffxReciprocal(thinB + (mxjG - mnjG));
    v *= ffxReciprocal(thinB + (mxkG - mnkG));
#else
    s *= ffxApproximateReciprocal(thinB + (mxfG - minimumGreen));
    t *= ffxApproximateReciprocal(thinB + (mxgG - mngG));
    u *= ffxApproximateReciprocal(thinB + (mxjG - mnjG));
    v *= ffxApproximateReciprocal(thinB + (mxkG - mnkG));
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Final weighting.
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    //  _____  _____  _____  _____
    //         fs        gt
    //
    //  _____  _____  _____  _____
    //  fs      s gt  fs  t     gt
    //         ju        kv
    //  _____  _____  _____  _____
    //         fs        gt
    //  ju      u kv  ju  v     kv
    //  _____  _____  _____  _____
    //
    //         ju        kv
    FfxFloat32 qbeR = wfR * s;
    FfxFloat32 qbeG = wfG * s;
    FfxFloat32 qbeB = wfB * s;
    FfxFloat32 qchR = wgR * t;
    FfxFloat32 qchG = wgG * t;
    FfxFloat32 qchB = wgB * t;
    FfxFloat32 qfR  = wgR * t + wjR * u + s;
    FfxFloat32 qfG  = wgG * t + wjG * u + s;
    FfxFloat32 qfB  = wgB * t + wjB * u + s;
    FfxFloat32 qgR  = wfR * s + wkR * v + t;
    FfxFloat32 qgG  = wfG * s + wkG * v + t;
    FfxFloat32 qgB  = wfB * s + wkB * v + t;
    FfxFloat32 qjR  = wfR * s + wkR * v + u;
    FfxFloat32 qjG  = wfG * s + wkG * v + u;
    FfxFloat32 qjB  = wfB * s + wkB * v + u;
    FfxFloat32 qkR  = wgR * t + wjR * u + v;
    FfxFloat32 qkG  = wgG * t + wjG * u + v;
    FfxFloat32 qkB  = wgB * t + wjB * u + v;
    FfxFloat32 qinR = wjR * u;
    FfxFloat32 qinG = wjG * u;
    FfxFloat32 qinB = wjB * u;
    FfxFloat32 qloR = wkR * v;
    FfxFloat32 qloG = wkG * v;
    FfxFloat32 qloB = wkB * v;

    // Using green coef only, depending on dead code removal to strip out the extra overhead.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat32 rcpWG = ffxReciprocal(FfxFloat32(2.0) * qbeG + FfxFloat32(2.0) * qchG + FfxFloat32(2.0) * qinG + FfxFloat32(2.0) * qloG + qfG + qgG + qjG + qkG);
#else
    FfxFloat32 rcpWG = ffxApproximateReciprocalMedium(FfxFloat32(2.0) * qbeG + FfxFloat32(2.0) * qchG + FfxFloat32(2.0) * qinG + FfxFloat32(2.0) * qloG + qfG +
                                                      qgG + qjG + qkG);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    pixR = ffxSaturate((b.r * qbeG + e.r * qbeG + c.r * qchG + h.r * qchG + i.r * qinG + n.r * qinG + l.r * qloG + o.r * qloG + f.r * qfG + g.r * qgG +
                        j.r * qjG +
                     k.r * qkG) *
                    rcpWG);
    pixG = ffxSaturate((b.g * qbeG + e.g * qbeG + c.g * qchG + h.g * qchG + i.g * qinG + n.g * qinG + l.g * qloG + o.g * qloG + f.g * qfG + g.g * qgG +
                        j.g * qjG +
                     k.g * qkG) *
                    rcpWG);
    pixB = ffxSaturate((b.b * qbeG + e.b * qbeG + c.b * qchG + h.b * qchG + i.b * qinG + n.b * qinG + l.b * qloG + o.b * qloG + f.b * qfG + g.b * qgG +
                        j.b * qjG +
                     k.b * qkG) *
                    rcpWG);
}

#if FFX_HALF == 1
// Half precision version algorithm with scaling and filters 2 tiles in one run.
void casFilterWithScalingHalf(
    FFX_PARAMETER_OUT FfxFloat16x2 pixR,
    FFX_PARAMETER_OUT FfxFloat16x2 pixG,
    FFX_PARAMETER_OUT FfxFloat16x2 pixB,
    FFX_PARAMETER_IN FfxUInt32x2 ip,      // Integer pixel position in output.
    FFX_PARAMETER_IN FfxUInt32x4 const0,  // Constants generated by ffxCasSetup().
    FFX_PARAMETER_IN FfxUInt32x4 const1)
{
    FfxFloat32x2 pp = FfxFloat32x2(ip) * ffxAsFloat(const0.xy) + ffxAsFloat(const0.zw);

    // Tile 0.
    // Fractional position is needed in high precision here.
    FfxFloat32x2 fp0 = floor(pp);
    FfxFloat16x2 ppX;
    ppX.x            = FfxFloat16(pp.x - fp0.x);
    FfxFloat16   ppY = FfxFloat16(pp.y - fp0.y);
    FfxInt16x2   sp0 = FfxInt16x2(fp0);
    FfxFloat16x3 a0  = casLoadHalf(sp0 + FfxInt16x2(-1, -1));
    FfxFloat16x3 b0  = casLoadHalf(sp0 + FfxInt16x2(0, -1));
    FfxFloat16x3 e0  = casLoadHalf(sp0 + FfxInt16x2(-1, 0));
    FfxFloat16x3 f0  = casLoadHalf(sp0);
    FfxFloat16x3 c0  = casLoadHalf(sp0 + FfxInt16x2(1, -1));
    FfxFloat16x3 d0  = casLoadHalf(sp0 + FfxInt16x2(2, -1));
    FfxFloat16x3 g0  = casLoadHalf(sp0 + FfxInt16x2(1, 0));
    FfxFloat16x3 h0  = casLoadHalf(sp0 + FfxInt16x2(2, 0));
    FfxFloat16x3 i0  = casLoadHalf(sp0 + FfxInt16x2(-1, 1));
    FfxFloat16x3 j0  = casLoadHalf(sp0 + FfxInt16x2(0, 1));
    FfxFloat16x3 m0  = casLoadHalf(sp0 + FfxInt16x2(-1, 2));
    FfxFloat16x3 n0  = casLoadHalf(sp0 + FfxInt16x2(0, 2));
    FfxFloat16x3 k0  = casLoadHalf(sp0 + FfxInt16x2(1, 1));
    FfxFloat16x3 l0  = casLoadHalf(sp0 + FfxInt16x2(2, 1));
    FfxFloat16x3 o0  = casLoadHalf(sp0 + FfxInt16x2(1, 2));
    FfxFloat16x3 p0  = casLoadHalf(sp0 + FfxInt16x2(2, 2));

    // Tile 1 (offset only in x).
    FfxFloat32 pp1   = pp.x + ffxAsFloat(const1.z);
    FfxFloat32 fp1   = floor(pp1);
    ppX.y            = FfxFloat16(pp1 - fp1);
    FfxInt16x2   sp1 = FfxInt16x2(fp1, sp0.y);
    FfxFloat16x3 a1  = casLoadHalf(sp1 + FfxInt16x2(-1, -1));
    FfxFloat16x3 b1  = casLoadHalf(sp1 + FfxInt16x2(0, -1));
    FfxFloat16x3 e1  = casLoadHalf(sp1 + FfxInt16x2(-1, 0));
    FfxFloat16x3 f1  = casLoadHalf(sp1);
    FfxFloat16x3 c1  = casLoadHalf(sp1 + FfxInt16x2(1, -1));
    FfxFloat16x3 d1  = casLoadHalf(sp1 + FfxInt16x2(2, -1));
    FfxFloat16x3 g1  = casLoadHalf(sp1 + FfxInt16x2(1, 0));
    FfxFloat16x3 h1  = casLoadHalf(sp1 + FfxInt16x2(2, 0));
    FfxFloat16x3 i1  = casLoadHalf(sp1 + FfxInt16x2(-1, 1));
    FfxFloat16x3 j1  = casLoadHalf(sp1 + FfxInt16x2(0, 1));
    FfxFloat16x3 m1  = casLoadHalf(sp1 + FfxInt16x2(-1, 2));
    FfxFloat16x3 n1  = casLoadHalf(sp1 + FfxInt16x2(0, 2));
    FfxFloat16x3 k1  = casLoadHalf(sp1 + FfxInt16x2(1, 1));
    FfxFloat16x3 l1  = casLoadHalf(sp1 + FfxInt16x2(2, 1));
    FfxFloat16x3 o1  = casLoadHalf(sp1 + FfxInt16x2(1, 2));
    FfxFloat16x3 p1  = casLoadHalf(sp1 + FfxInt16x2(2, 2));

    // AOS to SOA conversion.
    FfxFloat16x2 aR = FfxFloat16x2(a0.r, a1.r);
    FfxFloat16x2 aG = FfxFloat16x2(a0.g, a1.g);
    FfxFloat16x2 aB = FfxFloat16x2(a0.b, a1.b);
    FfxFloat16x2 bR = FfxFloat16x2(b0.r, b1.r);
    FfxFloat16x2 bG = FfxFloat16x2(b0.g, b1.g);
    FfxFloat16x2 bB = FfxFloat16x2(b0.b, b1.b);
    FfxFloat16x2 cR = FfxFloat16x2(c0.r, c1.r);
    FfxFloat16x2 cG = FfxFloat16x2(c0.g, c1.g);
    FfxFloat16x2 cB = FfxFloat16x2(c0.b, c1.b);
    FfxFloat16x2 dR = FfxFloat16x2(d0.r, d1.r);
    FfxFloat16x2 dG = FfxFloat16x2(d0.g, d1.g);
    FfxFloat16x2 dB = FfxFloat16x2(d0.b, d1.b);
    FfxFloat16x2 eR = FfxFloat16x2(e0.r, e1.r);
    FfxFloat16x2 eG = FfxFloat16x2(e0.g, e1.g);
    FfxFloat16x2 eB = FfxFloat16x2(e0.b, e1.b);
    FfxFloat16x2 fR = FfxFloat16x2(f0.r, f1.r);
    FfxFloat16x2 fG = FfxFloat16x2(f0.g, f1.g);
    FfxFloat16x2 fB = FfxFloat16x2(f0.b, f1.b);
    FfxFloat16x2 gR = FfxFloat16x2(g0.r, g1.r);
    FfxFloat16x2 gG = FfxFloat16x2(g0.g, g1.g);
    FfxFloat16x2 gB = FfxFloat16x2(g0.b, g1.b);
    FfxFloat16x2 hR = FfxFloat16x2(h0.r, h1.r);
    FfxFloat16x2 hG = FfxFloat16x2(h0.g, h1.g);
    FfxFloat16x2 hB = FfxFloat16x2(h0.b, h1.b);
    FfxFloat16x2 iR = FfxFloat16x2(i0.r, i1.r);
    FfxFloat16x2 iG = FfxFloat16x2(i0.g, i1.g);
    FfxFloat16x2 iB = FfxFloat16x2(i0.b, i1.b);
    FfxFloat16x2 jR = FfxFloat16x2(j0.r, j1.r);
    FfxFloat16x2 jG = FfxFloat16x2(j0.g, j1.g);
    FfxFloat16x2 jB = FfxFloat16x2(j0.b, j1.b);
    FfxFloat16x2 kR = FfxFloat16x2(k0.r, k1.r);
    FfxFloat16x2 kG = FfxFloat16x2(k0.g, k1.g);
    FfxFloat16x2 kB = FfxFloat16x2(k0.b, k1.b);
    FfxFloat16x2 lR = FfxFloat16x2(l0.r, l1.r);
    FfxFloat16x2 lG = FfxFloat16x2(l0.g, l1.g);
    FfxFloat16x2 lB = FfxFloat16x2(l0.b, l1.b);
    FfxFloat16x2 mR = FfxFloat16x2(m0.r, m1.r);
    FfxFloat16x2 mG = FfxFloat16x2(m0.g, m1.g);
    FfxFloat16x2 mB = FfxFloat16x2(m0.b, m1.b);
    FfxFloat16x2 nR = FfxFloat16x2(n0.r, n1.r);
    FfxFloat16x2 nG = FfxFloat16x2(n0.g, n1.g);
    FfxFloat16x2 nB = FfxFloat16x2(n0.b, n1.b);
    FfxFloat16x2 oR = FfxFloat16x2(o0.r, o1.r);
    FfxFloat16x2 oG = FfxFloat16x2(o0.g, o1.g);
    FfxFloat16x2 oB = FfxFloat16x2(o0.b, o1.b);
    FfxFloat16x2 pR = FfxFloat16x2(p0.r, p1.r);
    FfxFloat16x2 pG = FfxFloat16x2(p0.g, p1.g);
    FfxFloat16x2 pB = FfxFloat16x2(p0.b, p1.b);

    // Run optional input transform.
    casInputHalf(aR, aG, aB);
    casInputHalf(bR, bG, bB);
    casInputHalf(cR, cG, cB);
    casInputHalf(dR, dG, dB);
    casInputHalf(eR, eG, eB);
    casInputHalf(fR, fG, fB);
    casInputHalf(gR, gG, gB);
    casInputHalf(hR, hG, hB);
    casInputHalf(iR, iG, iB);
    casInputHalf(jR, jG, jB);
    casInputHalf(kR, kG, kB);
    casInputHalf(lR, lG, lB);
    casInputHalf(mR, mG, mB);
    casInputHalf(nR, nG, nB);
    casInputHalf(oR, oG, oB);
    casInputHalf(pR, pG, pB);

    // Soft min and max.
    // These are 2.0x bigger (factored out the extra multiply).
    //  a b c             b
    //  e f g * 0.5  +  e f g * 0.5  [F]
    //  i j k             j
    FfxFloat16x2 minimumRed = ffxMin3Half(ffxMin3Half(bR, eR, fR), gR, jR);
    FfxFloat16x2 minimumGreen = ffxMin3Half(ffxMin3Half(bG, eG, fG), gG, jG);
    FfxFloat16x2 minimumBlue = ffxMin3Half(ffxMin3Half(bB, eB, fB), gB, jB);

#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mnfR2 = ffxMin3Half(ffxMin3Half(minimumRed, aR, cR), iR, kR);
    FfxFloat16x2 mnfG2 = ffxMin3Half(ffxMin3Half(minimumGreen, aG, cG), iG, kG);
    FfxFloat16x2 mnfB2 = ffxMin3Half(ffxMin3Half(minimumBlue, aB, cB), iB, kB);
    minimumRed               = minimumRed + mnfR2;
    minimumGreen               = minimumGreen + mnfG2;
    minimumBlue               = minimumBlue + mnfB2;
#endif
    FfxFloat16x2 mxfR = ffxMax3Half(ffxMax3Half(bR, eR, fR), gR, jR);
    FfxFloat16x2 mxfG = ffxMax3Half(ffxMax3Half(bG, eG, fG), gG, jG);
    FfxFloat16x2 mxfB = ffxMax3Half(ffxMax3Half(bB, eB, fB), gB, jB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mxfR2 = ffxMax3Half(ffxMax3Half(mxfR, aR, cR), iR, kR);
    FfxFloat16x2 mxfG2 = ffxMax3Half(ffxMax3Half(mxfG, aG, cG), iG, kG);
    FfxFloat16x2 mxfB2 = ffxMax3Half(ffxMax3Half(mxfB, aB, cB), iB, kB);
    mxfR               = mxfR + mxfR2;
    mxfG               = mxfG + mxfG2;
    mxfB               = mxfB + mxfB2;
#endif
    //  b c d             c
    //  f g h * 0.5  +  f g h * 0.5  [G]
    //  j k l             k
    FfxFloat16x2 mngR = ffxMin3Half(ffxMin3Half(cR, fR, gR), hR, kR);
    FfxFloat16x2 mngG = ffxMin3Half(ffxMin3Half(cG, fG, gG), hG, kG);
    FfxFloat16x2 mngB = ffxMin3Half(ffxMin3Half(cB, fB, gB), hB, kB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mngR2 = ffxMin3Half(ffxMin3Half(mngR, bR, dR), jR, lR);
    FfxFloat16x2 mngG2 = ffxMin3Half(ffxMin3Half(mngG, bG, dG), jG, lG);
    FfxFloat16x2 mngB2 = ffxMin3Half(ffxMin3Half(mngB, bB, dB), jB, lB);
    mngR               = mngR + mngR2;
    mngG               = mngG + mngG2;
    mngB               = mngB + mngB2;
#endif
    FfxFloat16x2 mxgR = ffxMax3Half(ffxMax3Half(cR, fR, gR), hR, kR);
    FfxFloat16x2 mxgG = ffxMax3Half(ffxMax3Half(cG, fG, gG), hG, kG);
    FfxFloat16x2 mxgB = ffxMax3Half(ffxMax3Half(cB, fB, gB), hB, kB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mxgR2 = ffxMax3Half(ffxMax3Half(mxgR, bR, dR), jR, lR);
    FfxFloat16x2 mxgG2 = ffxMax3Half(ffxMax3Half(mxgG, bG, dG), jG, lG);
    FfxFloat16x2 mxgB2 = ffxMax3Half(ffxMax3Half(mxgB, bB, dB), jB, lB);
    mxgR               = mxgR + mxgR2;
    mxgG               = mxgG + mxgG2;
    mxgB               = mxgB + mxgB2;
#endif
    //  e f g             f
    //  i j k * 0.5  +  i j k * 0.5  [J]
    //  m n o             n
    FfxFloat16x2 mnjR = ffxMin3Half(ffxMin3Half(fR, iR, jR), kR, nR);
    FfxFloat16x2 mnjG = ffxMin3Half(ffxMin3Half(fG, iG, jG), kG, nG);
    FfxFloat16x2 mnjB = ffxMin3Half(ffxMin3Half(fB, iB, jB), kB, nB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mnjR2 = ffxMin3Half(ffxMin3Half(mnjR, eR, gR), mR, oR);
    FfxFloat16x2 mnjG2 = ffxMin3Half(ffxMin3Half(mnjG, eG, gG), mG, oG);
    FfxFloat16x2 mnjB2 = ffxMin3Half(ffxMin3Half(mnjB, eB, gB), mB, oB);
    mnjR               = mnjR + mnjR2;
    mnjG               = mnjG + mnjG2;
    mnjB               = mnjB + mnjB2;
#endif
    FfxFloat16x2 mxjR = ffxMax3Half(ffxMax3Half(fR, iR, jR), kR, nR);
    FfxFloat16x2 mxjG = ffxMax3Half(ffxMax3Half(fG, iG, jG), kG, nG);
    FfxFloat16x2 mxjB = ffxMax3Half(ffxMax3Half(fB, iB, jB), kB, nB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mxjR2 = ffxMax3Half(ffxMax3Half(mxjR, eR, gR), mR, oR);
    FfxFloat16x2 mxjG2 = ffxMax3Half(ffxMax3Half(mxjG, eG, gG), mG, oG);
    FfxFloat16x2 mxjB2 = ffxMax3Half(ffxMax3Half(mxjB, eB, gB), mB, oB);
    mxjR               = mxjR + mxjR2;
    mxjG               = mxjG + mxjG2;
    mxjB               = mxjB + mxjB2;
#endif
    //  f g h             g
    //  j k l * 0.5  +  j k l * 0.5  [K]
    //  n o p             o
    FfxFloat16x2 mnkR = ffxMin3Half(ffxMin3Half(gR, jR, kR), lR, oR);
    FfxFloat16x2 mnkG = ffxMin3Half(ffxMin3Half(gG, jG, kG), lG, oG);
    FfxFloat16x2 mnkB = ffxMin3Half(ffxMin3Half(gB, jB, kB), lB, oB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mnkR2 = ffxMin3Half(ffxMin3Half(mnkR, fR, hR), nR, pR);
    FfxFloat16x2 mnkG2 = ffxMin3Half(ffxMin3Half(mnkG, fG, hG), nG, pG);
    FfxFloat16x2 mnkB2 = ffxMin3Half(ffxMin3Half(mnkB, fB, hB), nB, pB);
    mnkR               = mnkR + mnkR2;
    mnkG               = mnkG + mnkG2;
    mnkB               = mnkB + mnkB2;
#endif
    FfxFloat16x2 mxkR = ffxMax3Half(ffxMax3Half(gR, jR, kR), lR, oR);
    FfxFloat16x2 mxkG = ffxMax3Half(ffxMax3Half(gG, jG, kG), lG, oG);
    FfxFloat16x2 mxkB = ffxMax3Half(ffxMax3Half(gB, jB, kB), lB, oB);
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 mxkR2 = ffxMax3Half(ffxMax3Half(mxkR, fR, hR), nR, pR);
    FfxFloat16x2 mxkG2 = ffxMax3Half(ffxMax3Half(mxkG, fG, hG), nG, pG);
    FfxFloat16x2 mxkB2 = ffxMax3Half(ffxMax3Half(mxkB, fB, hB), nB, pB);
    mxkR               = mxkR + mxkR2;
    mxkG               = mxkG + mxkG2;
    mxkB               = mxkB + mxkB2;
#endif
    // Smooth minimum distance to signal limit divided by smooth max.
#ifdef FFX_CAS_USE_PRECISE_MATH
    FfxFloat16x2 rcpMfR = ffxReciprocalHalf(mxfR);
    FfxFloat16x2 rcpMfG = ffxReciprocalHalf(mxfG);
    FfxFloat16x2 rcpMfB = ffxReciprocalHalf(mxfB);
    FfxFloat16x2 rcpMgR = ffxReciprocalHalf(mxgR);
    FfxFloat16x2 rcpMgG = ffxReciprocalHalf(mxgG);
    FfxFloat16x2 rcpMgB = ffxReciprocalHalf(mxgB);
    FfxFloat16x2 rcpMjR = ffxReciprocalHalf(mxjR);
    FfxFloat16x2 rcpMjG = ffxReciprocalHalf(mxjG);
    FfxFloat16x2 rcpMjB = ffxReciprocalHalf(mxjB);
    FfxFloat16x2 rcpMkR = ffxReciprocalHalf(mxkR);
    FfxFloat16x2 rcpMkG = ffxReciprocalHalf(mxkG);
    FfxFloat16x2 rcpMkB = ffxReciprocalHalf(mxkB);
#else
    FfxFloat16x2 rcpMfR = ffxApproximateReciprocalHalf(mxfR);
    FfxFloat16x2 rcpMfG = ffxApproximateReciprocalHalf(mxfG);
    FfxFloat16x2 rcpMfB = ffxApproximateReciprocalHalf(mxfB);
    FfxFloat16x2 rcpMgR = ffxApproximateReciprocalHalf(mxgR);
    FfxFloat16x2 rcpMgG = ffxApproximateReciprocalHalf(mxgG);
    FfxFloat16x2 rcpMgB = ffxApproximateReciprocalHalf(mxgB);
    FfxFloat16x2 rcpMjR = ffxApproximateReciprocalHalf(mxjR);
    FfxFloat16x2 rcpMjG = ffxApproximateReciprocalHalf(mxjG);
    FfxFloat16x2 rcpMjB = ffxApproximateReciprocalHalf(mxjB);
    FfxFloat16x2 rcpMkR = ffxApproximateReciprocalHalf(mxkR);
    FfxFloat16x2 rcpMkG = ffxApproximateReciprocalHalf(mxkG);
    FfxFloat16x2 rcpMkB = ffxApproximateReciprocalHalf(mxkB);
#endif
#ifdef FFX_CAS_BETTER_DIAGONALS
    FfxFloat16x2 ampfR = ffxSaturate(min(minimumRed, FFX_BROADCAST_FLOAT16X2(2.0) - mxfR) * rcpMfR);
    FfxFloat16x2 ampfG = ffxSaturate(min(minimumGreen, FFX_BROADCAST_FLOAT16X2(2.0) - mxfG) * rcpMfG);
    FfxFloat16x2 ampfB = ffxSaturate(min(minimumBlue, FFX_BROADCAST_FLOAT16X2(2.0) - mxfB) * rcpMfB);
    FfxFloat16x2 ampgR = ffxSaturate(min(mngR, FFX_BROADCAST_FLOAT16X2(2.0) - mxgR) * rcpMgR);
    FfxFloat16x2 ampgG = ffxSaturate(min(mngG, FFX_BROADCAST_FLOAT16X2(2.0) - mxgG) * rcpMgG);
    FfxFloat16x2 ampgB = ffxSaturate(min(mngB, FFX_BROADCAST_FLOAT16X2(2.0) - mxgB) * rcpMgB);
    FfxFloat16x2 ampjR = ffxSaturate(min(mnjR, FFX_BROADCAST_FLOAT16X2(2.0) - mxjR) * rcpMjR);
    FfxFloat16x2 ampjG = ffxSaturate(min(mnjG, FFX_BROADCAST_FLOAT16X2(2.0) - mxjG) * rcpMjG);
    FfxFloat16x2 ampjB = ffxSaturate(min(mnjB, FFX_BROADCAST_FLOAT16X2(2.0) - mxjB) * rcpMjB);
    FfxFloat16x2 ampkR = ffxSaturate(min(mnkR, FFX_BROADCAST_FLOAT16X2(2.0) - mxkR) * rcpMkR);
    FfxFloat16x2 ampkG = ffxSaturate(min(mnkG, FFX_BROADCAST_FLOAT16X2(2.0) - mxkG) * rcpMkG);
    FfxFloat16x2 ampkB = ffxSaturate(min(mnkB, FFX_BROADCAST_FLOAT16X2(2.0) - mxkB) * rcpMkB);
#else
    FfxFloat16x2 ampfR  = FfxFloat16x2(ffxSaturate(min(minimumRed, FFX_BROADCAST_FLOAT16X2(1.0) - mxfR) * rcpMfR));
    FfxFloat16x2 ampfG  = FfxFloat16x2(ffxSaturate(min(minimumGreen, FFX_BROADCAST_FLOAT16X2(1.0) - mxfG) * rcpMfG));
    FfxFloat16x2 ampfB  = FfxFloat16x2(ffxSaturate(min(minimumBlue, FFX_BROADCAST_FLOAT16X2(1.0) - mxfB) * rcpMfB));
    FfxFloat16x2 ampgR  = FfxFloat16x2(ffxSaturate(min(mngR, FFX_BROADCAST_FLOAT16X2(1.0) - mxgR) * rcpMgR));
    FfxFloat16x2 ampgG  = FfxFloat16x2(ffxSaturate(min(mngG, FFX_BROADCAST_FLOAT16X2(1.0) - mxgG) * rcpMgG));
    FfxFloat16x2 ampgB  = FfxFloat16x2(ffxSaturate(min(mngB, FFX_BROADCAST_FLOAT16X2(1.0) - mxgB) * rcpMgB));
    FfxFloat16x2 ampjR  = FfxFloat16x2(ffxSaturate(min(mnjR, FFX_BROADCAST_FLOAT16X2(1.0) - mxjR) * rcpMjR));
    FfxFloat16x2 ampjG  = FfxFloat16x2(ffxSaturate(min(mnjG, FFX_BROADCAST_FLOAT16X2(1.0) - mxjG) * rcpMjG));
    FfxFloat16x2 ampjB  = FfxFloat16x2(ffxSaturate(min(mnjB, FFX_BROADCAST_FLOAT16X2(1.0) - mxjB) * rcpMjB));
    FfxFloat16x2 ampkR  = FfxFloat16x2(ffxSaturate(min(mnkR, FFX_BROADCAST_FLOAT16X2(1.0) - mxkR) * rcpMkR));
    FfxFloat16x2 ampkG  = FfxFloat16x2(ffxSaturate(min(mnkG, FFX_BROADCAST_FLOAT16X2(1.0) - mxkG) * rcpMkG));
    FfxFloat16x2 ampkB  = FfxFloat16x2(ffxSaturate(min(mnkB, FFX_BROADCAST_FLOAT16X2(1.0) - mxkB) * rcpMkB));
#endif

    // Shaping amount of sharpening.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    ampfR = ffxSqrt(ampfR);
    ampfG = ffxSqrt(ampfG);
    ampfB = ffxSqrt(ampfB);
    ampgR = ffxSqrt(ampgR);
    ampgG = ffxSqrt(ampgG);
    ampgB = ffxSqrt(ampgB);
    ampjR = ffxSqrt(ampjR);
    ampjG = ffxSqrt(ampjG);
    ampjB = ffxSqrt(ampjB);
    ampkR = ffxSqrt(ampkR);
    ampkG = ffxSqrt(ampkG);
    ampkB = ffxSqrt(ampkB);
#else
    ampfR = ffxApproximateSqrtHalf(ampfR);
    ampfG = ffxApproximateSqrtHalf(ampfG);
    ampfB = ffxApproximateSqrtHalf(ampfB);
    ampgR = ffxApproximateSqrtHalf(ampgR);
    ampgG = ffxApproximateSqrtHalf(ampgG);
    ampgB = ffxApproximateSqrtHalf(ampgB);
    ampjR = ffxApproximateSqrtHalf(ampjR);
    ampjG = ffxApproximateSqrtHalf(ampjG);
    ampjB = ffxApproximateSqrtHalf(ampjB);
    ampkR = ffxApproximateSqrtHalf(ampkR);
    ampkG = ffxApproximateSqrtHalf(ampkG);
    ampkB = ffxApproximateSqrtHalf(ampkB);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Filter shape.
    FfxFloat16   peak = FFX_UINT32_TO_FLOAT16X2(const1.y).x;
    FfxFloat16x2 wfR  = ampfR * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wfG  = ampfG * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wfB  = ampfB * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wgR  = ampgR * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wgG  = ampgG * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wgB  = ampgB * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wjR  = ampjR * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wjG  = ampjG * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wjB  = ampjB * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wkR  = ampkR * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wkG  = ampkG * FFX_BROADCAST_FLOAT16X2(peak);
    FfxFloat16x2 wkB  = ampkB * FFX_BROADCAST_FLOAT16X2(peak);

    // Blend between 4 results.
    FfxFloat16x2 s = (FFX_BROADCAST_FLOAT16X2(1.0) - ppX) * (FFX_BROADCAST_FLOAT16X2(1.0) - FFX_BROADCAST_FLOAT16X2(ppY));
    FfxFloat16x2 t = ppX * (FFX_BROADCAST_FLOAT16X2(1.0) - FFX_BROADCAST_FLOAT16X2(ppY));
    FfxFloat16x2 u = (FFX_BROADCAST_FLOAT16X2(1.0) - ppX) * FFX_BROADCAST_FLOAT16X2(ppY);
    FfxFloat16x2 v = ppX * FFX_BROADCAST_FLOAT16X2(ppY);

    // Thin edges to hide bilinear interpolation (helps diagonals).
    FfxFloat16x2 thinB = FFX_BROADCAST_FLOAT16X2(1.0 / 32.0);

#if defined(FFX_CAS_USE_PRECISE_MATH)
    s *= ffxReciprocalHalf(thinB + (mxfG - minimumGreen));
    t *= ffxReciprocalHalf(thinB + (mxgG - mngG));
    u *= ffxReciprocalHalf(thinB + (mxjG - mnjG));
    v *= ffxReciprocalHalf(thinB + (mxkG - mnkG));
#else
    s *= ffxApproximateReciprocalHalf(thinB + (mxfG - minimumGreen));
    t *= ffxApproximateReciprocalHalf(thinB + (mxgG - mngG));
    u *= ffxApproximateReciprocalHalf(thinB + (mxjG - mnjG));
    v *= ffxApproximateReciprocalHalf(thinB + (mxkG - mnkG));
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    // Final weighting.
    FfxFloat16x2 qbeR = wfR * s;
    FfxFloat16x2 qbeG = wfG * s;
    FfxFloat16x2 qbeB = wfB * s;
    FfxFloat16x2 qchR = wgR * t;
    FfxFloat16x2 qchG = wgG * t;
    FfxFloat16x2 qchB = wgB * t;
    FfxFloat16x2 qfR  = wgR * t + wjR * u + s;
    FfxFloat16x2 qfG  = wgG * t + wjG * u + s;
    FfxFloat16x2 qfB  = wgB * t + wjB * u + s;
    FfxFloat16x2 qgR  = wfR * s + wkR * v + t;
    FfxFloat16x2 qgG  = wfG * s + wkG * v + t;
    FfxFloat16x2 qgB  = wfB * s + wkB * v + t;
    FfxFloat16x2 qjR  = wfR * s + wkR * v + u;
    FfxFloat16x2 qjG  = wfG * s + wkG * v + u;
    FfxFloat16x2 qjB  = wfB * s + wkB * v + u;
    FfxFloat16x2 qkR  = wgR * t + wjR * u + v;
    FfxFloat16x2 qkG  = wgG * t + wjG * u + v;
    FfxFloat16x2 qkB  = wgB * t + wjB * u + v;
    FfxFloat16x2 qinR = wjR * u;
    FfxFloat16x2 qinG = wjG * u;
    FfxFloat16x2 qinB = wjB * u;
    FfxFloat16x2 qloR = wkR * v;
    FfxFloat16x2 qloG = wkG * v;
    FfxFloat16x2 qloB = wkB * v;

    // Filter.
#if defined(FFX_CAS_USE_PRECISE_MATH)
    FfxFloat16x2 rcpWG = ffxReciprocalHalf(FFX_BROADCAST_FLOAT16X2(2.0) * qbeG + FFX_BROADCAST_FLOAT16X2(2.0) * qchG + FFX_BROADCAST_FLOAT16X2(2.0) * qinG +
                                           FFX_BROADCAST_FLOAT16X2(2.0) * qloG + qfG + qgG + qjG + qkG);
#else
    FfxFloat16x2 rcpWG = ffxApproximateReciprocalMediumHalf(
                            FFX_BROADCAST_FLOAT16X2(2.0) * qbeG + FFX_BROADCAST_FLOAT16X2(2.0) * qchG + FFX_BROADCAST_FLOAT16X2(2.0) * qinG + 
                            FFX_BROADCAST_FLOAT16X2(2.0) * qloG + qfG + qgG + qjG + qkG);
#endif  // #if defined(FFX_CAS_USE_PRECISE_MATH)

    pixR = FfxFloat16x2(ffxSaturate(
        (bR * qbeG + eR * qbeG + cR * qchG + hR * qchG + iR * qinG + nR * qinG + lR * qloG + oR * qloG + fR * qfG + gR * qgG + jR * qjG + kR * qkG) * rcpWG));
    pixG = FfxFloat16x2(ffxSaturate(
        (bG * qbeG + eG * qbeG + cG * qchG + hG * qchG + iG * qinG + nG * qinG + lG * qloG + oG * qloG + fG * qfG + gG * qgG + jG * qjG + kG * qkG) * rcpWG));
    pixB = FfxFloat16x2(ffxSaturate(
        (bB * qbeG + eB * qbeG + cB * qchG + hB * qchG + iB * qinG + nB * qinG + lB * qloG + oB * qloG + fB * qfG + gB * qgG + jB * qjG + kB * qkG) * rcpWG));
}
#endif // #if FFX_HALF == 1

/// Apply constant adaptive sharpening (CAS) filter to a single pixel.
/// 
/// @param [out] pixR                   Red channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [out] pixG                   Green channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [out] pixB                   Blue channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [in] samplePosition          The integer pixel position in the output.
/// @param [in] const0                  The first constant generated by <c><i>ffxCasSetup</i></c>.
/// @param [in] const1                  The second constant generated by <c><i>ffxCasSetup</i></c>.
/// @param [in] noScaling               Must be a compile-time literal value. A value of true applies sharpening only (no resizing).
/// 
/// @ingroup FfxGPUCas
void ffxCasFilter(
    FFX_PARAMETER_OUT FfxFloat32 pixR,
    FFX_PARAMETER_OUT FfxFloat32 pixG,
    FFX_PARAMETER_OUT FfxFloat32 pixB,
    FFX_PARAMETER_IN FfxUInt32x2 samplePosition,
    FFX_PARAMETER_IN FfxUInt32x4 const0,
    FFX_PARAMETER_IN FfxUInt32x4 const1,
    FFX_PARAMETER_IN FfxBoolean noScaling)
{
#if defined(FFX_CAS_DEBUG_CHECKER)
    // Debug a checker pattern of on/off tiles for visual inspection.
    if ((((samplePosition.x ^ samplePosition.y) >> 8u) & 1u) == 0u) {

        FfxFloat32x3 pix0 = casLoad(FfxInt32x2(samplePosition));
        pixR = pix0.r;
        pixG = pix0.g;
        pixB = pix0.b;
        casInput(pixR, pixG, pixB);
        return;
    }
#endif // #if defined(FFX_CAS_PACKED_ONLY)

    if (noScaling) {
        casFilterNoScaling(pixR, pixG, pixB, samplePosition, const0, const1);
    } else {
        casFilterWithScaling(pixR, pixG, pixB, samplePosition, const0, const1);
    }
}

#if FFX_HALF == 1
#if defined(FFX_HLSL)
#if !defined(FFX_CAS_USE_PRECISE_MATH)
// Missing a way to do packed re-interpetation, so must disable approximation optimizations.
#define FFX_CAS_USE_PRECISE_MATH        (1)
#endif // #if !defined(FFX_CAS_USE_PRECISE_MATH)
#endif // #if defined(FFX_HLSL)

/// A utility function which can be used to convert the packed SOA form results
/// returned by <c><i>ffxCasFilterHalf</i></c> into AOS form data ready for storing.
///
/// The implementation of both <c><i>ffxCasDepackHalf</i></c> and <c><i>ffxCasFilterHalf</i></c> assumes
/// that the pixels packed together are separated by 8 pixels in the X dimension.
///
/// It is suggested to only use <c><i>ffxCasDepack</i></c> right before stores. This is to maintain packed
/// math for any work after <c><i>ffxCasFilterHalf</i></c>.
///
/// An example might look as follows:
///     ffxCasFilterHalf(cR, cG, cB, gxy, const0, const1, false);
///     ...
///     ffxCasDepack(c0, c1, cR, cG, cB);
///     imageStore(imgDst, FfxInt32x2(gxy), FfxFloat4(c0));
///     imageStore(imgDst, FfxInt32x2(gxy) + FfxInt32x2(8, 0), FfxFloat4(c1));
///
/// @param [out] pix0                   
/// @param [out] pix1                   
/// @param [in] pixR                    The red channel components of two packed pixels.
/// @param [in] pixG                    The green channel components of two packed pixels.
/// @param [in] pixB                    The blue channel components of two packed pixels.
/// 
/// @ingroup FfxGPUCas
void ffxCasDepackHalf(
    FFX_PARAMETER_OUT FfxFloat16x4 pix0,
    FFX_PARAMETER_OUT FfxFloat16x4 pix1,
    FFX_PARAMETER_IN FfxFloat16x2 pixR,
    FFX_PARAMETER_IN FfxFloat16x2 pixG,
    FFX_PARAMETER_IN FfxFloat16x2 pixB)
{
#ifdef FFX_HLSL
    // Invoke a slower path for DX only, since it won't allow uninitialized values.
    pix0.a = pix1.a = 0.0;
#endif
    pix0.rgb = FfxFloat16x3(pixR.x, pixG.x, pixB.x);
    pix1.rgb = FfxFloat16x3(pixR.y, pixG.y, pixB.y);
}

/// Apply constant adaptive sharpening (CAS) filter to a pair of pixels.
/// 
/// Output values are for 2 separate 8x8 tiles in a 16x8 region.
///     pix<R,G,B>.x = right 8x8 tile
///     pix<R,G,B>.y =  left 8x8 tile
/// This enables later processing to easily be packed as well.
///
/// @param [out] pixR                   Red channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [out] pixG                   Green channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [out] pixB                   Blue channel output value. This is non-vector to enable switching between <c><i>ffxCasFilter</i></c> and <c><i>ffxCasFilterHalf</i></c>.
/// @param [in] samplePosition          The integer pixel position in the output.
/// @param [in] const0                  The first constant generated by <c><i>ffxCasSetup</i></c>.
/// @param [in] const1                  The second constant generated by <c><i>ffxCasSetup</i></c>.
/// @param [in] noScaling               Must be a compile-time literal value. A value of true applies sharpening only (no resizing).
/// 
/// @ingroup FfxGPUCas
void ffxCasFilterHalf(
   FFX_PARAMETER_OUT FfxFloat16x2 pixR,
   FFX_PARAMETER_OUT FfxFloat16x2 pixG,
   FFX_PARAMETER_OUT FfxFloat16x2 pixB,
   FFX_PARAMETER_IN FfxUInt32x2 samplePosition,
   FFX_PARAMETER_IN FfxUInt32x4 const0,
   FFX_PARAMETER_IN FfxUInt32x4 const1,
   FFX_PARAMETER_IN FfxBoolean noScaling)
{
#if defined(FFX_CAS_DEBUG_CHECKER)
    // Debug a checker pattern of on/off tiles for visual inspection. 
    if ((((samplePosition.x ^ samplePosition.y) >> 8u) & 1u) == 0u) {

        FfxFloat16x3 pix0 = casLoadHalf(FfxInt16x2(ip));
        FfxFloat16x3 pix1 = casLoadHalf(FfxInt16x2(ip) + FfxInt16x2(8, 0));
        pixR = FfxFloat16x2(pix0.r, pix1.r);
        pixG = FfxFloat16x2(pix0.g, pix1.g);
        pixB = FfxFloat16x2(pix0.b, pix1.b);
        casInputHalf(pixR, pixG, pixB);
        return;
    }
#endif // #if defined(FFX_CAS_PACKED_ONLY)

    // No scaling algorithm uses minimal 3x3 pixel neighborhood.
    if (noScaling) {
        casFilterNoScalingHalf(pixR, pixG, pixB, samplePosition, const0, const1);
    } else {
        casFilterWithScalingHalf(pixR, pixG, pixB, samplePosition, const0, const1);
    }
}
#endif // #if FFX_HALF == 1
#endif // #if defined(FFX_GPU)
