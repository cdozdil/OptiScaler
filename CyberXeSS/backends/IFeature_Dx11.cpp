#include "IFeature_Dx11.h"

void IFeature_Dx11::Shutdown()
{
	if (Imgui == nullptr || Imgui.get() == nullptr)
		Imgui.reset();
}

IFeature_Dx11::~IFeature_Dx11()
{
	if (Device)
	{
		D3D11_QUERY_DESC pQueryDesc;
		pQueryDesc.Query = D3D11_QUERY_EVENT;
		pQueryDesc.MiscFlags = 0;

		ID3D11Query* query = nullptr;
		auto result = Device->CreateQuery(&pQueryDesc, &query);

		if (result == S_OK)
		{
			// Associate the query with the copy operation
			DeviceContext->Begin(query);

			// Execute dx11 commands 
			DeviceContext->End(query);
			DeviceContext->Flush();

			// Wait for the query to be ready
			while (DeviceContext->GetData(query, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_FALSE)
				std::this_thread::yield();

			// Release the query
			query->Release();
		}
	}
}