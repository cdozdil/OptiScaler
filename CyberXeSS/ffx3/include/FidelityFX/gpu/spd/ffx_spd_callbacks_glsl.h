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


#include "ffx_spd_resources.h"

#if defined(FFX_GPU)
#include "ffx_core.h"
#endif // #if defined(FFX_GPU)

#if defined(FFX_GPU)
#ifndef FFX_PREFER_WAVE64
#define FFX_PREFER_WAVE64
#endif // #if defined(FFX_GPU)

#if defined(FFX_SPD_BIND_CB_SPD)
    layout (set = 0, binding = FFX_SPD_BIND_CB_SPD, std140) uniform cbFSR1_t
    {
        FfxUInt32       mips;
        FfxUInt32       numWorkGroups;
        FfxUInt32x2     workGroupOffset;
        FfxFloat32x2    invInputSize;       // Only used for linear sampling mode
        FfxFloat32x2    padding;
    } cbFSR1;
#endif


FfxUInt32 Mips()
{
    return cbFSR1.mips;
}

FfxUInt32 NumWorkGroups()
{
    return cbFSR1.numWorkGroups;
}

FfxUInt32x2  WorkGroupOffset()
{
    return cbFSR1.workGroupOffset;
}

FfxFloat32x2 InvInputSize()
{
    return cbFSR1.invInputSize;
}

layout (set = 0, binding = 1000) uniform sampler s_LinearClamp;

// SRVs
#if defined FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC
    layout (set = 0, binding = FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC)  uniform texture2DArray  r_input_downsample_src;
#endif

// UAV declarations
#if defined FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC
    layout (set = 0, binding = FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC, std430)            coherent buffer                rw_internal_global_atomic_t
    {
        FfxUInt32 counter[6];
    } rw_internal_global_atomic;
#endif

#if defined FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP
    layout (set = 0, binding = FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP, rgba32f)  coherent uniform image2DArray  rw_input_downsample_src_mid_mip;
#endif

#if defined FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS
    layout (set = 0, binding = FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS, rgba32f)                 uniform image2DArray  rw_input_downsample_src_mips[SPD_MAX_MIP_LEVELS+1];
#endif

#if FFX_HALF

    FfxFloat16x4 SampleSrcImageH(FfxFloat32x2 uv, FfxUInt32 slice)
    {
        FfxFloat32x2 textureCoord = FfxFloat32x2(uv) * InvInputSize() + InvInputSize();
#if defined(FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC)
        FfxFloat32x4 result = textureLod(sampler2DArray(r_input_downsample_src, s_LinearClamp), FfxFloat32x3(textureCoord, slice), 0);
        return FfxFloat16x4(ffxSrgbToLinear(result.x), ffxSrgbToLinear(result.y), ffxSrgbToLinear(result.z), result.w);
#endif // defined(FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC)
        return FfxFloat16x4(0.f);
    }

    FfxFloat16x4 LoadSrcImageH(FfxFloat32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
        return FfxFloat16x4(imageLoad(rw_input_downsample_src_mips[0], FfxInt32x3(uv, slice)));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)

        return FfxFloat16x4(0.f);
    }

    void StoreSrcMipH(FfxFloat16x4 value, FfxInt32x2 uv, FfxUInt32 slice, FfxUInt32 mip)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
        imageStore(rw_input_downsample_src_mips[mip], FfxInt32x3(uv, slice), FfxFloat32x4(value));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
    }

    FfxFloat16x4 LoadMidMipH(FfxInt32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        return FfxFloat16x4(imageLoad(rw_input_downsample_src_mid_mip, FfxInt32x3(uv, slice)));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        return FfxFloat16x4(0.f);
    }

    void StoreMidMipH(FfxFloat16x4 value, FfxInt32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        imageStore(rw_input_downsample_src_mid_mip, FfxInt32x3(uv, slice), FfxFloat32x4(value));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
    }

#else // FFX_HALF

    FfxFloat32x4 SampleSrcImage(FfxInt32x2 uv, FfxUInt32 slice)
    {
        FfxFloat32x2 textureCoord = FfxFloat32x2(uv) * InvInputSize() + InvInputSize();
#if defined(FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC)
        FfxFloat32x4 result = textureLod(sampler2DArray(r_input_downsample_src, s_LinearClamp), FfxFloat32x3(textureCoord, slice), 0);
        return FfxFloat32x4(ffxSrgbToLinear(result.x), ffxSrgbToLinear(result.y), ffxSrgbToLinear(result.z), result.w);
#endif // defined(FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC)
        return FfxFloat32x4(0.f);
    }

    FfxFloat32x4 LoadSrcImage(FfxInt32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
        return imageLoad(rw_input_downsample_src_mips[0], FfxInt32x3(uv, slice));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)

        return FfxFloat32x4(0.f);
    }

    void StoreSrcMip(FfxFloat32x4 value, FfxInt32x2 uv, FfxUInt32 slice, FfxUInt32 mip)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
        imageStore(rw_input_downsample_src_mips[mip], FfxInt32x3(uv, slice), value);
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS)
    }

    FfxFloat32x4 LoadMidMip(FfxInt32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        return imageLoad(rw_input_downsample_src_mid_mip, FfxInt32x3(uv, slice));
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        return FfxFloat32x4(0.f);
    }

    void StoreMidMip(FfxFloat32x4 value, FfxInt32x2 uv, FfxUInt32 slice)
    {
#if defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
        imageStore(rw_input_downsample_src_mid_mip, FfxInt32x3(uv, slice), value);
#endif // defined(FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP)
    }

#endif // FFX_HALF

void IncreaseAtomicCounter(FFX_PARAMETER_IN FfxUInt32 slice, FFX_PARAMETER_INOUT FfxUInt32 counter)
{
#if defined(FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC)
    counter = atomicAdd(rw_internal_global_atomic.counter[slice], 1);
#endif // defined(FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC)
}

void ResetAtomicCounter(FFX_PARAMETER_IN FfxUInt32 slice)
{
#if defined(FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC)
    rw_internal_global_atomic.counter[slice] = 0;
#endif // defined(FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC)
}

#endif // #if defined(FFX_GPU)
