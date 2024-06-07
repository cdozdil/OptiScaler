#pragma once

#include "../IFeature.h"
#include "../../pch.h"
#include "../../NVNGX_Parameter.h"
#include <string>

typedef uint32_t(*PFN_NVSDK_NGX_GetSnippetVersion)(void);

class DLSSFeature : public virtual IFeature
{
private:
	feature_version _version = { 0, 0, 0 };
	inline static HMODULE _nvngx = nullptr;
		
protected:
	NVSDK_NGX_Parameter* Parameters = GetNGXParameters("DLSS");
	NVSDK_NGX_Handle _dlssHandle = {};
	NVSDK_NGX_Handle* _p_dlssHandle = nullptr;
	inline static bool _dlssInited = false;

	HMODULE NVNGX() { return _nvngx; }

	void ProcessEvaluateParams(const NVSDK_NGX_Parameter* InParameters);
	void ProcessInitParams(const NVSDK_NGX_Parameter* InParameters);
	void GetFeatureCommonInfo(NVSDK_NGX_FeatureCommonInfo* fcInfo);

	static void Shutdown();

public:
	feature_version Version() final { return feature_version{ _version.major, _version.minor, _version.patch }; }
	const char* Name() final { return "DLSS"; } 
	void ReadVersion();

	DLSSFeature(unsigned int handleId, const NVSDK_NGX_Parameter* InParameters);

	~DLSSFeature();
};