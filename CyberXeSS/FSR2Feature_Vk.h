#include <vulkan/vulkan.hpp>

#include "FidelityFX/host/backends/vk/ffx_vk.h"

#include "FSR2Feature.h"
#include "IFeature_Vk.h"

#ifdef _DEBUG
#pragma comment(lib, "FidelityFX/lib/ffx_backend_vk_x64d.lib")
#else
#pragma comment(lib, "FidelityFX/lib/ffx_backend_vk_x64.lib")
#endif // DEBUG

#ifdef FidelityFX
#pragma comment(lib, "FidelityFX/lib/ffx_fsr2_x64d.lib")
#else
#pragma comment(lib, "FidelityFX/lib/ffx_fsr2_x64.lib")
#endif // DEBUG


class FSR2FeatureVk : public FSR2Feature, public IFeature_Vk
{
private:
	VkDeviceContext _deviceContext{};
	FfxDevice _ffxDevice = nullptr;
	size_t _scratchBufferSize = 0;

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters) override;
};
