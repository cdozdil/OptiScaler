/*
* Copyright (c) 2022-2023 NVIDIA CORPORATION. All rights reserved
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <limits.h>

#include "sl_struct.h"
#include "sl_consts.h"
#include "sl_version.h"
#include "sl_result.h"
#include "sl_appidentity.h"
#include "sl_device_wrappers.h"

#include "sl_core_api.h"
#include "sl_core_types.h"

#define SL_FUN_DECL(name) PFun_##name* name{}
//! IMPORTANT: Macros which use `slGetFeatureFunction` can only be used AFTER device is set by calling either slSetD3DDevice or slSetVulkanInfo.
#define SL_FEATURE_FUN_IMPORT(feature, func) slGetFeatureFunction(feature, #func, (void*&) ##func)
#define SL_FEATURE_FUN_IMPORT_STATIC(feature, func)                             \
static PFun_##func* s_ ##func{};                                                \
if(!s_ ##func) {                                                                \
    sl::Result res = slGetFeatureFunction(feature, #func, (void*&) s_ ##func);  \
    if(res != sl::Result::eOk) return res;                                      \
}                                                                               \

