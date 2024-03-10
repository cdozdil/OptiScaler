// This file is part of the FidelityFX SDK.
//
// Copyright © 2023 Advanced Micro Devices, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files(the “Software”), to deal 
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
// THE SOFTWARE.

/// @defgroup DX11Backend DX11 Backend
/// FidelityFX SDK native backend implementation for DirectX 11.
/// 
/// @ingroup Backends

#pragma once

#include <d3d11_1.h>
#include <host/ffx_interface.h>

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)

/// Query how much memory is required for the DirectX 11 backend's scratch buffer.
/// 
/// @param [in] maxContexts                 The maximum number of simultaneous effect contexts that will share the backend.
///                                         (Note that some effects contain internal contexts which count towards this maximum)
///
/// @returns
/// The size (in bytes) of the required scratch memory buffer for the DX11 backend.
/// @ingroup DX11Backend
FFX_API size_t ffxGetScratchMemorySizeDX11(size_t maxContexts);

/// Create a <c><i>FfxDevice</i></c> from a <c><i>ID3D11Device</i></c>.
///
/// @param [in] device                      A pointer to the DirectX11 device.
///
/// @returns
/// An abstract FidelityFX device.
///
/// @ingroup DX11Backend
FFX_API FfxDevice ffxGetDeviceDX11(ID3D11Device* device);

/// Populate an interface with pointers for the DX11 backend.
///
/// @param [out] backendInterface           A pointer to a <c><i>FfxInterface</i></c> structure to populate with pointers.
/// @param [in] device                      A pointer to the DirectX11 device.
/// @param [in] scratchBuffer               A pointer to a buffer of memory which can be used by the DirectX(R)11 backend.
/// @param [in] scratchBufferSize           The size (in bytes) of the buffer pointed to by <c><i>scratchBuffer</i></c>.
/// @param [in] maxContexts                 The maximum number of simultaneous effect contexts that will share the backend.
///                                         (Note that some effects contain internal contexts which count towards this maximum)
///
/// @retval
/// FFX_OK                                  The operation completed successfully.
/// @retval
/// FFX_ERROR_CODE_INVALID_POINTER          The <c><i>interface</i></c> pointer was <c><i>NULL</i></c>.
///
/// @ingroup DX11Backend
FFX_API FfxErrorCode ffxGetInterfaceDX11(
    FfxInterface* backendInterface,
    FfxDevice device,
    void* scratchBuffer,
    size_t scratchBufferSize, 
    uint32_t maxContexts);

/// Create a <c><i>FfxCommandList</i></c> from a <c><i>ID3D11DeviceContext</i></c>.
///
/// @param [in] deviceContext               A pointer to the DirectX11 device context.
///
/// @returns
/// An abstract FidelityFX command list.
///
/// @ingroup DX11Backend
FFX_API FfxCommandList ffxGetCommandListDX11(ID3D11DeviceContext* deviceContext);

/// Fetch a <c><i>FfxResource</i></c> from a <c><i>GPUResource</i></c>.
///
/// @param [in] dx11Resource                A pointer to the DX11 resource.
/// @param [in] ffxResDescription           An <c><i>FfxResourceDescription</i></c> for the resource representation.
/// @param [in] ffxResName                  (optional) A name string to identify the resource in debug mode.
/// @param [in] state                       The state the resource is currently in.
///
/// @returns
/// An abstract FidelityFX resources.
///
/// @ingroup DX11Backend
FFX_API FfxResource ffxGetResourceDX11(const ID3D11Resource* dx11Resource,
    FfxResourceDescription       ffxResDescription,
    wchar_t const*               ffxResName,
    FfxResourceStates            state = FFX_RESOURCE_STATE_COMPUTE_READ);

FFX_API FfxSurfaceFormat ffxGetSurfaceFormatDX11(DXGI_FORMAT format);

FFX_API FfxResourceDescription GetFfxResourceDescriptionDX11(ID3D11Resource* pResource);

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)
