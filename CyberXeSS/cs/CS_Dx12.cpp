#include "CS_Dx12.h"

ID3DBlob* CompileShader(const char* shaderCode, const char* entryPoint, const char* target)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT hr = D3DCompile(shaderCode, strlen(shaderCode), nullptr, nullptr, nullptr, entryPoint, target, 0, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
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

bool CreateComputeShader(ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3D12PipelineState** pipelineState, ID3DBlob* shaderBlob)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature;
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState));

	if (FAILED(hr))
	{
		// Handle error
		return false;
	}

	return true;
}

bool CS_Dx12::CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState)
{
	if (InDevice == nullptr || InSource == nullptr)
		return false;

	D3D12_RESOURCE_DESC texDesc = InSource->GetDesc();

	if (_buffer != nullptr)
	{
		auto bufDesc = _buffer->GetDesc();

		if (bufDesc.Width != texDesc.Width || bufDesc.Height != texDesc.Height || bufDesc.Format != texDesc.Format)
		{
			_buffer->Release();
			_buffer = nullptr;
		}
		else
			return true;
	}

	spdlog::debug("CS_Dx12::CreateBufferResource Start!");

	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags;
	HRESULT hr = InSource->GetHeapProperties(&heapProperties, &heapFlags);

	if (hr != S_OK)
	{
		spdlog::error("CS_Dx12::CreateBufferResource GetHeapProperties result: {0:x}", hr);
		return false;
	}

	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

	hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(&_buffer));

	if (hr != S_OK)
	{
		spdlog::error("CS_Dx12::CreateBufferResource CreateCommittedResource result: {0:x}", hr);
		return false;
	}

	_bufferState = InState;

	return true;
}

void CS_Dx12::SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = _buffer;
	barrier.Transition.StateBefore = _bufferState;
	barrier.Transition.StateAfter = InState;
	barrier.Transition.Subresource = 0;
	InCommandList->ResourceBarrier(1, &barrier);
	_bufferState = InState;
}

bool CS_Dx12::Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* OutResource, uint32_t InNumThreadsX, uint32_t InNumThreadsY)
{
	if (!_init || InDevice == nullptr || InCmdList == nullptr || InResource == nullptr || OutResource == nullptr || InNumThreadsX == 0 || InNumThreadsY == 0)
		return false;

	ID3D12DescriptorHeap* _srvHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 2; // One for SRV and one for UAV
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap));

	if (FAILED(hr))
	{
		return false;
	}

	auto srvHandle = _srvHeap->GetCPUDescriptorHandleForHeapStart();
	auto uavHandle = srvHandle;
	uavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle = _srvHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuUavHandle = gpuSrvHandle;
	gpuUavHandle.ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create SRV for Input Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = InResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	InDevice->CreateShaderResourceView(InResource, &srvDesc, srvHandle);

	// Create UAV for Output Texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = OutResource->GetDesc().Format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	InDevice->CreateUnorderedAccessView(OutResource, nullptr, &uavDesc, uavHandle);

	ID3D12DescriptorHeap* heaps[] = { _srvHeap };
	InCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	InCmdList->SetComputeRootSignature(_rootSignature);
	InCmdList->SetPipelineState(_pipelineState);

	InCmdList->SetComputeRootDescriptorTable(0, gpuSrvHandle);
	InCmdList->SetComputeRootDescriptorTable(1, gpuUavHandle);

	UINT dispatchWidth = (InResource->GetDesc().Width + InNumThreadsX - 1) / InNumThreadsX;
	UINT dispatchHeight = (InResource->GetDesc().Height + InNumThreadsY - 1) / InNumThreadsY;
	InCmdList->Dispatch(dispatchWidth, dispatchHeight, 1);

	/*ID3D12Fence* d3d12Fence = nullptr;

	do
	{
		if (InDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
			break;

		d3d12Fence->Signal(999);

		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
		{
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}

	} while (false);

	if (d3d12Fence != nullptr)
	{
		d3d12Fence->Release();
		d3d12Fence = nullptr;
	}*/

	_srvHeap->Release();
	_srvHeap = nullptr;

	return true;
}

CS_Dx12::CS_Dx12(ID3D12Device* InDevice, const char* InShaderCode, const char* InEntry, const char* InTarget)
{
	// Describe and create the root signature
// ---------------------------------------------------
	D3D12_DESCRIPTOR_RANGE descriptorRange[2];

	// SRV Range (Input Texture)
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = 0; // Assuming t0 register in HLSL for SRV
	descriptorRange[0].RegisterSpace = 0;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// UAV Range (Output Texture)
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].BaseShaderRegister = 0; // Assuming u0 register in HLSL for UAV
	descriptorRange[1].RegisterSpace = 0;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Define the root parameter (descriptor table)
	// ---------------------------------------------------
	D3D12_ROOT_PARAMETER rootParameters[2];

	// Root Parameter for SRV
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;						// One range (SRV)
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];		// Point to the SRV range
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root Parameter for UAV
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;						// One range (UAV)
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];		// Point to the UAV range
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// A root signature is an array of root parameters
	// ---------------------------------------------------
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.NumParameters = 2; // Two root parameters
	rootSigDesc.pParameters = rootParameters;
	rootSigDesc.NumStaticSamplers = 0;
	rootSigDesc.pStaticSamplers = nullptr;
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ID3DBlob* errorBlob;
	ID3DBlob* signatureBlob;

	do
	{
		auto hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

		if (FAILED(hr))
		{
			break;
		}

		hr = InDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));

		if (FAILED(hr))
		{
			break;
		}

	} while (false);

	if (errorBlob != nullptr)
	{
		errorBlob->Release();
		errorBlob = nullptr;
	}

	if (signatureBlob != nullptr)
	{
		signatureBlob->Release();
		signatureBlob = nullptr;
	}

	if (_rootSignature == nullptr)
	{
		return;
	}

	// Compile shader blobs
	auto _recEncodeShader = CompileShader(InShaderCode, InEntry, InTarget);

	if (_recEncodeShader == nullptr)
	{
		return;
	}

	// create pso objects
	if (!CreateComputeShader(InDevice, _rootSignature, &_pipelineState, _recEncodeShader))
	{
		return;
	}

	if (_recEncodeShader != nullptr)
	{
		_recEncodeShader->Release();
		_recEncodeShader = nullptr;
	}

	_init = _pipelineState != nullptr;
}

CS_Dx12::~CS_Dx12()
{
	if (!_init)
		return;

	if (_rootSignature != nullptr)
	{
		_rootSignature->Release();
		_rootSignature = nullptr;
	}

	if (_pipelineState != nullptr)
	{
		_pipelineState->Release();
		_pipelineState = nullptr;
	}

	//if (_srvHeap != nullptr)
	//{
	//	_srvHeap->Release();
	//	_srvHeap = nullptr;
	//}

	if (_buffer != nullptr)
	{
		_buffer->Release();
		_buffer = nullptr;
	}
}
