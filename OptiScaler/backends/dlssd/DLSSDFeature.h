#pragma once

#include "../../pch.h"
#include "../../NVNGX_Proxy.h"
#include "../IFeature.h"

typedef uint32_t(*PFN_NVSDK_NGX_GetSnippetVersion)(void);

class DLSSDFeature : public virtual IFeature
{
private:
	feature_version _version = { 0, 0, 0 };
		
protected:
	NVSDK_NGX_Handle _dlssdHandle = {};
	NVSDK_NGX_Handle* _p_dlssdHandle = nullptr;
	inline static bool _dlssdInited = false;

	void ProcessEvaluateParams(NVSDK_NGX_Parameter* InParameters);
	void ProcessInitParams(NVSDK_NGX_Parameter* InParameters);

	static void Shutdown();

public:
	feature_version Version() final { return feature_version{ _version.major, _version.minor, _version.patch }; }
	const char* Name() final { return "DLSSD"; } 
	void ReadVersion();

	DLSSDFeature(unsigned int handleId, NVSDK_NGX_Parameter* InParameters);

	~DLSSDFeature();
};