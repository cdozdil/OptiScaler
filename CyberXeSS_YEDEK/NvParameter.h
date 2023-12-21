#pragma once
#include "pch.h"

enum NvParameterType {
	NvInt,
	NvFloat,
	NvDouble,
	NvUInt,
	NvULL,
	NvD3D11Resource,
	NvD3D12Resource,
	NvVoidPtr
};

struct NvParameter : NVSDK_NGX_Parameter
{
	unsigned int Width{}, Height{}, OutWidth{}, OutHeight{};
	NVSDK_NGX_PerfQuality_Value PerfQualityValue = NVSDK_NGX_PerfQuality_Value_Balanced;
	bool RTXValue{}, FreeMemOnReleaseFeature{};
	int CreationNodeMask{}, VisibilityNodeMask{}, OptLevel{}, IsDevSnippetBranch{};
	int DLAAPreset{}, UltraQualityPreset{}, QualityPreset{}, BalancedPreset{}, PerfPreset{}, UltraPerfPreset{};
	float Sharpness = 1.0f;
	float ExposureScale = 1.0f;
	int ResetRender = 0;
	float MVScaleX = 1.0, MVScaleY = 1.0;
	float JitterOffsetX{}, JitterOffsetY{};

	long long SizeInBytes{};

	bool DepthInverted{}, AutoExposure{}, Hdr{}, EnableSharpening{}, JitterMotion{}, LowRes{};

	//external Resources
	void* InputBiasCurrentColorMask{};
	void* Color{};
	void* Depth{};
	void* MotionVectors{};
	void* Output{};
	void* TransparencyMask{};
	void* ExposureTexture{};

	virtual void Set(const char* InName, unsigned long long InValue) override;
	virtual void Set(const char* InName, float InValue) override;
	virtual void Set(const char* InName, double InValue) override;
	virtual void Set(const char* InName, unsigned int InValue) override;
	virtual void Set(const char* InName, int InValue) override;
	virtual void Set(const char* InName, ID3D11Resource* InValue) override;
	virtual void Set(const char* InName, ID3D12Resource* InValue) override;
	virtual void Set(const char* InName, void* InValue) override;
	virtual NVSDK_NGX_Result Get(const char* InName, unsigned long long* OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, float* OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, double* OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, unsigned int* OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, int* OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, ID3D11Resource** OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, ID3D12Resource** OutValue) const override;
	virtual NVSDK_NGX_Result Get(const char* InName, void** OutValue) const override;
	virtual void Reset() override;

	void Set_Internal(const char* InName, unsigned long long InValue, NvParameterType ParameterType);
	NVSDK_NGX_Result Get_Internal(const char* InName, unsigned long long* OutValue, NvParameterType ParameterType) const;

	void EvaluateRenderScale();

	std::vector<std::shared_ptr<NvParameter>> Params;

	__declspec(noinline) NvParameter* AllocateParameters()
	{
		Params.push_back(std::make_shared<NvParameter>());
		return Params.back().get();
	}

	__declspec(noinline) void DeleteParameters(NvParameter* param)
	{
		auto it = std::find_if(Params.begin(), Params.end(),
			[param](const auto& p) { return p.get() == param; });
		Params.erase(it);
	}

	static std::shared_ptr<NvParameter> instance()
	{
		static std::shared_ptr<NvParameter> INSTANCE { std::make_shared<NvParameter>() };
		return INSTANCE;
	}
};
