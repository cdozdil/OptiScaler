#pragma once

#include "imgui_dx_base.h"

#include <d3d11.h>

class Imgui_Dx11 : public ImguiDxBase
{
private:
	bool _dx11Init = false;
	ID3D11Device* _device = nullptr;
	
	ID3D11Texture2D* _renderTargetTexture = nullptr;
	ID3D11RenderTargetView* _renderTargetView = nullptr;

	void CreateRenderTarget(ID3D11Resource* out);

public:
	bool Render(ID3D11DeviceContext* pCmdList, ID3D11Resource* outTexture);

	Imgui_Dx11(HWND handle, ID3D11Device* pDevice);

	~Imgui_Dx11();
};