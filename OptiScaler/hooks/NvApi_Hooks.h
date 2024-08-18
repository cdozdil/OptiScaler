#pragma once

namespace Hooks
{
    void AttachNVAPISpoofingHooks();
    void DetachNVAPISpoofingHooks();

    bool IsArchSupportsDLSS();
    void NvApiCheckDLSSSupport();
}