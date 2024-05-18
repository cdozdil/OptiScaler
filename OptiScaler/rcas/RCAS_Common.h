#pragma once

#include "../pch.h"
#include <d3dcompiler.h>

struct RcasConstants
{
	float Sharpness;

	// Motion Vector Stuff
	bool DynamicSharpenEnabled;
	bool DisplaySizeMV;
	bool Debug;

	float MotionSharpness;
	float MotionTextureScale;
	float MvScaleX;
	float MvScaleY;
	float Threshold;
	float ScaleLimit;

	int RenderWidth;
	int RenderHeight;
	int DisplayWidth;
	int DisplayHeight;
};

static ID3DBlob* CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		spdlog::error("CompileShader error while compiling shader");

		if (errorBlob)
		{
			spdlog::error("CompileShader error while compiling shader : {0}", (char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return nullptr;
	}

	if (errorBlob)
		errorBlob->Release();

	return shaderBlob;
}