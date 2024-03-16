//#include "pch.h"
//#include "Config.h"
//#include "CyberXess.h"
//#include "Util.h"
//
//FeatureContext* CyberXessContext::CreateContext()
//{
//	auto handleId = handleCounter++;
//	Contexts[handleId] = std::make_unique<FeatureContext>();
//	Contexts[handleId]->Handle.Id = handleId;
//	return Contexts[handleId].get();
//}
//
//void CyberXessContext::DeleteContext(NVSDK_NGX_Handle* handle)
//{
//	if (handle == nullptr || handle == NULL)
//		return;
//
//	auto handleId = handle->Id;
//	auto it = std::find_if(Contexts.begin(), Contexts.end(), [&handleId](const auto& p) { return p.first == handleId; });
//
//	Contexts.erase(it);
//}
//
//CyberXessContext::CyberXessContext()
//{
//	MyConfig = std::make_unique<Config>(L"nvngx.ini");
//}
//
