#pragma once
#include <d3d12.h>
#include "IFeature.h"

#include "../pch.h"
#include "../Util.h"
#include "../imgui/Imgui_Dx12.h"

#include "../bicubicscaling/BS_Dx12.h"
#include "../rcas/RCAS_Dx12.h"

class IFeature_Dx12 : public virtual IFeature
{
protected:
	ID3D12Device* Device = nullptr;

	static inline std::unique_ptr<Imgui_Dx12> Imgui = nullptr;
	std::unique_ptr<BS_Dx12> OutputScaler = nullptr;
	std::unique_ptr<RCAS_Dx12> RCAS = nullptr;

	void ResourceBarrier(ID3D12GraphicsCommandList* InCommandList, ID3D12Resource* InResource, D3D12_RESOURCE_STATES InBeforeState, D3D12_RESOURCE_STATES InAfterState) const;
	bool BeforeEvaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams, ID3D12Resource* OutputTexture);
	void AfterEvaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams, ID3D12Resource* OutputTexture);

public:
	virtual bool Init(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCommandList) = 0;
	virtual bool Evaluate(ID3D12GraphicsCommandList* InCommandList, const IFeatureEvaluateParams* InParams) = 0;

	IFeature_Dx12(unsigned int InHandleId, const IFeatureCreateParams InParameters);

	void Shutdown() final;

	~IFeature_Dx12();
};
