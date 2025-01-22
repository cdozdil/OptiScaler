#pragma once

#include "ffx_api.h"

ffxReturnCode_t ffxCreateContext_Dx12(ffxContext* context, ffxCreateContextDescHeader* desc, const ffxAllocationCallbacks* memCb);
ffxReturnCode_t ffxDestroyContext_Dx12(ffxContext* context, const ffxAllocationCallbacks* memCb);
ffxReturnCode_t ffxConfigure_Dx12(ffxContext* context, const ffxConfigureDescHeader* desc);
ffxReturnCode_t ffxQuery_Dx12(ffxContext* context, ffxQueryDescHeader* desc);
ffxReturnCode_t ffxDispatch_Dx12(ffxContext* context, ffxDispatchDescHeader* desc);