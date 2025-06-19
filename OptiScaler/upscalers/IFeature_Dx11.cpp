#include "IFeature_Dx11.h"

#include <State.h>

void IFeature_Dx11::Shutdown()
{
    if (Imgui != nullptr || Imgui.get() != nullptr)
        Imgui.reset();
}

IFeature_Dx11::~IFeature_Dx11()
{
    if (State::Instance().isShuttingDown)
        return;

    if (Imgui != nullptr && Imgui.get() != nullptr)
    {
        Imgui.reset();
        Imgui = nullptr;
    }

    if (OutputScaler != nullptr && OutputScaler.get() != nullptr)
    {
        OutputScaler.reset();
        OutputScaler = nullptr;
    }

    if (RCAS != nullptr && RCAS.get() != nullptr)
    {
        RCAS.reset();
        RCAS = nullptr;
    }

    if (Bias != nullptr && Bias.get() != nullptr)
    {
        Bias.reset();
        Bias = nullptr;
    }
}
