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
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Enable
//
//! DESCRIPTION:   This APU enables stereo mode in the registry.
//!                Calls to this function affect the entire system.
//!                If stereo is not enabled, then calls to functions that require that stereo is enabled have no effect,
//!                and will return the appropriate error code.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \retval ::NVAPI_OK                      Stereo is now enabled.
//! \retval ::NVAPI_API_NOT_INTIALIZED 
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED  Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Enable(void);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Disable
//
//! DESCRIPTION:   This API disables stereo mode in the registry.
//!                Calls to this function affect the entire system.
//!                If stereo is not enabled, then calls to functions that require that stereo is enabled have no effect,
//!                and will return the appropriate error code.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \retval ::NVAPI_OK                     Stereo is now disabled.
//! \retval ::NVAPI_API_NOT_INTIALIZED  
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi 
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Disable(void);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsEnabled
//
//! DESCRIPTION:   This API checks if stereo mode is enabled in the registry.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [out]     pIsStereoEnabled   Address where the result of the inquiry will be placed.
//!
//! \retval ::NVAPI_OK                       Check was sucessfully completed and result reflects current state of stereo availability.
//! \retval ::NVAPI_API_NOT_INTIALIZED 
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED   Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsEnabled(NvU8 *pIsStereoEnabled);
#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__)|| defined(__d3d12_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_CreateHandleFromIUnknown
//
//! DESCRIPTION:   This API creates a stereo handle that is used in subsequent calls related to a given device interface.
//!                This must be called before any other NvAPI_Stereo_ function for that handle.
//!                Multiple devices can be used at one time using multiple calls to this function (one per each device). 
//!
//! HOW TO USE:    After the Direct3D device is created, create the stereo handle.
//!                On call success:
//!                -# Use all other NvAPI_Stereo_ functions that have stereo handle as first parameter.
//!                -# After the device interface that corresponds to the the stereo handle is destroyed,
//!                the application should call NvAPI_DestroyStereoHandle() for that stereo handle. 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     pDevice        Pointer to IUnknown interface that is IDirect3DDevice9* in DX9, ID3D10Device*.
//! \param [out]    pStereoHandle  Pointer to the newly created stereo handle.
//!
//! \retval ::NVAPI_OK                       Stereo handle is created for given device interface.
//! \retval ::NVAPI_INVALID_ARGUMENT         Provided device interface is invalid.
//! \retval ::NVAPI_API_NOT_INTIALIZED  
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED   Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_CreateHandleFromIUnknown(IUnknown *pDevice, StereoHandle *pStereoHandle);

#endif // defined(_D3D9_H_) || defined(__d3d10_h__)
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_DestroyHandle
//
//! DESCRIPTION:   This API destroys the stereo handle created with one of the NvAPI_Stereo_CreateHandleFrom() functions.
//!                This should be called after the device corresponding to the handle has been destroyed.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle  Stereo handle that is to be destroyed.
//!
//! \retval ::NVAPI_OK                      Stereo handle is destroyed.
//! \retval ::NVAPI_API_NOT_INTIALIZED      
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED  Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR                   
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_DestroyHandle(StereoHandle stereoHandle);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Activate
//
//! DESCRIPTION:   This API activates stereo for the device interface corresponding to the given stereo handle.
//!                Activating stereo is possible only if stereo was enabled previously in the registry.
//!                If stereo is not activated, then calls to functions that require that stereo is activated have no effect,
//!                and will return the appropriate error code. 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]    stereoHandle  Stereo handle corresponding to the device interface.
//!
//! \retval ::NVAPI_OK                                Stereo is turned on.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE   Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED 
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED            Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Activate(StereoHandle stereoHandle);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Deactivate
//
//! DESCRIPTION:   This API deactivates stereo for the given device interface.
//!                If stereo is not activated, then calls to functions that require that stereo is activated have no effect,
//!                and will return the appropriate error code. 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle  Stereo handle that corresponds to the device interface.
//!
//! \retval ::NVAPI_OK                               Stereo is turned off.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE  Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED 
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED           Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Deactivate(StereoHandle stereoHandle);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsActivated
//
//! DESCRIPTION:   This API checks if stereo is activated for the given device interface. 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]    stereoHandle  Stereo handle that corresponds to the device interface.
//! \param [in]    pIsStereoOn   Address where result of the inquiry will be placed.
//! 
//! \retval ::NVAPI_OK - Check was sucessfully completed and result reflects current state of stereo (on/off).
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR - Something is wrong (generic error).
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsActivated(StereoHandle stereoHandle, NvU8 *pIsStereoOn);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetSeparation
//
//! DESCRIPTION:   This API gets current separation value (in percents). 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle           Stereo handle that corresponds to the device interface.
//! \param [out]    pSeparationPercentage  Address of @c float type variable to store current separation percentage in.
//!
//! \retval ::NVAPI_OK                                Retrieval of separation percentage was successfull.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE   Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED  
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED            Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR  
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetSeparation(StereoHandle stereoHandle, float *pSeparationPercentage);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetSeparation
//
//! DESCRIPTION:   This API sets separation to given percentage. 
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle             Stereo handle that corresponds to the device interface.
//! \param [in]     newSeparationPercentage  New value for separation percentage.
//!
//! \retval ::NVAPI_OK                               Setting of separation percentage was successfull.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE  Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED               NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED           Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_STEREO_PARAMETER_OUT_OF_RANGE    Given separation percentage is out of [0..100] range.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetSeparation(StereoHandle stereoHandle, float newSeparationPercentage);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetConvergence
//
//! DESCRIPTION:   This API gets the current convergence value.
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle   Stereo handle that corresponds to the device interface.
//! \param [out]    pConvergence   Address of @c float type variable to store current convergence value in.
//!
//! \retval ::NVAPI_OK                               Retrieval of convergence value was successfull.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE  Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED  
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED           Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetConvergence(StereoHandle stereoHandle, float *pConvergence);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetConvergence
//
//! DESCRIPTION:   This API sets convergence to the given value.
//!
//! WHEN TO USE:   After the stereo handle for the device interface is created via successfull call to the appropriate NvAPI_Stereo_CreateHandleFrom() function.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 180
//!
//! \param [in]     stereoHandle              Stereo handle that corresponds to the device interface.
//! \param [in]     newConvergence            New value for convergence.
//! 
//! \retval ::NVAPI_OK                                Setting of convergence value was successfull.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE   Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED  
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED            Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR 
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetConvergence(StereoHandle stereoHandle, float newConvergence);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetActiveEye
//
//! \fn NvAPI_Stereo_SetActiveEye(StereoHandle hStereoHandle, NV_STEREO_ACTIVE_EYE StereoEye);
//! DESCRIPTION:   This API sets the back buffer to left or right in Direct stereo mode.
//!                  
//! HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate 
//!                NvAPI_Stereo_CreateHandleFrom function.
//!
//! \since Release: 285
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \param [in]   stereoHandle  Stereo handle that corresponds to the device interface.
//! \param [in]   StereoEye     Defines active eye in Direct stereo mode
//!
//! \retval ::NVAPI_OK - Active eye is set.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE - Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED - NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED - Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_INVALID_ARGUMENT - StereoEye parameter has not allowed value.
//! \retval ::NVAPI_SET_NOT_ALLOWED  - Current stereo mode is not Direct
//! \retval ::NVAPI_ERROR - Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

//! \ingroup stereoapi
typedef enum _NV_StereoActiveEye
{
    NVAPI_STEREO_EYE_RIGHT = 1,
    NVAPI_STEREO_EYE_LEFT = 2,
    NVAPI_STEREO_EYE_MONO = 3,
} NV_STEREO_ACTIVE_EYE;

//! \ingroup stereoapi
NVAPI_INTERFACE NvAPI_Stereo_SetActiveEye(StereoHandle hStereoHandle, NV_STEREO_ACTIVE_EYE StereoEye);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetDriverMode
//
//! \fn NvAPI_Stereo_SetDriverMode( NV_STEREO_DRIVER_MODE mode );
//! DESCRIPTION:   This API sets the 3D stereo driver mode: Direct or Automatic
//!                  
//! HOW TO USE:    This API must be called before the device is created.
//!                Applies to DirectX 9 and higher.
//!
//! \since Release: 285
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!      
//! \param [in]    mode       Defines the 3D stereo driver mode: Direct or Automatic
//!
//! \retval ::NVAPI_OK                      Active eye is set.
//! \retval ::NVAPI_API_NOT_INTIALIZED      NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED  Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_INVALID_ARGUMENT        mode parameter has not allowed value.
//! \retval ::NVAPI_ERROR                   Something is wrong (generic error).
//
///////////////////////////////////////////////////////////////////////////////

//! \ingroup stereoapi
typedef enum _NV_StereoDriverMode
{
    NVAPI_STEREO_DRIVER_MODE_AUTOMATIC = 0,
    NVAPI_STEREO_DRIVER_MODE_DIRECT    = 2,
} NV_STEREO_DRIVER_MODE;

//! \ingroup stereoapi
NVAPI_INTERFACE NvAPI_Stereo_SetDriverMode( NV_STEREO_DRIVER_MODE mode );

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetEyeSeparation
//
//! DESCRIPTION:   This API returns eye separation as a ratio of <between eye distance>/<physical screen width>.
//! 
//! HOW TO USE:    After the stereo handle for device interface is created via successfull call to appropriate API. Applies only to DirectX 9 and up.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \param [in]   stereoHandle  Stereo handle that corresponds to the device interface.
//! \param [out]  pSeparation   Eye separation.
//!
//! \retval ::NVAPI_OK                               Active eye is set.
//! \retval ::NVAPI_STEREO_INVALID_DEVICE_INTERFACE  Device interface is not valid. Create again, then attach again.
//! \retval ::NVAPI_API_NOT_INTIALIZED               NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED           Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR  (generic error).
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetEyeSeparation(StereoHandle hStereoHandle,  float *pSeparation );
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_IsWindowedModeSupported
//
//! DESCRIPTION:   This API returns availability of windowed mode stereo
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \param [out] bSupported(OUT)    != 0  - supported,  \n
//!                                 == 0  - is not supported 
//!
//!
//! \retval ::NVAPI_OK                      Retrieval of frustum adjust mode was successfull.
//! \retval ::NVAPI_API_NOT_INTIALIZED      NVAPI not initialized.
//! \retval ::NVAPI_STEREO_NOT_INITIALIZED  Stereo part of NVAPI not initialized.
//! \retval ::NVAPI_ERROR                   Something is wrong (generic error).
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_IsWindowedModeSupported(NvU8* bSupported);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetSurfaceCreationMode
//
//! \function NvAPI_Stereo_SetSurfaceCreationMode(StereoHandle hStereoHandle, NVAPI_STEREO_SURFACECREATEMODE creationMode)
//! \param [in]   hStereoHandle   Stereo handle that corresponds to the device interface.
//! \param [in]   creationMode    New surface creation mode for this device interface.
//!
//! \since Release: 285
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! DESCRIPTION: This API sets surface creation mode for this device interface.
//!
//! WHEN TO USE: After the stereo handle for device interface is created via successful call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//!
//! \return      This API can return any of the error codes enumerated in #NvAPI_Status. 
//!              There are no return error codes with specific meaning for this API.
//!
///////////////////////////////////////////////////////////////////////////////

//! \ingroup stereoapi
typedef enum _NVAPI_STEREO_SURFACECREATEMODE 
{
    NVAPI_STEREO_SURFACECREATEMODE_AUTO,        //!< Use driver registry profile settings for surface creation mode. 
    NVAPI_STEREO_SURFACECREATEMODE_FORCESTEREO, //!< Always create stereo surfaces. 
    NVAPI_STEREO_SURFACECREATEMODE_FORCEMONO    //!< Always create mono surfaces. 
} NVAPI_STEREO_SURFACECREATEMODE; 

//! \ingroup stereoapi
NVAPI_INTERFACE NvAPI_Stereo_SetSurfaceCreationMode(__in StereoHandle hStereoHandle, __in NVAPI_STEREO_SURFACECREATEMODE creationMode);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetSurfaceCreationMode
//
//! \function NvAPI_Stereo_GetSurfaceCreationMode(StereoHandle hStereoHandle, NVAPI_STEREO_SURFACECREATEMODE* pCreationMode)
//! \param [in]   hStereoHandle   Stereo handle that corresponds to the device interface.
//! \param [out]   pCreationMode   The current creation mode for this device interface.
//!
//! \since Release: 295
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! DESCRIPTION: This API gets surface creation mode for this device interface.
//!
//! WHEN TO USE: After the stereo handle for device interface is created via successful call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//!
//! \return      This API can return any of the error codes enumerated in #NvAPI_Status. 
//!              There are no return error codes with specific meaning for this API.
//!
///////////////////////////////////////////////////////////////////////////////

//! \ingroup stereoapi
NVAPI_INTERFACE NvAPI_Stereo_GetSurfaceCreationMode(__in StereoHandle hStereoHandle, __in NVAPI_STEREO_SURFACECREATEMODE* pCreationMode);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_Debug_WasLastDrawStereoized
//
//! \param [in]  hStereoHandle    Stereo handle that corresponds to the device interface.
//! \param [out] pWasStereoized   Address where result of the inquiry will be placed.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! DESCRIPTION: This API checks if the last draw call was stereoized. It is a very expensive to call and should be used for debugging purpose *only*.
//!
//! WHEN TO USE: After the stereo handle for device interface is created via successful call to appropriate NvAPI_Stereo_CreateHandleFrom function.
//!
//! \return      This API can return any of the error codes enumerated in #NvAPI_Status. 
//!              There are no return error codes with specific meaning for this API.
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_Debug_WasLastDrawStereoized(__in StereoHandle hStereoHandle, __out NvU8 *pWasStereoized);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_SetDefaultProfile
//
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! DESCRIPTION: This API defines the stereo profile used by the driver in case the application has no associated profile.
//!
//! WHEN TO USE: To take effect, this API must be called before D3D device is created. Calling once a device has been created will not affect the current device.
//!
//! \param [in]  szProfileName        Default profile name. 
//!                                 
//! \return      This API can return any of the error codes enumerated in #NvAPI_Status. 
//!              Error codes specific to this API are described below.
//!              
//! \retval      NVAPI_SUCCESS                               - Default stereo profile name has been copied into szProfileName.
//! \retval      NVAPI_INVALID_ARGUMENT                      - szProfileName == NULL.
//! \retval      NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST - Default stereo profile does not exist
//!
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_SetDefaultProfile(__in const char* szProfileName);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_Stereo_GetDefaultProfile
//
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! DESCRIPTION: This API retrieves the current default stereo profile.
//!              
//!              After call cbSizeOut contain 0 if default profile is not set required buffer size cbSizeOut.
//!              To get needed buffer size this function can be called with szProfileName==0 and cbSizeIn == 0. 
//!
//! WHEN TO USE: This API can be called at any time.
//!              
//!
//! \param [in]   cbSizeIn             Size of buffer allocated for default stereo profile name.                  
//! \param [out]  szProfileName        Default stereo profile name. 
//! \param [out]  pcbSizeOut           Required buffer size.
//!                     # ==0 - there is no default stereo profile name currently set
//!                     # !=0 - size of buffer required for currently set default stereo profile name including trailing '0'.
//!
//!
//! \return      This API can return any of the error codes enumerated in #NvAPI_Status. 
//!              Error codes specific to this API are described below.
//! 
//! \retval      NVAPI_SUCCESS                                - Default stereo profile name has been copied into szProfileName.
//! \retval      NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED  - There is no default stereo profile set at this time.
//! \retval      NVAPI_INVALID_ARGUMENT                       - pcbSizeOut == 0 or cbSizeIn >= *pcbSizeOut && szProfileName == 0
//! \retval      NVAPI_INSUFFICIENT_BUFFER                    - cbSizeIn < *pcbSizeOut
//!  
//! \ingroup stereoapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_Stereo_GetDefaultProfile( __in NvU32 cbSizeIn, __out_bcount_part_opt(cbSizeIn, *pcbSizeOut) char* szProfileName,  __out NvU32 *pcbSizeOut);

#include"nvapi_lite_salend.h"
#ifdef __cplusplus
}
#endif
#pragma pack(pop)
