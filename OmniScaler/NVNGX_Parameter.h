#pragma once
#include "pch.h"
#include <ankerl/unordered_dense.h>
#include "Config.h"

inline std::optional<float> GetQualityOverrideRatio(const NVSDK_NGX_PerfQuality_Value input)
{
	std::optional<float> output;

	if (Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false))
	{
		output = Config::Instance()->UpscaleRatioOverrideValue.value_or(1.3f);
		return  output;
	}

	if (!Config::Instance()->QualityRatioOverrideEnabled.value_or(false))
		return output; // override not enabled

	switch (input)
	{
	case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
		output = Config::Instance()->QualityRatio_UltraPerformance.value_or(3.0);
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxPerf:
		output = Config::Instance()->QualityRatio_Performance.value_or(2.0);
		break;
	case NVSDK_NGX_PerfQuality_Value_Balanced:
		output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);
		break;
	case NVSDK_NGX_PerfQuality_Value_MaxQuality:
		output = Config::Instance()->QualityRatio_Quality.value_or(1.5);
		break;
	case NVSDK_NGX_PerfQuality_Value_UltraQuality:
		output = Config::Instance()->QualityRatio_UltraQuality.value_or(1.3);
		break;
	case NVSDK_NGX_PerfQuality_Value_DLAA:
		output = 1.0f;
		break;
	default:
		spdlog::warn("GetQualityOverrideRatio: Unknown quality: {0}", (int)input);
		output = Config::Instance()->QualityRatio_Balanced.value_or(1.7);
		break;
	}
	return output;
}

inline NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetOptimalSettingsCallback(NVSDK_NGX_Parameter* InParams)
{
	unsigned int Width;
	unsigned int Height;
	unsigned int OutWidth;
	unsigned int OutHeight;
	float scalingRatio = 0.0f;
	int PerfQualityValue;

	if (InParams->Get(NVSDK_NGX_Parameter_Width, &Width) != NVSDK_NGX_Result_Success)
		return NVSDK_NGX_Result_Fail;

	if (InParams->Get(NVSDK_NGX_Parameter_Height, &Height) != NVSDK_NGX_Result_Success)
		return NVSDK_NGX_Result_Fail;

	if (InParams->Get(NVSDK_NGX_Parameter_PerfQualityValue, &PerfQualityValue) != NVSDK_NGX_Result_Success)
		return NVSDK_NGX_Result_Fail;

	bool usingXess = (Config::Instance()->Api == NVNGX_DX11 && Config::Instance()->Dx11Upscaler.value_or("fsr22") == "xess") ||
		(Config::Instance()->Api == NVNGX_DX12 && Config::Instance()->Dx12Upscaler.value_or("xess") == "xess");

	auto enumPQValue = (NVSDK_NGX_PerfQuality_Value)PerfQualityValue;

	spdlog::debug("NVSDK_NGX_DLSS_GetOptimalSettingsCallback Output Resolution: {0}x{1}", Width, Height);

	const std::optional<float> QualityRatio = GetQualityOverrideRatio(enumPQValue);

	if (QualityRatio.has_value()) {
		OutHeight = (unsigned int)((float)Height / QualityRatio.value());
		OutWidth = (unsigned int)((float)Width / QualityRatio.value());
		scalingRatio = 1.0f / QualityRatio.value();
	}
	else {
		spdlog::debug("NVSDK_NGX_DLSS_GetOptimalSettingsCallback Quality: {0}", PerfQualityValue);

		switch (enumPQValue)
		{
		case NVSDK_NGX_PerfQuality_Value_UltraPerformance:
			OutHeight = (unsigned int)((float)Height / 3.0);
			OutWidth = (unsigned int)((float)Width / 3.0);
			scalingRatio = 0.33333333f;
			break;

		case NVSDK_NGX_PerfQuality_Value_MaxPerf:
			OutHeight = (unsigned int)((float)Height / 2.0);
			OutWidth = (unsigned int)((float)Width / 2.0);
			scalingRatio = 0.5f;
			break;

		case NVSDK_NGX_PerfQuality_Value_Balanced:
			OutHeight = (unsigned int)((float)Height / 1.7);
			OutWidth = (unsigned int)((float)Width / 1.7);
			scalingRatio = 1.0f / 1.7f;
			break;

		case NVSDK_NGX_PerfQuality_Value_MaxQuality:
			OutHeight = (unsigned int)((float)Height / 1.5);
			OutWidth = (unsigned int)((float)Width / 1.5);
			scalingRatio = 1.0f / 1.5f;
			break;

		case NVSDK_NGX_PerfQuality_Value_UltraQuality:
			OutHeight = (unsigned int)((float)Height / 1.3);
			OutWidth = (unsigned int)((float)Width / 1.3);
			scalingRatio = 1.0f / 1.3f;
			break;

		case NVSDK_NGX_PerfQuality_Value_DLAA:
			OutHeight = Height;
			OutWidth = Width;
			scalingRatio = 1.0f;
			break;

		default:
			OutHeight = (unsigned int)((float)Height / 1.7);
			OutWidth = (unsigned int)((float)Width / 1.7);
			scalingRatio = 1.0f / 1.7f;
			break;
		}
	}

	InParams->Set(NVSDK_NGX_Parameter_Scale, scalingRatio);
	InParams->Set(NVSDK_NGX_Parameter_SuperSampling_ScaleFactor, scalingRatio);
	InParams->Set(NVSDK_NGX_Parameter_OutWidth, OutWidth);
	InParams->Set(NVSDK_NGX_Parameter_OutHeight, OutHeight);

	if (Config::Instance()->QualityRatioOverrideEnabled.value_or(false) || 
		Config::Instance()->UpscaleRatioOverrideEnabled.value_or(false) || 
		Config::Instance()->SuperSamplingEnabled.value_or(false))
	{
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width, OutWidth);
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height, OutHeight);
	}
	else if (enumPQValue == NVSDK_NGX_PerfQuality_Value_DLAA)
	{
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width, Width);
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height, Height);
	}
	else
	{
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Width, (unsigned int)((float)Width / 3.0f));
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Min_Render_Height, (unsigned int)((float)Height / 3.0f));
	}

	if (Config::Instance()->DrsMaxOverrideEnabled.value_or(false))
	{
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Width, OutWidth);
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Height, OutHeight);
	}
	else
	{
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Width, Width);
		InParams->Set(NVSDK_NGX_Parameter_DLSS_Get_Dynamic_Max_Render_Height, Height);
	}

	InParams->Set(NVSDK_NGX_Parameter_SizeInBytes, Width * Height * 31);
	InParams->Set(NVSDK_NGX_Parameter_DLSSMode, NVSDK_NGX_DLSS_Mode_DLSS);

	InParams->Set(NVSDK_NGX_EParameter_Scale, scalingRatio);
	InParams->Set(NVSDK_NGX_EParameter_OutWidth, OutWidth);
	InParams->Set(NVSDK_NGX_EParameter_OutHeight, OutHeight);
	InParams->Set(NVSDK_NGX_EParameter_SizeInBytes, Width * Height * 31);
	InParams->Set(NVSDK_NGX_EParameter_DLSSMode, NVSDK_NGX_DLSS_Mode_DLSS);

	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance, (uint32_t)NVSDK_NGX_DLSS_Hint_Render_Preset_A);

	spdlog::debug("NVSDK_NGX_DLSS_GetOptimalSettingsCallback: Output Resolution: {0}x{1} Render Resolution: {2}x{3}", Width, Height, OutWidth, OutHeight);
	return NVSDK_NGX_Result_Success;
}

inline NVSDK_NGX_Result NVSDK_CONV NVSDK_NGX_DLSS_GetStatsCallback(NVSDK_NGX_Parameter* InParams)
{
	spdlog::debug("NVSDK_NGX_DLSS_GetStatsCallback");

	if (!InParams)
		return NVSDK_NGX_Result_Success;

	unsigned int Width = 1920;
	unsigned int Height = 1080;

	InParams->Get(NVSDK_NGX_Parameter_Width, &Width);
	InParams->Get(NVSDK_NGX_Parameter_Height, &Height);
	InParams->Set(NVSDK_NGX_Parameter_SizeInBytes, Width * Height * 31);

	return NVSDK_NGX_Result_Success;
}

inline void InitNGXParameters(NVSDK_NGX_Parameter* InParams)
{
	InParams->Set(NVSDK_NGX_Parameter_SuperSampling_Available, 1);

	if (Config::Instance()->NVNGX_Engine == NVSDK_NGX_ENGINE_TYPE_UNREAL)
	{
		InParams->Set(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, 10);
		InParams->Set(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, 10);
	}
	else
	{
		InParams->Set(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, 0);
		InParams->Set(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, 0);
	}

	InParams->Set(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, 0);
	InParams->Set(NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult, 1);
	InParams->Set(NVSDK_NGX_Parameter_OptLevel, 0);
	InParams->Set(NVSDK_NGX_Parameter_IsDevSnippetBranch, 0);
	InParams->Set(NVSDK_NGX_Parameter_DLSSOptimalSettingsCallback, NVSDK_NGX_DLSS_GetOptimalSettingsCallback);
	InParams->Set(NVSDK_NGX_Parameter_DLSSGetStatsCallback, NVSDK_NGX_DLSS_GetStatsCallback);
	InParams->Set(NVSDK_NGX_Parameter_Sharpness, 0.0f);
	InParams->Set(NVSDK_NGX_Parameter_MV_Scale_X, 1.0f);
	InParams->Set(NVSDK_NGX_Parameter_MV_Scale_Y, 1.0f);
	InParams->Set(NVSDK_NGX_Parameter_MV_Offset_X, 0.0f);
	InParams->Set(NVSDK_NGX_Parameter_MV_Offset_Y, 0.0f);
	InParams->Set(NVSDK_NGX_Parameter_DLSS_Exposure_Scale, 1.0f);
	InParams->Set(NVSDK_NGX_Parameter_SizeInBytes, 1920 * 1080 * 31);

	InParams->Set(NVSDK_NGX_EParameter_SuperSampling_Available, 1);
	InParams->Set(NVSDK_NGX_EParameter_OptLevel, 0);
	InParams->Set(NVSDK_NGX_EParameter_IsDevSnippetBranch, 0);
	InParams->Set(NVSDK_NGX_EParameter_DLSSOptimalSettingsCallback, NVSDK_NGX_DLSS_GetOptimalSettingsCallback);
	InParams->Set(NVSDK_NGX_EParameter_Sharpness, 0.0f);
	InParams->Set(NVSDK_NGX_EParameter_MV_Scale_X, 1.0f);
	InParams->Set(NVSDK_NGX_EParameter_MV_Scale_Y, 1.0f);
	InParams->Set(NVSDK_NGX_EParameter_MV_Offset_X, 0.0f);
	InParams->Set(NVSDK_NGX_EParameter_MV_Offset_Y, 0.0f);
}

struct Parameter
{
	template<typename T>
	void operator=(T value)
	{
		key = typeid(T).hash_code();
		if constexpr (std::is_same<T, float>::value) values.f = value;
		else if constexpr (std::is_same<T, int>::value) values.i = value;
		else if constexpr (std::is_same<T, unsigned int>::value) values.ui = value;
		else if constexpr (std::is_same<T, double>::value) values.d = value;
		else if constexpr (std::is_same<T, unsigned long long>::value) values.ull = value;
		else if constexpr (std::is_same<T, void*>::value) values.vp = value;
		else if constexpr (std::is_same<T, ID3D11Resource*>::value) values.d11r = value;
		else if constexpr (std::is_same<T, ID3D12Resource*>::value) values.d12r = value;
	}

	template<typename T>
	operator T() const
	{
		T v = {};
		if constexpr (std::is_same<T, float>::value)
		{
			if (key == typeid(unsigned long long).hash_code()) v = (T)values.ull;
			else if (key == typeid(float).hash_code()) v = (T)values.f;
			else if (key == typeid(double).hash_code()) v = (T)values.d;
			else if (key == typeid(unsigned int).hash_code()) v = (T)values.ui;
			else if (key == typeid(int).hash_code()) v = (T)values.i;
		}
		else if constexpr (std::is_same<T, int>::value)
		{
			if (key == typeid(unsigned long long).hash_code()) v = (T)values.ull;
			else if (key == typeid(float).hash_code()) v = (T)values.f;
			else if (key == typeid(double).hash_code()) v = (T)values.d;
			else if (key == typeid(int).hash_code()) v = (T)values.i;
			else if (key == typeid(unsigned int).hash_code()) v = (T)values.ui;
		}
		else if constexpr (std::is_same<T, unsigned int>::value)
		{
			if (key == typeid(unsigned long long).hash_code()) v = (T)values.ull;
			else if (key == typeid(float).hash_code()) v = (T)values.f;
			else if (key == typeid(double).hash_code()) v = (T)values.d;
			else if (key == typeid(int).hash_code()) v = (T)values.i;
			else if (key == typeid(unsigned int).hash_code()) v = (T)values.ui;
		}
		else if constexpr (std::is_same<T, double>::value)
		{
			if (key == typeid(unsigned long long).hash_code()) v = (T)values.ull;
			else if (key == typeid(float).hash_code()) v = (T)values.f;
			else if (key == typeid(double).hash_code()) v = (T)values.d;
			else if (key == typeid(int).hash_code()) v = (T)values.i;
			else if (key == typeid(unsigned int).hash_code()) v = (T)values.ui;
		}
		else if constexpr (std::is_same<T, unsigned long long>::value)
		{
			if (key == typeid(unsigned long long).hash_code()) v = (T)values.ull;
			else if (key == typeid(float).hash_code()) v = (T)values.f;
			else if (key == typeid(double).hash_code()) v = (T)values.d;
			else if (key == typeid(int).hash_code()) v = (T)values.i;
			else if (key == typeid(unsigned int).hash_code()) v = (T)values.ui;
			else if (key == typeid(void*).hash_code()) v = (T)values.vp;
		}
		else if constexpr (std::is_same<T, void*>::value)
		{
			if (key == typeid(void*).hash_code()) v = values.vp;
		}
		else if constexpr (std::is_same<T, ID3D11Resource*>::value)
		{
			if (key == typeid(ID3D11Resource*).hash_code()) v = values.d11r;
			else if (key == typeid(void*).hash_code()) v = (T)values.vp;
		}
		else if constexpr (std::is_same<T, ID3D12Resource*>::value)
		{
			if (key == typeid(ID3D12Resource*).hash_code()) v = values.d12r;
			else if (key == typeid(void*).hash_code()) v = (T)values.vp;
		}

		return v;
	}

	union
	{
		float f;
		double d;
		int i;
		unsigned int ui;
		unsigned long long ull;
		void* vp;
		ID3D11Resource* d11r;
		ID3D12Resource* d12r;
	} values;

	size_t key = 0;
};

struct NVNGX_Parameters : public NVSDK_NGX_Parameter
{
	void Set(const char* key, unsigned long long value) override { spdlog::trace("NVNGX_Parameters::Set ulong('{0}', {1})", key, value); setT(key, value); }
	void Set(const char* key, float value) override { spdlog::trace("NVNGX_Parameters::Set float('{0}', {1})", key, value); setT(key, value); }
	void Set(const char* key, double value) override { spdlog::trace("NVNGX_Parameters::Set double('{0}', {1})", key, value); setT(key, value); }
	void Set(const char* key, unsigned int value) override { spdlog::trace("NVNGX_Parameters::Set uint('{0}', {1})", key, value); setT(key, value); }
	void Set(const char* key, int value) override { spdlog::trace("NVNGX_Parameters::Set int('{0}', {1})", key, value); setT(key, value); }
	void Set(const char* key, void* value) override { spdlog::trace("NVNGX_Parameters::Set void('{0}', '{1}null')", key, value == nullptr ? "" : "not "); setT(key, value); }
	void Set(const char* key, ID3D11Resource* value) override { spdlog::trace("NVNGX_Parameters::Set d3d11('{0}', '{1}null')", key, value == nullptr ? "" : "not "); setT(key, value); }
	void Set(const char* key, ID3D12Resource* value) override { spdlog::trace("NVNGX_Parameters::Set d3d12('{0}', '{1}null')", key, value == nullptr ? "" : "not "); setT(key, value); }

	NVSDK_NGX_Result Get(const char* key, unsigned long long* value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get ulong('{0}', {1})", key, *value); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, float* value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get float('{0}', {1})", key, *value); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, double* value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get double('{0}', {1})", key, *value); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, unsigned int* value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get uint('{0}', {1})", key, *value); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, int* value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get int('{0}', {1})", key, *value); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, void** value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get void('{0}')", key); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, ID3D11Resource** value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get d3d11('{0}')", key); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }
	NVSDK_NGX_Result Get(const char* key, ID3D12Resource** value) const override { auto result = getT(key, value); if (result == NVSDK_NGX_Result_Success) { spdlog::trace("NVNGX_Parameters::Get d3d12('{0}')", key); return NVSDK_NGX_Result_Success; } return NVSDK_NGX_Result_Fail; }

	void Reset() override
	{
		if (!m_values.empty())
			m_values.clear();

		spdlog::debug("NVNGX_Parameters::Reset Start");

		InitNGXParameters(this);

		spdlog::debug("NVNGX_Parameters::Reset End");
	}

	std::vector<std::string> enumerate() const
	{
		std::vector<std::string> keys;
		for (auto& value : m_values)
		{
			keys.push_back(value.first);
		}
		return keys;
	}

private:
	ankerl::unordered_dense::map<std::string, Parameter> m_values;
	mutable std::mutex m_mutex;

	template<typename T>
	void setT(const char* key, T& value)
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		m_values[key] = value;
	}

	template<typename T>
	NVSDK_NGX_Result getT(const char* key, T* value) const
	{
		const std::lock_guard<std::mutex> lock(m_mutex);
		auto k = m_values.find(key);

		if (k == m_values.end())
		{
			spdlog::debug("NVNGX_Parameters::GetT('{0}', FAIL)", key);
			return NVSDK_NGX_Result_Fail;
		};

		const Parameter& p = (*k).second;
		*value = p;

		return NVSDK_NGX_Result_Success;
	}


};

inline NVSDK_NGX_Parameter* GetNGXParameters()
{
	auto params = new NVNGX_Parameters();
	InitNGXParameters(params);

	return params;
}

