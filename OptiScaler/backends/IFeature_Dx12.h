#pragma once
#include <d3d12.h>
#include "IFeature.h"

#include "../pch.h"
#include "../Util.h"
#include "../imgui/Imgui_Dx12.h"
#include "../bicubicscaling/BS_Dx12.h"

class IFeature_Dx12 : public virtual IFeature
{
protected:
	ID3D12Device* Device = nullptr;
	static inline std::unique_ptr<Imgui_Dx12> Imgui = nullptr;
	std::unique_ptr<BS_Dx12> OutputScaler = nullptr;

	void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState) const;

public:
	virtual bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters) = 0;
	virtual bool Evaluate(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter* InParameters) = 0;

	IFeature_Dx12(unsigned int InHandleId, const NVSDK_NGX_Parameter* InParameters);

	void Shutdown() final;

	~IFeature_Dx12();
};
