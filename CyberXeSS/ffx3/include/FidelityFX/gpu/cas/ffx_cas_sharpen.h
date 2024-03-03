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


#include "ffx_core.h"

#if FFX_HALF

#define FFX_CAS_PACKED_ONLY 1

#endif  // FFX_HALF

#include "cas/ffx_cas.h"

void Sharpen(FfxUInt32x3 LocalThreadId, FfxUInt32x3 WorkGroupId, FfxUInt32x3 Dtid)
{
    // Do remapping of local xy in workgroup for a more PS-like swizzle pattern.
    FfxUInt32x2 gxy = ffxRemapForQuad(LocalThreadId.x) + FfxUInt32x2(WorkGroupId.x << 4u, WorkGroupId.y << 4u);

    FfxBoolean sharpenOnly;
#if FFX_CAS_OPTION_SHARPEN_ONLY
    sharpenOnly = true;
#else
    sharpenOnly = false;
#endif  // FFX_CAS_OPTION_SHARPEN_ONLY

#if FFX_HALF

    // Filter.
    FfxFloat16x4 c0, c1;
    FfxFloat16x2 cR, cG, cB;

    ffxCasFilterHalf(cR, cG, cB, gxy, Const0(), Const1(), sharpenOnly);
    casOutputHalf(cR, cG, cB);
    ffxCasDepackHalf(c0, c1, cR, cG, cB);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c0));
    casStoreOutput(FfxInt32x2(gxy) + FfxInt32x2(8, 0), FfxFloat32x4(c1));
    gxy.y += 8u;

    ffxCasFilterHalf(cR, cG, cB, gxy, Const0(), Const1(), sharpenOnly);
    casOutputHalf(cR, cG, cB);
    ffxCasDepackHalf(c0, c1, cR, cG, cB);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c0));
    casStoreOutput(FfxInt32x2(gxy) + FfxInt32x2(8, 0), FfxFloat32x4(c1));

#else

    // Filter.
    FfxFloat32x3 c;

    ffxCasFilter(c.r, c.g, c.b, gxy, Const0(), Const1(), sharpenOnly);
    casOutput(c.r, c.g, c.b);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c, 1));
    gxy.x += 8u;

    ffxCasFilter(c.r, c.g, c.b, gxy, Const0(), Const1(), sharpenOnly);
    casOutput(c.r, c.g, c.b);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c, 1));
    gxy.y += 8u;

    ffxCasFilter(c.r, c.g, c.b, gxy, Const0(), Const1(), sharpenOnly);
    casOutput(c.r, c.g, c.b);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c, 1));
    gxy.x -= 8u;

    ffxCasFilter(c.r, c.g, c.b, gxy, Const0(), Const1(), sharpenOnly);
    casOutput(c.r, c.g, c.b);
    casStoreOutput(FfxInt32x2(gxy), FfxFloat32x4(c, 1));

#endif  // FFX_HALF
}
