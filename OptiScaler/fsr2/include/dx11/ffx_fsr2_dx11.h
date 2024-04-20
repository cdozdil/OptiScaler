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

// @defgroup DX11

#pragma once

#include <d3d11.h>
#include "../ffx_fsr2_interface.h"

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)

/// Query how much memory is required for the DirectX 11 backend's scratch buffer.
///
/// @returns
/// The size (in bytes) of the required scratch memory buffer for the DX11 backend.
size_t ffxFsr2GetScratchMemorySizeDX11();

/// Populate an interface with pointers for the DX11 backend.
///
/// @param [out] fsr2Interface              A pointer to a <c><i>FfxFsr2Interface</i></c> structure to populate with pointers.
/// @param [in] device                      A pointer to the DirectX11 device.
/// @param [in] scratchBuffer               A pointer to a buffer of memory which can be used by the DirectX(R)11 backend.
/// @param [in] scratchBufferSize           The size (in bytes) of the buffer pointed to by <c><i>scratchBuffer</i></c>.
/// 
/// @retval
/// FFX_OK                                  The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_INVALID_POINTER          The <c><i>interface</i></c> pointer was <c><i>NULL</i></c>.
/// 
/// @ingroup FSR2 DX11
FfxErrorCode ffxFsr2GetInterfaceDX11(
    FfxFsr2Interface* fsr2Interface,
    ID3D11Device* device,
    void* scratchBuffer,
    size_t scratchBufferSize);

/// Create a <c><i>FfxFsr2Device</i></c> from a <c><i>ID3D11Device</i></c>.
///
/// @param [in] device                      A pointer to the DirectX11 device.
/// 
/// @returns
/// An abstract FidelityFX device.
/// 
/// @ingroup FSR2 DX11
FfxDevice ffxGetDeviceDX11(ID3D11Device* device);

/// Create a <c><i>FfxResource</i></c> from a <c><i>ID3D11Resource</i></c>.
///
/// @param [in] fsr2Interface               A pointer to a <c><i>FfxFsr2Interface</i></c> structure.
/// @param [in] resDx11                     A pointer to the DirectX11 resource.
/// @param [in] name                        (optional) A name string to identify the resource in debug mode.
/// @param [in] state                       The state the resource is currently in.
/// 
/// @returns
/// An abstract FidelityFX resources.
/// 
/// @ingroup FSR2 DX11
FfxResource ffxGetResourceDX11(
    FfxFsr2Context* context,
    ID3D11Resource* resDx11,
    const wchar_t* name = nullptr,
    FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ);

/// Retrieve a <c><i>ID3D11Resource</i></c> pointer associated with a RESOURCE_IDENTIFIER.
/// Used for debug purposes when blitting internal surfaces.
///
/// @param [in] context                     A pointer to a <c><i>FfxFsr2Context</i></c> structure.
/// @param [in] resId                       A resource.
/// 
/// @returns
/// A <c><i>ID3D11Resource</i> pointer</c>.
/// 
/// @ingroup FSR2 DX11
ID3D11Resource* ffxGetDX11ResourcePtr(FfxFsr2Context* context, uint32_t resId);

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)
