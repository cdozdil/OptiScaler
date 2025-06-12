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
#pragma pack(push,8)
#ifdef __cplusplus
extern "C" {
#endif
#if (defined(WIN32) || defined(_WIN32)) && defined(_MSC_VER) && (_MSC_VER > 1399) && !defined(NVAPI_INTERNAL) && !defined(NVAPI_DEPRECATED_OLD)
#ifndef __nvapi_deprecated_function
#define __nvapi_deprecated_function(message) __declspec(deprecated(message))
#endif
#ifndef __nvapi_deprecated_datatype
#define __nvapi_deprecated_datatype(FirstRelease) __declspec(deprecated("Do not use this data type - it is deprecated in release " #FirstRelease "."))
#endif
#else
#ifndef __nvapi_deprecated_function
#define __nvapi_deprecated_function(message)
#endif
#ifndef __nvapi_deprecated_datatype
#define __nvapi_deprecated_datatype(FirstRelease)
#endif
#endif

#define NV_U8_MAX       (+255U)
#define NV_U16_MAX      (+65535U)
#define NV_S32_MAX      (+2147483647)
#define NV_U32_MIN      (0U)
#define NV_U32_MAX      (+4294967295U)
#define NV_U64_MAX      (+18446744073709551615ULL)

/* 64-bit types for compilers that support them, plus some obsolete variants */
#if defined(__GNUC__) || defined(__arm) || defined(__IAR_SYSTEMS_ICC__) || defined(__ghs__) || defined(_WIN64)
typedef unsigned long long NvU64; /* 0 to 18446744073709551615  */
typedef long long          NvS64; /* -9223372036854775808 to 9223372036854775807  */
#else
typedef unsigned __int64   NvU64; /* 0 to 18446744073709551615  */
typedef __int64            NvS64; /* -9223372036854775808 to 9223372036854775807  */
#endif

#ifndef NVAPI_USE_STDINT
#define NVAPI_USE_STDINT 0
#endif

#if NVAPI_USE_STDINT
typedef uint32_t           NvV32; /* "void": enumerated or multiple fields   */
typedef uint32_t           NvU32; /* 0 to 4294967295                         */
typedef  int32_t           NvS32; /* -2147483648 to 2147483647               */

#else
// mac os 32-bit still needs this
#if (defined(macintosh) || defined(__APPLE__)) && !defined(__LP64__)
typedef signed long        NvS32; /* -2147483648 to 2147483647  */   
#else
typedef signed int         NvS32; /* -2147483648 to 2147483647 */  
#endif

#if !((defined(NV_UNIX)) ||  (defined(__unix)))
// mac os 32-bit still needs this
#if ( (defined(macintosh) && defined(__LP64__) && (__NVAPI_RESERVED0__)) || \
      (!defined(macintosh) && defined(__NVAPI_RESERVED0__)) ) 
typedef unsigned int       NvU32; /* 0 to 4294967295                         */
#elif defined(__clang__)
typedef unsigned int       NvU32; /* 0 to 4294967295                         */
#else
typedef unsigned long      NvU32; /* 0 to 4294967295                         */
#endif
#else
typedef unsigned int       NvU32; /* 0 to 4294967295                         */
#endif
#endif

typedef unsigned long    temp_NvU32; /* 0 to 4294967295                         */
typedef signed   short   NvS16;
typedef unsigned short   NvU16;
typedef unsigned char    NvU8;
typedef signed   char    NvS8;
typedef float            NvF32;
typedef double           NvF64;

/*!
 * Macro to convert NvU32 to NvF32.
 */
#define NvU32TONvF32(_pData) *(NvF32 *)(_pData)
/*!
 * Macro to convert NvF32 to NvU32.
 */
#define NvF32TONvU32(_pData) *(NvU32 *)(_pData)

#define NVAPI_SDK_VERSION 57594
/* Boolean type */
typedef NvU8 NvBool;
#define NV_TRUE           ((NvBool)(0 == 0))
#define NV_FALSE          ((NvBool)(0 != 0))

typedef struct _NV_RECT
{
    NvU32    left;
    NvU32    top;
    NvU32    right;
    NvU32    bottom;
} NV_RECT;


#define NV_DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

//! \addtogroup nvapihandles
//! NVAPI Handles - These handles are retrieved from various calls and passed in to others in NvAPI
//!                 These are meant to be opaque types.  Do not assume they correspond to indices, HDCs,
//!                 display indexes or anything else.
//!
//!                 Most handles remain valid until a display re-configuration (display mode set) or GPU
//!                 reconfiguration (going into or out of SLI modes) occurs.  If NVAPI_HANDLE_INVALIDATED
//!                 is received by an app, it should discard all handles, and re-enumerate them.
//! @{  
NV_DECLARE_HANDLE(NvLogicalGpuHandle);             //!< One or more physical GPUs acting in concert (SLI)
NV_DECLARE_HANDLE(NvPhysicalGpuHandle);            //!< A single physical GPU
NV_DECLARE_HANDLE(NvDisplayHandle);                //!< Display Device driven by NVIDIA GPU(s) (an attached display)
NV_DECLARE_HANDLE(NvMonitorHandle);                //!< Monitor handle
NV_DECLARE_HANDLE(NvUnAttachedDisplayHandle);      //!< Unattached Display Device driven by NVIDIA GPU(s)
NV_DECLARE_HANDLE(NvVisualComputingDeviceHandle);  //!< A handle to a Visual Computing Device
NV_DECLARE_HANDLE(NvEventHandle);                  //!< A handle to an event registration instance


NV_DECLARE_HANDLE(NvHICHandle);                    //!< A handle to a Host Interface Card
NV_DECLARE_HANDLE(NvGSyncDeviceHandle);            //!< A handle to a Sync device
NV_DECLARE_HANDLE(NvVioHandle);                    //!< A handle to an SDI device
NV_DECLARE_HANDLE(NvTransitionHandle);             //!< A handle to address a single transition request
NV_DECLARE_HANDLE(NvAudioHandle);                  //!< NVIDIA HD Audio Device
NV_DECLARE_HANDLE(Nv3DVPContextHandle);            //!< A handle for a 3D Vision Pro (3DVP) context
NV_DECLARE_HANDLE(Nv3DVPTransceiverHandle);        //!< A handle for a 3DVP RF transceiver
NV_DECLARE_HANDLE(Nv3DVPGlassesHandle);            //!< A handle for a pair of 3DVP RF shutter glasses
NV_DECLARE_HANDLE(NvPcfClientHandle);              //!< A handle for NVPCF clients

typedef void* StereoHandle;                        //!< A stereo handle, that corresponds to the device interface

NV_DECLARE_HANDLE(NvSourceHandle);                 //!< Unique source handle on the system
NV_DECLARE_HANDLE(NvTargetHandle);                 //!< Unique target handle on the system
NV_DECLARE_HANDLE(NVDX_SwapChainHandle);           //!< DirectX SwapChain objects
static const NVDX_SwapChainHandle NVDX_SWAPCHAIN_NONE = 0;
NV_DECLARE_HANDLE(NvPresentBarrierClientHandle);   //!< PresentBarrier client object
//! @}

//! \ingroup nvapihandles
//! @{
#define NVAPI_DEFAULT_HANDLE        0
#define NV_BIT(x)    (1 << (x)) 
//! @}



//! \addtogroup nvapitypes
//! @{
#define NVAPI_GENERIC_STRING_MAX    4096
#define NVAPI_LONG_STRING_MAX       256
#define NVAPI_SHORT_STRING_MAX      64

typedef struct 
{
    NvS32   sX;
    NvS32   sY;
    NvS32   sWidth;
    NvS32   sHeight;
} NvSBox;

#ifndef NvGUID_Defined
#define NvGUID_Defined

typedef struct
{
    NvU32 data1;
    NvU16 data2;
    NvU16 data3;
    NvU8  data4[8];
} NvGUID, NvLUID;


#endif //#ifndef NvGUID_Defined
#define NVAPI_MAX_PHYSICAL_GPUS             64


#define NVAPI_MAX_PHYSICAL_BRIDGES          100
#define NVAPI_PHYSICAL_GPUS                 32
#define NVAPI_MAX_LOGICAL_GPUS              64
#define NVAPI_MAX_AVAILABLE_GPU_TOPOLOGIES  256
#define NVAPI_MAX_AVAILABLE_SLI_GROUPS      256
#define NVAPI_MAX_GPU_TOPOLOGIES            NVAPI_MAX_PHYSICAL_GPUS
#define NVAPI_MAX_GPU_PER_TOPOLOGY          8
#define NVAPI_MAX_DISPLAY_HEADS             2
#define NVAPI_ADVANCED_DISPLAY_HEADS        4
#define NVAPI_MAX_DISPLAYS                  NVAPI_PHYSICAL_GPUS * NVAPI_ADVANCED_DISPLAY_HEADS
#define NVAPI_MAX_ACPI_IDS                  16
#define NVAPI_MAX_VIEW_MODES                8


#define NVAPI_SYSTEM_MAX_HWBCS              128
#define NVAPI_SYSTEM_HWBC_INVALID_ID        0xffffffff

#define NVAPI_SYSTEM_MAX_DISPLAYS           NVAPI_MAX_PHYSICAL_GPUS * NV_MAX_HEADS
#define NV_MAX_HEADS                        4   //!< Maximum heads, each with NVAPI_DESKTOP_RES resolution
#define NVAPI_MAX_HEADS_PER_GPU             32
#define NV_MAX_VID_STREAMS      4   //!< Maximum number of input video streams, each with a #NVAPI_VIDEO_SRC_INFO
#define NV_MAX_VID_STREAMS_EX  20   //!< Increasing MAX no. of input video streams, each with a #NVAPI_VIDEO_SRC_INFO
#define NV_MAX_VID_PROFILES     4   //!< Maximum number of output video profiles supported

#define NVAPI_MAX_AUDIO_DEVICES             16


typedef char NvAPI_String[NVAPI_GENERIC_STRING_MAX];
typedef char NvAPI_LongString[NVAPI_LONG_STRING_MAX];
typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];
typedef NvU16 NvAPI_UnicodeShortString[NVAPI_SHORT_STRING_MAX];
//! @}


// =========================================================================================
//!  NvAPI Version Definition \n
//!  Maintain per structure specific version define using the MAKE_NVAPI_VERSION macro. \n
//!  Usage: #define NV_GENLOCK_STATUS_VER  MAKE_NVAPI_VERSION(NV_GENLOCK_STATUS, 1)
//!  \ingroup nvapitypes
// =========================================================================================
#define MAKE_NVAPI_VERSION(typeName,ver) (NvU32)(sizeof(typeName) | ((ver)<<16))

//!  \ingroup nvapitypes
#define GET_NVAPI_VERSION(ver) (NvU32)((ver)>>16)

//!  \ingroup nvapitypes
#define GET_NVAPI_SIZE(ver) (NvU32)((ver) & 0xffff)


// ====================================================
//! NvAPI Status Values
//!   All NvAPI functions return one of these codes.
//!   \ingroup nvapistatus 
// ====================================================


typedef enum _NvAPI_Status
{
    NVAPI_OK                                    =  0,      //!< Success. Request is completed.
    NVAPI_ERROR                                 = -1,      //!< Generic error
    NVAPI_LIBRARY_NOT_FOUND                     = -2,      //!< NVAPI support library cannot be loaded.
    NVAPI_NO_IMPLEMENTATION                     = -3,      //!< not implemented in current driver installation
    NVAPI_API_NOT_INITIALIZED                   = -4,      //!< NvAPI_Initialize has not been called (successfully)
    NVAPI_INVALID_ARGUMENT                      = -5,      //!< The argument/parameter value is not valid or NULL.
    NVAPI_NVIDIA_DEVICE_NOT_FOUND               = -6,      //!< No NVIDIA display driver, or NVIDIA GPU driving a display, was found.
    NVAPI_END_ENUMERATION                       = -7,      //!< No more items to enumerate
    NVAPI_INVALID_HANDLE                        = -8,      //!< Invalid handle
    NVAPI_INCOMPATIBLE_STRUCT_VERSION           = -9,      //!< An argument's structure version is not supported
    NVAPI_HANDLE_INVALIDATED                    = -10,     //!< The handle is no longer valid (likely due to GPU or display re-configuration)
    NVAPI_OPENGL_CONTEXT_NOT_CURRENT            = -11,     //!< No NVIDIA OpenGL context is current (but needs to be)
    NVAPI_INVALID_POINTER                       = -14,     //!< An invalid pointer, usually NULL, was passed as a parameter
    NVAPI_NO_GL_EXPERT                          = -12,     //!< OpenGL Expert is not supported by the current drivers
    NVAPI_INSTRUMENTATION_DISABLED              = -13,     //!< OpenGL Expert is supported, but driver instrumentation is currently disabled
    NVAPI_NO_GL_NSIGHT                          = -15,     //!< OpenGL does not support Nsight

    NVAPI_EXPECTED_LOGICAL_GPU_HANDLE           = -100,    //!< Expected a logical GPU handle for one or more parameters
    NVAPI_EXPECTED_PHYSICAL_GPU_HANDLE          = -101,    //!< Expected a physical GPU handle for one or more parameters
    NVAPI_EXPECTED_DISPLAY_HANDLE               = -102,    //!< Expected an NV display handle for one or more parameters
    NVAPI_INVALID_COMBINATION                   = -103,    //!< The combination of parameters is not valid. 
    NVAPI_NOT_SUPPORTED                         = -104,    //!< Requested feature is not supported in the selected GPU
    NVAPI_PORTID_NOT_FOUND                      = -105,    //!< No port ID was found for the I2C transaction
    NVAPI_EXPECTED_UNATTACHED_DISPLAY_HANDLE    = -106,    //!< Expected an unattached display handle as one of the input parameters.
    NVAPI_INVALID_PERF_LEVEL                    = -107,    //!< Invalid perf level 
    NVAPI_DEVICE_BUSY                           = -108,    //!< Device is busy; request not fulfilled
    NVAPI_NV_PERSIST_FILE_NOT_FOUND             = -109,    //!< NV persist file is not found
    NVAPI_PERSIST_DATA_NOT_FOUND                = -110,    //!< NV persist data is not found
    NVAPI_EXPECTED_TV_DISPLAY                   = -111,    //!< Expected a TV output display
    NVAPI_EXPECTED_TV_DISPLAY_ON_DCONNECTOR     = -112,    //!< Expected a TV output on the D Connector - HDTV_EIAJ4120.
    NVAPI_NO_ACTIVE_SLI_TOPOLOGY                = -113,    //!< SLI is not active on this device.
    NVAPI_SLI_RENDERING_MODE_NOTALLOWED         = -114,    //!< Setup of SLI rendering mode is not possible right now.
    NVAPI_EXPECTED_DIGITAL_FLAT_PANEL           = -115,    //!< Expected a digital flat panel.
    NVAPI_ARGUMENT_EXCEED_MAX_SIZE              = -116,    //!< Argument exceeds the expected size.
    NVAPI_DEVICE_SWITCHING_NOT_ALLOWED          = -117,    //!< Inhibit is ON due to one of the flags in NV_GPU_DISPLAY_CHANGE_INHIBIT or SLI active.
    NVAPI_TESTING_CLOCKS_NOT_SUPPORTED          = -118,    //!< Testing of clocks is not supported.
    NVAPI_UNKNOWN_UNDERSCAN_CONFIG              = -119,    //!< The specified underscan config is from an unknown source (e.g. INF)
    NVAPI_TIMEOUT_RECONFIGURING_GPU_TOPO        = -120,    //!< Timeout while reconfiguring GPUs
    NVAPI_DATA_NOT_FOUND                        = -121,    //!< Requested data was not found
    NVAPI_EXPECTED_ANALOG_DISPLAY               = -122,    //!< Expected an analog display
    NVAPI_NO_VIDLINK                            = -123,    //!< No SLI video bridge is present
    NVAPI_REQUIRES_REBOOT                       = -124,    //!< NVAPI requires a reboot for the settings to take effect
    NVAPI_INVALID_HYBRID_MODE                   = -125,    //!< The function is not supported with the current Hybrid mode.
    NVAPI_MIXED_TARGET_TYPES                    = -126,    //!< The target types are not all the same
    NVAPI_SYSWOW64_NOT_SUPPORTED                = -127,    //!< The function is not supported from 32-bit on a 64-bit system.
    NVAPI_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,    //!< There is no implicit GPU topology active. Use NVAPI_SetHybridMode to change topology.
    NVAPI_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,      //!< Prompt the user to close all non-migratable applications.    
    NVAPI_OUT_OF_MEMORY                         = -130,    //!< Could not allocate sufficient memory to complete the call.
    NVAPI_WAS_STILL_DRAWING                     = -131,    //!< The previous operation that is transferring information to or from this surface is incomplete.
    NVAPI_FILE_NOT_FOUND                        = -132,    //!< The file was not found.
    NVAPI_TOO_MANY_UNIQUE_STATE_OBJECTS         = -133,    //!< There are too many unique instances of a particular type of state object.
    NVAPI_INVALID_CALL                          = -134,    //!< The method call is invalid. For example, a method's parameter may not be a valid pointer.
    NVAPI_D3D10_1_LIBRARY_NOT_FOUND             = -135,    //!< d3d10_1.dll cannot be loaded.
    NVAPI_FUNCTION_NOT_FOUND                    = -136,    //!< Couldn't find the function in the loaded DLL.
    NVAPI_INVALID_USER_PRIVILEGE                = -137,    //!< The application will require Administrator privileges to access this API.
	                                                       //!< The application can be elevated to a higher permission level by selecting "Run as Administrator".
    NVAPI_EXPECTED_NON_PRIMARY_DISPLAY_HANDLE   = -138,    //!< The handle corresponds to GDIPrimary.
    NVAPI_EXPECTED_COMPUTE_GPU_HANDLE           = -139,    //!< Setting Physx GPU requires that the GPU is compute-capable.
    NVAPI_STEREO_NOT_INITIALIZED                = -140,    //!< The Stereo part of NVAPI failed to initialize completely. Check if the stereo driver is installed.
    NVAPI_STEREO_REGISTRY_ACCESS_FAILED         = -141,    //!< Access to stereo-related registry keys or values has failed.
    NVAPI_STEREO_REGISTRY_PROFILE_TYPE_NOT_SUPPORTED = -142, //!< The given registry profile type is not supported.
    NVAPI_STEREO_REGISTRY_VALUE_NOT_SUPPORTED   = -143,    //!< The given registry value is not supported.
    NVAPI_STEREO_NOT_ENABLED                    = -144,    //!< Stereo is not enabled and the function needed it to execute completely.
    NVAPI_STEREO_NOT_TURNED_ON                  = -145,    //!< Stereo is not turned on and the function needed it to execute completely.
    NVAPI_STEREO_INVALID_DEVICE_INTERFACE       = -146,    //!< Invalid device interface.
    NVAPI_STEREO_PARAMETER_OUT_OF_RANGE         = -147,    //!< Separation percentage or JPEG image capture quality is out of [0-100] range.
    NVAPI_STEREO_FRUSTUM_ADJUST_MODE_NOT_SUPPORTED = -148, //!< The given frustum adjust mode is not supported.
    NVAPI_TOPO_NOT_POSSIBLE                     = -149,    //!< The mosaic topology is not possible given the current state of the hardware.
    NVAPI_MODE_CHANGE_FAILED                    = -150,    //!< An attempt to do a display resolution mode change has failed.        
    NVAPI_D3D11_LIBRARY_NOT_FOUND               = -151,    //!< d3d11.dll/d3d11_beta.dll cannot be loaded.
    NVAPI_INVALID_ADDRESS                       = -152,    //!< Address is outside of valid range.
    NVAPI_STRING_TOO_SMALL                      = -153,    //!< The pre-allocated string is too small to hold the result.
    NVAPI_MATCHING_DEVICE_NOT_FOUND             = -154,    //!< The input does not match any of the available devices.
    NVAPI_DRIVER_RUNNING                        = -155,    //!< Driver is running.
    NVAPI_DRIVER_NOTRUNNING                     = -156,    //!< Driver is not running.
    NVAPI_ERROR_DRIVER_RELOAD_REQUIRED          = -157,    //!< A driver reload is required to apply these settings.
    NVAPI_SET_NOT_ALLOWED                       = -158,    //!< Intended setting is not allowed.
    NVAPI_ADVANCED_DISPLAY_TOPOLOGY_REQUIRED    = -159,    //!< Information can't be returned due to "advanced display topology".
    NVAPI_SETTING_NOT_FOUND                     = -160,    //!< Setting is not found.
    NVAPI_SETTING_SIZE_TOO_LARGE                = -161,    //!< Setting size is too large.
    NVAPI_TOO_MANY_SETTINGS_IN_PROFILE          = -162,    //!< There are too many settings for a profile. 
    NVAPI_PROFILE_NOT_FOUND                     = -163,    //!< Profile is not found.
    NVAPI_PROFILE_NAME_IN_USE                   = -164,    //!< Profile name is duplicated.
    NVAPI_PROFILE_NAME_EMPTY                    = -165,    //!< Profile name is empty.
    NVAPI_EXECUTABLE_NOT_FOUND                  = -166,    //!< Application not found in the Profile.
    NVAPI_EXECUTABLE_ALREADY_IN_USE             = -167,    //!< Application already exists in the other profile.
    NVAPI_DATATYPE_MISMATCH                     = -168,    //!< Data Type mismatch 
    NVAPI_PROFILE_REMOVED                       = -169,    //!< The profile passed as parameter has been removed and is no longer valid.
    NVAPI_UNREGISTERED_RESOURCE                 = -170,    //!< An unregistered resource was passed as a parameter. 
    NVAPI_ID_OUT_OF_RANGE                       = -171,    //!< The DisplayId corresponds to a display which is not within the normal outputId range.
    NVAPI_DISPLAYCONFIG_VALIDATION_FAILED       = -172,    //!< Display topology is not valid so the driver cannot do a mode set on this configuration.
    NVAPI_DPMST_CHANGED                         = -173,    //!< Display Port Multi-Stream topology has been changed.
    NVAPI_INSUFFICIENT_BUFFER                   = -174,    //!< Input buffer is insufficient to hold the contents.    
    NVAPI_ACCESS_DENIED                         = -175,    //!< No access to the caller.
    NVAPI_MOSAIC_NOT_ACTIVE                     = -176,    //!< The requested action cannot be performed without Mosaic being enabled.
    NVAPI_SHARE_RESOURCE_RELOCATED              = -177,    //!< The surface is relocated away from video memory.
    NVAPI_REQUEST_USER_TO_DISABLE_DWM           = -178,    //!< The user should disable DWM before calling NvAPI.
    NVAPI_D3D_DEVICE_LOST                       = -179,    //!< D3D device status is D3DERR_DEVICELOST or D3DERR_DEVICENOTRESET - the user has to reset the device.
    NVAPI_INVALID_CONFIGURATION                 = -180,    //!< The requested action cannot be performed in the current state.
    NVAPI_STEREO_HANDSHAKE_NOT_DONE             = -181,    //!< Call failed as stereo handshake not completed.
    NVAPI_EXECUTABLE_PATH_IS_AMBIGUOUS          = -182,    //!< The path provided was too short to determine the correct NVDRS_APPLICATION
    NVAPI_DEFAULT_STEREO_PROFILE_IS_NOT_DEFINED = -183,    //!< Default stereo profile is not currently defined
    NVAPI_DEFAULT_STEREO_PROFILE_DOES_NOT_EXIST = -184,    //!< Default stereo profile does not exist
    NVAPI_CLUSTER_ALREADY_EXISTS                = -185,    //!< A cluster is already defined with the given configuration.
    NVAPI_DPMST_DISPLAY_ID_EXPECTED             = -186,    //!< The input display id is not that of a multi stream enabled connector or a display device in a multi stream topology 
    NVAPI_INVALID_DISPLAY_ID                    = -187,    //!< The input display id is not valid or the monitor associated to it does not support the current operation
    NVAPI_STREAM_IS_OUT_OF_SYNC                 = -188,    //!< While playing secure audio stream, stream goes out of sync
    NVAPI_INCOMPATIBLE_AUDIO_DRIVER             = -189,    //!< Older audio driver version than required
    NVAPI_VALUE_ALREADY_SET                     = -190,    //!< Value already set, setting again not allowed.
    NVAPI_TIMEOUT                               = -191,    //!< Requested operation timed out 
    NVAPI_GPU_WORKSTATION_FEATURE_INCOMPLETE    = -192,    //!< The requested workstation feature set has incomplete driver internal allocation resources
    NVAPI_STEREO_INIT_ACTIVATION_NOT_DONE       = -193,    //!< Call failed because InitActivation was not called.
    NVAPI_SYNC_NOT_ACTIVE                       = -194,    //!< The requested action cannot be performed without Sync being enabled.    
    NVAPI_SYNC_MASTER_NOT_FOUND                 = -195,    //!< The requested action cannot be performed without Sync Master being enabled.
    NVAPI_INVALID_SYNC_TOPOLOGY                 = -196,    //!< Invalid displays passed in the NV_GSYNC_DISPLAY pointer.
    NVAPI_ECID_SIGN_ALGO_UNSUPPORTED            = -197,    //!< The specified signing algorithm is not supported. Either an incorrect value was entered or the current installed driver/hardware does not support the input value.
    NVAPI_ECID_KEY_VERIFICATION_FAILED          = -198,    //!< The encrypted public key verification has failed.
    NVAPI_FIRMWARE_OUT_OF_DATE                  = -199,    //!< The device's firmware is out of date.
    NVAPI_FIRMWARE_REVISION_NOT_SUPPORTED       = -200,    //!< The device's firmware is not supported.
    NVAPI_LICENSE_CALLER_AUTHENTICATION_FAILED  = -201,    //!< The caller is not authorized to modify the License.
    NVAPI_D3D_DEVICE_NOT_REGISTERED             = -202,    //!< The user tried to use a deferred context without registering the device first  
    NVAPI_RESOURCE_NOT_ACQUIRED                 = -203,    //!< Head or SourceId was not reserved for the VR Display before doing the Modeset or the dedicated display.
    NVAPI_TIMING_NOT_SUPPORTED                  = -204,    //!< Provided timing is not supported.
    NVAPI_HDCP_ENCRYPTION_FAILED                = -205,    //!< HDCP Encryption Failed for the device. Would be applicable when the device is HDCP Capable.
    NVAPI_PCLK_LIMITATION_FAILED                = -206,    //!< Provided mode is over sink device pclk limitation.
    NVAPI_NO_CONNECTOR_FOUND                    = -207,    //!< No connector on GPU found. 
    NVAPI_HDCP_DISABLED                         = -208,    //!< When a non-HDCP capable HMD is connected, we would inform user by this code.
    NVAPI_API_IN_USE                            = -209,    //!< Atleast an API is still being called
    NVAPI_NVIDIA_DISPLAY_NOT_FOUND              = -210,    //!< No display found on Nvidia GPU(s).
    NVAPI_PRIV_SEC_VIOLATION                    = -211,    //!< Priv security violation, improper access to a secured register.
    NVAPI_INCORRECT_VENDOR                      = -212,    //!< NVAPI cannot be called by this vendor
    NVAPI_DISPLAY_IN_USE                        = -213,    //!< DirectMode Display is already in use
    NVAPI_UNSUPPORTED_CONFIG_NON_HDCP_HMD       = -214,    //!< The Config is having Non-NVidia GPU with Non-HDCP HMD connected
    NVAPI_MAX_DISPLAY_LIMIT_REACHED             = -215,    //!< GPU's Max Display Limit has Reached
    NVAPI_INVALID_DIRECT_MODE_DISPLAY           = -216,    //!< DirectMode not Enabled on the Display
    NVAPI_GPU_IN_DEBUG_MODE                     = -217,    //!< GPU is in debug mode, OC is NOT allowed.
    NVAPI_D3D_CONTEXT_NOT_FOUND                 = -218,    //!< No NvAPI context was found for this D3D object
    NVAPI_STEREO_VERSION_MISMATCH               = -219,    //!< there is version mismatch between stereo driver and dx driver
    NVAPI_GPU_NOT_POWERED                       = -220,    //!< GPU is not powered and so the request cannot be completed.
    NVAPI_ERROR_DRIVER_RELOAD_IN_PROGRESS       = -221,    //!< The display driver update in progress.
    NVAPI_WAIT_FOR_HW_RESOURCE                  = -222,    //!< Wait for HW resources allocation
    NVAPI_REQUIRE_FURTHER_HDCP_ACTION           = -223,    //!< operation requires further HDCP action
    NVAPI_DISPLAY_MUX_TRANSITION_FAILED         = -224,    //!< Dynamic Mux transition failure
    NVAPI_INVALID_DSC_VERSION                   = -225,    //!< Invalid DSC version
    NVAPI_INVALID_DSC_SLICECOUNT                = -226,    //!< Invalid DSC slice count
    NVAPI_INVALID_DSC_OUTPUT_BPP                = -227,    //!< Invalid DSC output BPP
    NVAPI_FAILED_TO_LOAD_FROM_DRIVER_STORE      = -228,    //!< There was an error while loading nvapi.dll from the driver store.
    NVAPI_NO_VULKAN                             = -229,    //!< OpenGL does not export Vulkan fake extensions
    NVAPI_REQUEST_PENDING                       = -230,    //!< A request for NvTOPPs telemetry CData has already been made and is pending a response.
    NVAPI_RESOURCE_IN_USE                       = -231,    //!< Operation cannot be performed because the resource is in use.
    NVAPI_INVALID_IMAGE                         = -232,    //!< Device kernel image is invalid
    NVAPI_INVALID_PTX                           = -233,    //!< PTX JIT compilation failed
    NVAPI_NVLINK_UNCORRECTABLE                  = -234,    //!< Uncorrectable NVLink error was detected during the execution
    NVAPI_JIT_COMPILER_NOT_FOUND                = -235,    //!< PTX JIT compiler library was not found.
    NVAPI_INVALID_SOURCE                        = -236,    //!< Device kernel source is invalid.
    NVAPI_ILLEGAL_INSTRUCTION                   = -237,    //!< While executing a kernel, the device encountered an illegal instruction.
    NVAPI_INVALID_PC                            = -238,    //!< While executing a kernel, the device program counter wrapped its address space
    NVAPI_LAUNCH_FAILED                         = -239,    //!< An exception occurred on the device while executing a kernel
    NVAPI_NOT_PERMITTED                         = -240,    //!< Attempted operation is not permitted.
    NVAPI_CALLBACK_ALREADY_REGISTERED           = -241,    //!< The callback function has already been registered.
    NVAPI_CALLBACK_NOT_FOUND                    = -242,    //!< The callback function is not found or not registered.
    NVAPI_INVALID_OUTPUT_WIRE_FORMAT            = -243,    //!< Invalid Wire Format for the VR HMD
} NvAPI_Status;

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_SYS_GetDriverAndBranchVersion
//
//!   DESCRIPTION: This API returns display driver version and driver-branch string.
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//! 
//! \param [out]  pDriverVersion         Contains the driver version after successful return.
//! \param [out]  szBuildBranchString    Contains the driver-branch string after successful return.
//!
//! \retval ::NVAPI_INVALID_ARGUMENT: either pDriverVersion is NULL or enum index too big
//! \retval ::NVAPI_OK - completed request
//! \retval ::NVAPI_API_NOT_INTIALIZED - NVAPI not initialized
//! \retval ::NVAPI_ERROR - miscellaneous error occurred
//! 
//! \ingroup driverapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_SYS_GetDriverAndBranchVersion(NvU32* pDriverVersion, NvAPI_ShortString szBuildBranchString);
//! \ingroup driverapi
//! Used in NvAPI_GPU_GetMemoryInfo().
typedef struct 
{
    NvU32   version;                        //!< Version info
    NvU32   dedicatedVideoMemory;           //!< Size(in kb) of the physical framebuffer.
    NvU32   availableDedicatedVideoMemory;  //!< Size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    NvU32   systemVideoMemory;              //!< Size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;             //!< Size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.

} NV_DISPLAY_DRIVER_MEMORY_INFO_V1;


//! \ingroup driverapi
//! Used in NvAPI_GPU_GetMemoryInfo().
typedef struct
{
    NvU32   version;                           //!< Version info
    NvU32   dedicatedVideoMemory;              //!< Size(in kb) of the physical framebuffer.
    NvU32   availableDedicatedVideoMemory;     //!< Size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    NvU32   systemVideoMemory;                 //!< Size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;                //!< Size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.
    NvU32   curAvailableDedicatedVideoMemory;  //!< Size(in kb) of the current available physical framebuffer for allocating video memory surfaces.

} NV_DISPLAY_DRIVER_MEMORY_INFO_V2;

//! \ingroup driverapi
//! Used in NvAPI_GPU_GetMemoryInfo().
typedef struct
{
    NvU32   version;                           //!< Version info
    NvU32   dedicatedVideoMemory;              //!< Size(in kb) of the physical framebuffer.
    NvU32   availableDedicatedVideoMemory;     //!< Size(in kb) of the available physical framebuffer for allocating video memory surfaces.
    NvU32   systemVideoMemory;                 //!< Size(in kb) of system memory the driver allocates at load time.
    NvU32   sharedSystemMemory;                //!< Size(in kb) of shared system memory that driver is allowed to commit for surfaces across all allocations.
    NvU32   curAvailableDedicatedVideoMemory;  //!< Size(in kb) of the current available physical framebuffer for allocating video memory surfaces.
    NvU32   dedicatedVideoMemoryEvictionsSize; //!< Size(in kb) of the total size of memory released as a result of the evictions.
    NvU32   dedicatedVideoMemoryEvictionCount; //!< Indicates the number of eviction events that caused an allocation to be removed from dedicated video memory to free GPU
                                               //!< video memory to make room for other allocations.
} NV_DISPLAY_DRIVER_MEMORY_INFO_V3;

//! \ingroup driverapi
typedef NV_DISPLAY_DRIVER_MEMORY_INFO_V3 NV_DISPLAY_DRIVER_MEMORY_INFO;

//! \ingroup driverapi
//! Macro for constructing the version field of NV_DISPLAY_DRIVER_MEMORY_INFO_V1
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER_1  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V1,1)

//! \ingroup driverapi
//! Macro for constructing the version field of NV_DISPLAY_DRIVER_MEMORY_INFO_V2
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER_2  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V2,2)

//! \ingroup driverapi
//! Macro for constructing the version field of NV_DISPLAY_DRIVER_MEMORY_INFO_V3
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER_3  MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V3,3)

//! \ingroup driverapi
#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER    NV_DISPLAY_DRIVER_MEMORY_INFO_VER_3




///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetMemoryInfo
//
//!   DESCRIPTION: This function retrieves the available driver memory footprint for the specified GPU. 
//!                If the GPU is in TCC Mode, only dedicatedVideoMemory will be returned in pMemoryInfo (NV_DISPLAY_DRIVER_MEMORY_INFO).
//!
//! \deprecated  Do not use this function - it is deprecated in release 520. Instead, use NvAPI_GPU_GetMemoryInfoEx.
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! TCC_SUPPORTED
//!
//! \since Release: 177
//!
//!  \param [in]   hPhysicalGpu  Handle of the physical GPU for which the memory information is to be extracted.
//!  \param [out]  pMemoryInfo   The memory footprint available in the driver. See NV_DISPLAY_DRIVER_MEMORY_INFO.
//!
//!  \retval       NVAPI_INVALID_ARGUMENT             pMemoryInfo is NULL.
//!  \retval       NVAPI_OK                           Call successful.
//!  \retval       NVAPI_NVIDIA_DEVICE_NOT_FOUND      No NVIDIA GPU driving a display was found.
//!  \retval       NVAPI_INCOMPATIBLE_STRUCT_VERSION  NV_DISPLAY_DRIVER_MEMORY_INFO structure version mismatch.
//!
//!  \ingroup  driverapi
///////////////////////////////////////////////////////////////////////////////
__nvapi_deprecated_function("Do not use this function - it is deprecated in release 520. Instead, use NvAPI_GPU_GetMemoryInfoEx.")
NVAPI_INTERFACE NvAPI_GPU_GetMemoryInfo(NvPhysicalGpuHandle hPhysicalGpu, NV_DISPLAY_DRIVER_MEMORY_INFO *pMemoryInfo);


//! \ingroup driverapi
//! Used in NvAPI_GPU_GetMemoryInfoEx().
typedef struct
{
    NvU32   version;                           //!< Structure version
    NvU64   dedicatedVideoMemory;              //!< Size(in bytes) of the physical framebuffer. Refers to the dedicated video memory on discrete GPUs.
                                               //!  It is more performant for GPU operations than the reserved systemVideoMemory.
    NvU64   availableDedicatedVideoMemory;     //!< Size(in bytes) of the available physical framebuffer for allocating video memory surfaces.
    NvU64   systemVideoMemory;                 //!< Size(in bytes) of system memory the driver allocates at load time. It is a substitute for dedicated video memory.
                                               //!< Typically used with integrated GPUs that do not have dedicated video memory.
    NvU64   sharedSystemMemory;                //!< Size(in bytes) of shared system memory that driver is allowed to commit for surfaces across all allocations.
                                               //!< On discrete GPUs, it is used to utilize system memory for various operations. It does not need to be reserved during boot.
                                               //!< It may be used by both GPU and CPU, and has an "on-demand" type of usage.
    NvU64   curAvailableDedicatedVideoMemory;  //!< Size(in bytes) of the current available physical framebuffer for allocating video memory surfaces.
    NvU64   dedicatedVideoMemoryEvictionsSize; //!< Size(in bytes) of the total size of memory released as a result of the evictions.
    NvU64   dedicatedVideoMemoryEvictionCount; //!< Indicates the number of eviction events that caused an allocation to be removed from dedicated video memory to free GPU
                                               //!< video memory to make room for other allocations.
    NvU64 dedicatedVideoMemoryPromotionsSize;  //!< Size(in bytes) of the total size of memory allocated as a result of the promotions.
    NvU64 dedicatedVideoMemoryPromotionCount;  //!< Indicates the number of promotion events that caused an allocation to be promoted to dedicated video memory 
} NV_GPU_MEMORY_INFO_EX_V1;

//! \ingroup driverapi
typedef NV_GPU_MEMORY_INFO_EX_V1 NV_GPU_MEMORY_INFO_EX;

//! \ingroup driverapi
//! Macro for constructing the version field of NV_GPU_MEMORY_INFO_EX_V1
#define NV_GPU_MEMORY_INFO_EX_VER_1  MAKE_NVAPI_VERSION(NV_GPU_MEMORY_INFO_EX_V1,1)

//! \ingroup driverapi
#define NV_GPU_MEMORY_INFO_EX_VER    NV_GPU_MEMORY_INFO_EX_VER_1



///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_GPU_GetMemoryInfoEx
//
//!   DESCRIPTION: This function retrieves the available driver memory footprint for the specified GPU. 
//!                If the GPU is in TCC Mode, only dedicatedVideoMemory will be returned in pMemoryInfo (NV_GPU_MEMORY_INFO_EX).
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! TCC_SUPPORTED
//!
//! MCDM_SUPPORTED
//!
//! \since Release: 520
//!
//!  \param [in]   hPhysicalGpu  Handle of the physical GPU for which the memory information is to be extracted.
//!  \param [out]  pMemoryInfo   The memory footprint available in the driver. See NV_GPU_MEMORY_INFO_EX.
//!
//!  \retval       NVAPI_INVALID_ARGUMENT             pMemoryInfo is NULL.
//!  \retval       NVAPI_OK                           Call successful.
//!  \retval       NVAPI_NVIDIA_DEVICE_NOT_FOUND      No NVIDIA GPU driving a display was found.
//!  \retval       NVAPI_INCOMPATIBLE_STRUCT_VERSION  NV_GPU_MEMORY_INFO_EX structure version mismatch.
//!
//!  \ingroup  driverapi
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_GPU_GetMemoryInfoEx(NvPhysicalGpuHandle hPhysicalGpu, NV_GPU_MEMORY_INFO_EX *pMemoryInfo);
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_EnumPhysicalGPUs
//
//! This function returns an array of physical GPU handles.
//! Each handle represents a physical GPU present in the system.
//! That GPU may be part of an SLI configuration, or may not be visible to the OS directly.
//!
//! At least one GPU must be present in the system and running an NVIDIA display driver.
//!
//! The array nvGPUHandle will be filled with physical GPU handle values. The returned
//! gpuCount determines how many entries in the array are valid.
//!
//! \note In drivers older than 105.00, all physical GPU handles get invalidated on a
//!       modeset. So the calling applications need to renum the handles after every modeset.\n
//!       With drivers 105.00 and up, all physical GPU handles are constant.
//!       Physical GPU handles are constant as long as the GPUs are not physically moved and
//!       the SBIOS VGA order is unchanged.
//!
//!       For GPU handles in TCC MODE please use NvAPI_EnumTCCPhysicalGPUs()
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \par Introduced in
//! \since Release: 80
//!
//! \retval NVAPI_INVALID_ARGUMENT         nvGPUHandle or pGpuCount is NULL
//! \retval NVAPI_OK                       One or more handles were returned
//! \retval NVAPI_NVIDIA_DEVICE_NOT_FOUND  No NVIDIA GPU driving a display was found
//! \ingroup gpu
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_EnumPhysicalGPUs(NvPhysicalGpuHandle nvGPUHandle[NVAPI_MAX_PHYSICAL_GPUS], NvU32 *pGpuCount);
#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__) || defined(__d3d12_h__)

NV_DECLARE_HANDLE(NVDX_ObjectHandle);  // DX Objects
static const NVDX_ObjectHandle NVDX_OBJECT_NONE = 0;

#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__) || defined(__d3d12_h__)
#if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__) || defined(__d3d12_h__)

///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION NAME: NvAPI_D3D_GetObjectHandleForResource
//
//!   DESCRIPTION: This API gets a handle to a resource.
//!
//! \param [in]  pDev       The ID3D11Device, ID3D10Device or IDirect3DDevice9 or ID3D11DeviceContext to use
//! \param [in]  pResource  The ID3D11Resource, ID3D10Resource or IDirect3DResource9 from which
//!                         we want the NvAPI handle
//! \param [out]  pHandle   A handle to the resource
//!
//! SUPPORTED OS:  Windows 10 and higher
//!
//!
//! \since Release: 185
//!
//! \return ::NVAPI_OK if the handle was populated.
//!
//! \ingroup dx
///////////////////////////////////////////////////////////////////////////////
NVAPI_INTERFACE NvAPI_D3D_GetObjectHandleForResource(
    IUnknown *pDevice,
    IUnknown *pResource,
    NVDX_ObjectHandle *pHandle);


#endif //if defined(_D3D9_H_) || defined(__d3d10_h__) || defined(__d3d11_h__) || defined(__d3d12_h__)

#include"nvapi_lite_salend.h"
#ifdef __cplusplus
}
#endif
#pragma pack(pop)
