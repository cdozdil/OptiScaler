#pragma once
#include "DLSSFeature.h"
#include "../IFeature_Dx12.h"
//#include "../../cas/CAS_Dx12.h"
#include "../../rcas/RCAS_Dx12.h"
#include <string>

typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_Init_Ext)(unsigned long long InApplicationId, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_Init_ProjectID)(const char* InProjectId, NVSDK_NGX_EngineType InEngineType, const char* InEngineVersion, const wchar_t* InApplicationDataPath, ID3D12Device* InDevice, NVSDK_NGX_Version InSDKVersion, const NVSDK_NGX_FeatureCommonInfo* InFeatureInfo);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_Shutdown)(void);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_Shutdown1)(ID3D12Device* InDevice);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_GetParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_AllocateParameters)(NVSDK_NGX_Parameter** OutParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_DestroyParameters)(NVSDK_NGX_Parameter* InParameters);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_CreateFeature)(ID3D12GraphicsCommandList* InCmdList, NVSDK_NGX_Feature InFeatureID, NVSDK_NGX_Parameter* InParameters, NVSDK_NGX_Handle** OutHandle);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_ReleaseFeature)(NVSDK_NGX_Handle* InHandle);
typedef NVSDK_NGX_Result(*PFN_NVSDK_NGX_D3D12_EvaluateFeature)(ID3D12GraphicsCommandList* InCmdList, const NVSDK_NGX_Handle* InFeatureHandle, const NVSDK_NGX_Parameter* InParameters, PFN_NVSDK_NGX_ProgressCallback InCallback);

class DLSSFeatureDx12 : public DLSSFeature, public IFeature_Dx12
{
private:
	inline static PFN_NVSDK_NGX_D3D12_Init_ProjectID _Init_with_ProjectID = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_Init_Ext _Init_Ext = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_Shutdown _Shutdown = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_Shutdown1 _Shutdown1 = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_GetParameters _GetParameters = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_AllocateParameters _AllocateParameters = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_DestroyParameters _DestroyParameters = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_CreateFeature _CreateFeature = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_ReleaseFeature _ReleaseFeature = nullptr;
	inline static PFN_NVSDK_NGX_D3D12_EvaluateFeature _EvaluateFeature = nullptr;
	
	float GetSharpness(const NVSDK_NGX_Parameter* InParameters);

protected:


public:
	bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters) override;
	bool Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters) override;

	static void Shutdown(ID3D12Device* InDevice);

	DLSSFeatureDx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters);
	~DLSSFeatureDx12();
};