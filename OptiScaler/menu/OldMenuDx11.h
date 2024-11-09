#pragma once

#include "OldMenuBase.h"

#include <d3d11.h>

class OldMenuDx11 : public OldMenuBase
{
private:
	bool _dx11Init = false;
	ID3D11Device* _device = nullptr;
	
	ID3D11Texture2D* _renderTargetTexture = nullptr;
	ID3D11RenderTargetView* _renderTargetView = nullptr;

	void CreateRenderTarget(ID3D11Resource* out);

public:
	bool Render(ID3D11DeviceContext* pCmdList, ID3D11Resource* outTexture);

	OldMenuDx11(HWND handle, ID3D11Device* pDevice);

	~OldMenuDx11();
};