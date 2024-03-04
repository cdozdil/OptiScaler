#include <vulkan/vulkan.hpp>

#include "ffx3/include/FidelityFX/host/backends/vk/ffx_vk.h"

#include "FSR2Feature.h"
#include "IFeature_Vk.h"

#ifdef _DEBUG
#pragma comment(lib, "ffx3/lib/ffx_backend_vk_x64d.lib")
#else
#pragma comment(lib, "ffx3/lib/ffx_backend_vk_x64.lib")
#endif // DEBUG

#ifdef _DEBUG
#pragma comment(lib, "ffx3/lib/ffx_fsr2_x64d.lib")
#else
#pragma comment(lib, "ffx3/lib/ffx_fsr2_x64.lib")
#endif // DEBUG


class FSR2FeatureVk : public FSR2Feature, public IFeature_Vk
{
private:

protected:
	bool InitFSR2(const NVSDK_NGX_Parameter* InParameters);

public:
	FSR2FeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters) : FSR2Feature(InHandleId, InParameters), IFeature_Vk(InHandleId, InParameters), IFeature(InHandleId, InParameters)
	{
	}

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters) override;
};
