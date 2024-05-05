#pragma once
#include "../IFeature_Vk.h"
#include "DLSSFeature.h"
#include <string>
#include "nvsdk_ngx_vk.h"

typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_Init)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_Shutdown1)(VkDevice InDevice);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_CreateFeature)(VkCommandBuffer InCmdBuffer, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_VULKAN_EvaluateFeature)(VkCommandBuffer InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

class DLSSFeatureVk : public DLSSFeature, public IFeature_Vk
{
private:
	inline static PFN_NVSDK_NGX_VULKAN_Init_ProjectID _Init_with_ProjectID = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_Init _Init_Ext = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_Shutdown _Shutdown = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_Shutdown1 _Shutdown1 = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_GetParameters _GetParameters = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_AllocateParameters _AllocateParameters = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_DestroyParameters _DestroyParameters = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_CreateFeature _CreateFeature = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_ReleaseFeature _ReleaseFeature = nullptr;
	inline static PFN_NVSDK_NGX_VULKAN_EvaluateFeature _EvaluateFeature = nullptr;

protected:


public:
	bool Init(VkInstance InInstance, VkPhysicalDevice InPD, VkDevice InDevice, VkCommandBuffer InCmdList, PFN_vkGetInstanceProcAddr InGIPA, PFN_vkGetDeviceProcAddr InGDPA, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(VkCommandBuffer InCmdBuffer, const NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(VkDevice InDevice);

	DLSSFeatureVk(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters);
	~DLSSFeatureVk();
};