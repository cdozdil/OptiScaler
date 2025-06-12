/*********************************************************************************************************\
|*                                                                                                        *|
|* SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.  *|
|* SPDX-License-Identifier: MIT                                                                           *|
|*                                                                                                        *|
|* Permission is hereby granted, free of charge, to any person obtaining a                                *|
|* copy of this software and associated documentation files (the "Software"),                             *|
|* to deal in the Software without restriction, including without limitation                              *|
|* the rights to use, copy, modify, merge, publish, distribute, sublicense,                               *|
|* and/or sell copies of the Software, and to permit persons to whom the                                  *|
|* Software is furnished to do so, subject to the following conditions:                                   *|
|*                                                                                                        *|
|* The above copyright notice and this permission notice shall be included in                             *|
|* all copies or substantial portions of the Software.                                                    *|
|*                                                                                                        *|
|* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                             *|
|* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                               *|
|* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL                               *|
|* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                             *|
|* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING                                *|
|* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER                                    *|
|* DEALINGS IN THE SOFTWARE.                                                                              *|
|*                                                                                                        *|
|*                                                                                                        *|
\*********************************************************************************************************/

#pragma once
#include"nvapi_lite_salstart.h"
#include"nvapi_lite_common.h"
#pragma pack(push,8)
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
// DirectX APIs
//-----------------------------------------------------------------------------


//! \ingroup dx
//! Used in NvAPI_D3D10_GetCurrentSLIState(), and NvAPI_D3D_GetCurrentSLIState().
typedef struct
{
    NvU32 version;                    //!< Structure version
    NvU32 maxNumAFRGroups;            //!< [OUT] The maximum possible value of numAFRGroups
    NvU32 numAFRGroups;               //!< [OUT] The number of AFR groups enabled in the system
    NvU32 currentAFRIndex;            //!< [OUT] The AFR group index for the frame currently being rendered
    NvU32 nextFrameAFRIndex;          //!< [OUT] What the AFR group index will be for the next frame (i.e. after calling Present)
    NvU32 previousFrameAFRIndex;      //!< [OUT] The AFR group index that was used for the previous frame (~0 if more than one frame has not been rendered yet)
    NvU32 bIsCurAFRGroupNew;          //!< [OUT] Boolean: Is this frame the first time running on the current AFR group

} NV_GET_CURRENT_SLI_STATE_V1;

typedef struct
{
    NvU32 version;                    //!< Structure version
    NvU32 maxNumAFRGroups;            //!< [OUT] The maximum possible value of numAFRGroups
    NvU32 numAFRGroups;               //!< [OUT] The number of AFR groups enabled in the system
    NvU32 currentAFRIndex;            //!< [OUT] The AFR group index for the frame currently being rendered
    NvU32 nextFrameAFRIndex;          //!< [OUT] What the AFR group index will be for the next frame (i.e. after calling Present)
    NvU32 previousFrameAFRIndex;      //!< [OUT] The AFR group index that was used for the previous frame (~0 if more than one frame has not been rendered yet)
    NvU32 bIsCurAFRGroupNew;          //!< [OUT] Boolean: Is this frame the first time running on the current AFR group
    NvU32 numVRSLIGpus;               //!< [OUT] The number of GPUs used in VR-SLI. If it is 0 VR-SLI is not active

} NV_GET_CURRENT_SLI_STATE_V2;

//! \ingroup dx
#define NV_GET_CURRENT_SLI_STATE_VER1  MAKE_NVAPI_VERSION(NV_GET_CURRENT_SLI_STATE_V1,1)
#define NV_GET_CURRENT_SLI_STATE_VER2  MAKE_NVAPI_VERSION(NV_GET_CURRENT_SLI_STATE_V2,1)
#define NV_GET_CURRENT_SLI_STATE_VER NV_GET_CURRENT_SLI_STATE_VER2
#define NV_GET_CURRENT_SLI_STATE     NV_GET_CURRENT_SLI_STATE_V2
#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME:   NvAPI_D3D_GetCurrentSLIState
//
//! DESCRIPTION:     This function returns the current SLI state for the specified device.  The structure
//!                  contains the number of AFR groups, the current AFR group index,
//!                  and what the AFR group index will be for the next frame. \p
//!                  pDevice can be either a IDirect3DDevice9 or ID3D10Device pointer.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 173
//!
//! \retval         NVAPI_OK     Completed request
//! \retval         NVAPI_ERROR  Error occurred
//!
//! \ingroup  dx
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(IUnknown *pDevice, NV_GET_CURRENT_SLI_STATE *pSliState);
#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_SetResourceHint
//
//! \fn NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj,
//!                                          NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, 
//!                                          NvU32 dwHintName, 
//!                                          NvU32 *pdwHintValue)
//!
//!   DESCRIPTION: This is a general purpose function for passing down various resource
//!                related hints to the driver. Hints are divided into categories
//!                and types within each category. For DX11 devices this function is free-threaded.
//!                An application is responsible to complete this call before making use of the resource 
//!                in a rendering context (therefore applying inter-thread synchronization as appropriate).
//!                As a debug help to an application the driver enforces that a resource in this call was never bound.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 185
//!
//! \param [in]      pDev            The ID3D10Device or IDirect3DDevice9 that is a using the resource
//! \param [in]      obj             Previously obtained HV resource handle
//! \param [in]      dwHintCategory  Category of the hints
//! \param [in]      dwHintName      A hint within this category
//! \param [in/out]  *pdwHintValue   Pointer to location containing hint value, function returns previous hint value in this slot
//!
//! \return an int which could be an NvAPI status or DX HRESULT code
//!
//! \retval ::NVAPI_OK               
//! \retval ::NVAPI_INVALID_ARGUMENT  
//! \retval ::NVAPI_INVALID_CALL     It is illegal to change a hint dynamically when the resource is already bound.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//! \ingroup dx
//! Valid categories for NvAPI_D3D_SetResourceHint()
typedef enum _NVAPI_D3D_SETRESOURCEHINT_CATEGORY
{
    NVAPI_D3D_SRH_CATEGORY_SLI = 1
} NVAPI_D3D_SETRESOURCEHINT_CATEGORY;


//
//  NVAPI_D3D_SRH_SLI_APP_CONTROLLED_INTERFRAME_CONTENT_SYNC:
//  NVAPI_D3D_SRH_SLI_ASK_FOR_BROADCAST_USING:
//  NVAPI_D3D_SRH_SLI_RESPECT_DRIVER_INTERFRAME_CONTENT_SYNC:


//! \ingroup dx
//!  Types of SLI hints; \n
//!  NVAPI_D3D_SRH_SLI_APP_CONTROLLED_INTERFRAME_CONTENT_SYNC: Valid values : 0 or 1 \n
//!  Default value: 0 \n
//!  Explanation: If the value is 1, the driver will not track any rendering operations that would mark this resource as dirty, 
//!  avoiding any form of synchronization across frames rendered in parallel in multiple GPUs in AFR mode.
//! 
//!  NVAPI_D3D_SRH_SLI_ASK_FOR_BROADCAST_USAGE: Valid values : 0 or 1 \n
//!  Default value: 0 \n
//!  Explanation: If the value is 1, the driver will try to perform operations which involved target resource in broadcast, 
//!  where it's possible. Hint is static and must be set before resource starts using.
//!  
//!  NVAPI_D3D_SRH_SLI_RESPECT_DRIVER_INTERFRAME_CONTENT_SYNC: Valid values : 0 or 1 \n
//!  Default value: 0 \n
//!  Explanation: If the value is 1, the driver will do dirty resource resolve regardless of discard flags in the application profile or 
//!  AFR-FriendlyD3DHints.exe name using.
//!
typedef enum _NVAPI_D3D_SETRESOURCEHINT_SLI
{
    NVAPI_D3D_SRH_SLI_APP_CONTROLLED_INTERFRAME_CONTENT_SYNC = 1,
    NVAPI_D3D_SRH_SLI_ASK_FOR_BROADCAST_USAGE = 2,
    NVAPI_D3D_SRH_SLI_RESPECT_DRIVER_INTERFRAME_CONTENT_SYNC = 3
}  NVAPI_D3D_SETRESOURCEHINT_SLI;

//! \ingroup dx
NVAPI_INTERFACE NvAPI_D3D_SetResourceHint(IUnknown *pDev, NVDX_ObjectHandle obj,
                                          NVAPI_D3D_SETRESOURCEHINT_CATEGORY dwHintCategory, 
                                          NvU32 dwHintName, 
                                          NvU32 *pdwHintValue);
#endif //defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_BeginResourceRendering
//
//! \fn NvAPI_D3D_BeginResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags)
//!   DESCRIPTION: This function tells the driver that the resource will begin to receive updates. It must be used in combination with NvAPI_D3D_EndResourceRendering(). 
//!                The primary use of this function is allow the driver to initiate early inter-frame synchronization of resources while running in AFR SLI mode. 
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 185
//!
//! \param [in]  pDev         IDirect3DDevice9, ID3D10Device, ID3D11Device or ID3D11DeviceContext that is using the resource
//! \param [in]  obj          Previously obtained HV resource handle
//! \param [in]  Flags        The flags for functionality applied to resource while being used.
//!
//! \retval ::NVAPI_OK                Function succeeded, if used properly and driver can initiate proper sync'ing of the resources.
//! \retval ::NVAPI_INVALID_ARGUMENT  Bad argument(s) or invalid flag values
//! \retval ::NVAPI_INVALID_CALL      Mismatched begin/end calls
//
///////////////////////////////////////////////////////////////////////////////

//! \ingroup dx
//! Used in NvAPI_D3D_BeginResourceRendering().
typedef enum  _NVAPI_D3D_RESOURCERENDERING_FLAG
{
    NVAPI_D3D_RR_FLAG_DEFAULTS                 = 0x00000000,  //!< All bits set to 0 are defaults.
    NVAPI_D3D_RR_FLAG_FORCE_DISCARD_CONTENT    = 0x00000001,  //!< (bit 0) The flag forces to discard previous content of the resource regardless of the NvApiHints_Sli_Disable_InterframeSync hint
    NVAPI_D3D_RR_FLAG_FORCE_KEEP_CONTENT       = 0x00000002,   //!< (bit 1) The flag forces to respect previous content of the resource regardless of the NvApiHints_Sli_Disable_InterframeSync hint
    NVAPI_D3D_RR_FLAG_MULTI_FRAME              = 0x00000004   //!< (bit 2) The flag hints the driver that content will be used for many frames. If not specified then the driver assumes that content is used only on the next frame
} NVAPI_D3D_RESOURCERENDERING_FLAG;

//! \ingroup dx
NVAPI_INTERFACE NvAPI_D3D_BeginResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags);

#endif //defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_EndResourceRendering
//
//!   DESCRIPTION: This function tells the driver that the resource is done receiving updates. It must be used in combination with
//!                NvAPI_D3D_BeginResourceRendering(). 
//!                The primary use of this function is allow the driver to initiate early inter-frame syncs of resources while running in AFR SLI mode. 
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 185
//!
//! \param [in]  pDev         IDirect3DDevice9, ID3D10Device, ID3D11Device or ID3D11DeviceContext that is using the resource
//! \param [in]  obj          Previously obtained HV resource handle
//! \param [in]  Flags        Reserved, must be zero
//
//! \retval ::NVAPI_OK                Function succeeded, if used properly and driver can initiate proper sync'ing of the resources.
//! \retval ::NVAPI_INVALID_ARGUMENT  Bad argument(s) or invalid flag values
//! \retval ::NVAPI_INVALID_CALL      Mismatched begin/end calls
//!
//! \ingroup dx
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_EndResourceRendering(IUnknown *pDeviceOrContext, NVDX_ObjectHandle obj, NvU32 Flags);
#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)

#include"nvapi_lite_salend.h"
#ifdef __cplusplus
}
#endif
#pragma pack(pop)
