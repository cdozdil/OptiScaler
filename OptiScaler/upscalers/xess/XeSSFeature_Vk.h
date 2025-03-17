#pragma once

#include <pch.h>

#include <proxies/XeSS_Proxy.h>
#include <upscalers/IFeature_Vk.h>

#include <xess_vk.h>

class XeSSFeature_Vk : public virtual IFeature_Vk
{
private:
	std::string _version = "1.3.0";

protected:
	xess_context_handle_t _xessContext = nullptr;
	int dumpCount = 0;

public:
	// version is above 1.3 if we can use vulkan
	feature_version Version() final { return feature_version{ XeSSProxy::Version().major, XeSSProxy::Version().minor, XeSSProxy::Version().patch }; }
	const char* Name() override { return "XeSS"; }

	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Parameter* InParameters) override;

	XeSSFeature_Vk(unsigned int handleId, NVSDK_NGX_Parameter* InParameters);
	~XeSSFeature_Vk();
};