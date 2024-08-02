#include "IFeature_Dx11.h"

void IFeature_Dx11::Shutdown()
{
	if (Imgui != nullptr || Imgui.get() != nullptr)
		Imgui.reset();
}

IFeature_Dx11::~IFeature_Dx11()
{
}