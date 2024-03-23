// This file is part of the FidelityFX SDK.
//
// Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
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

// This file contains function declarations to convert DX12 resources 
// to and from API independent FFX resources.

// @defgroup DX12

#pragma once

#include <d3d12.h>
#include "../ffx_fsr2_interface.h"

namespace Fsr212
{


#if defined(__cplusplus)
    extern "C" {
#endif // #if defined(__cplusplus)

        /// Query how much memory is required for the DirectX 12 backend's scratch buffer.
        ///
        /// @returns
        /// The size (in bytes) of the required scratch memory buffer for the DX12 backend.
        size_t ffxFsr2GetScratchMemorySizeDX12_212();

        /// Populate an interface with pointers for the DX12 backend.
        ///
        /// @param [out] fsr2Interface              A pointer to a <c><i>FfxFsr2Interface</i></c> structure to populate with pointers.
        /// @param [in] device                      A pointer to the DirectX12 device.
        /// @param [in] scratchBuffer               A pointer to a buffer of memory which can be used by the DirectX(R)12 backend.
        /// @param [in] scratchBufferSize           The size (in bytes) of the buffer pointed to by <c><i>scratchBuffer</i></c>.
        /// 
        /// @retval
        /// FFX_OK                                  The operation completed successfully.
        /// @retval
        /// FFX_ERROR_CODE_INVALID_POINTER          The <c><i>interface</i></c> pointer was <c><i>NULL</i></c>.
        /// 
        /// @ingroup FSR2 DX12
        FfxErrorCode ffxFsr2GetInterfaceDX12_212(
            FfxFsr2Interface212* fsr2Interface,
            ID3D12Device* device,
            void* scratchBuffer,
            size_t scratchBufferSize);

        /// Create a <c><i>FfxFsr2Device</i></c> from a <c><i>ID3D12Device</i></c>.
        ///
        /// @param [in] device                      A pointer to the DirectX12 device.
        /// 
        /// @returns
        /// An abstract FidelityFX device.
        /// 
        /// @ingroup FSR2 DX12
        FfxDevice ffxGetDeviceDX12_212(ID3D12Device* device);

        /// Create a <c><i>FfxCommandList</i></c> from a <c><i>ID3D12CommandList</i></c>.
        ///
        /// @param [in] cmdList                     A pointer to the DirectX12 command list.
        /// 
        /// @returns
        /// An abstract FidelityFX command list.
        /// 
        /// @ingroup FSR2 DX12
        FfxCommandList ffxGetCommandListDX12_212(ID3D12CommandList* cmdList);

        /// Create a <c><i>FfxResource</i></c> from a <c><i>ID3D12Resource</i></c>.
        ///
        /// @param [in] fsr2Interface               A pointer to a <c><i>FfxFsr2Interface</i></c> structure.
        /// @param [in] resDx12                     A pointer to the DirectX12 resource.
        /// @param [in] name                        (optional) A name string to identify the resource in debug mode.
        /// @param [in] state                       The state the resource is currently in.
        /// @param [in] shaderComponentMapping      The shader component mapping.
        /// 
        /// @returns
        /// An abstract FidelityFX resources.
        /// 
        /// @ingroup FSR2 DX12
        FfxResource ffxGetResourceDX12_212(
            FfxFsr2Context* context,
            ID3D12Resource* resDx12,
            const wchar_t* name = nullptr,
            FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ,
            UINT shaderComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING);

        /// Retrieve a <c><i>ID3D12Resource</i></c> pointer associated with a UAV RESOURCE_IDENTIFIER.
        /// Used for debug purposes when blitting internal surfaces.
        ///
        /// @param [in] context                     A pointer to a <c><i>FfxFsr2Context</i></c> structure.
        /// @param [in] resId                       A resourceID.
        /// 
        /// @returns
        /// A <c><i>ID3D12Resource</i> pointer</c>.
        /// 
        /// @ingroup FSR2 DX12
        ID3D12Resource* ffxGetDX12ResourcePtr_212(FfxFsr2Context* context, uint32_t resId);

#if defined(__cplusplus)
    }
#endif // #if defined(__cplusplus)

}