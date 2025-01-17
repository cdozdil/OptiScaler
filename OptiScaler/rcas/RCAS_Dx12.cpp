#include "RCAS_Dx12.h"

#include "precompile/RCAS_Shader.h"

#include "../Config.h"

inline static DXGI_FORMAT TranslateTypelessFormats(DXGI_FORMAT format)
{
	switch (format) {
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case DXGI_FORMAT_R32G32B32_TYPELESS:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_FLOAT;
		case DXGI_FORMAT_R32G32_TYPELESS:
			return DXGI_FORMAT_R32G32_FLOAT;
		default:
			return format;
	}
}

bool RCAS_Dx12::CreateComputeShader(ID3D12Device* device, ID3D12RootSignature* rootSignature, ID3D12PipelineState** pipelineState, ID3DBlob* shaderBlob)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.CS = CD3DX12_SHADER_BYTECODE(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

	HRESULT hr = device->CreateComputePipelineState(&psoDesc, __uuidof(ID3D12PipelineState*), (void**)pipelineState);

	if (FAILED(hr))
	{
		LOG_ERROR("CreateComputePipelineState error {0:x}", hr);
		return false;
	}

	return true;
}

bool RCAS_Dx12::CreateBufferResource(ID3D12Device* InDevice, ID3D12Resource* InSource, D3D12_RESOURCE_STATES InState)
{
	if (InDevice == nullptr || InSource == nullptr)
		return false;

	D3D12_RESOURCE_DESC texDesc = InSource->GetDesc();

	if (_buffer != nullptr)
	{
		auto bufDesc = _buffer->GetDesc();

		if (bufDesc.Width != (UINT64)(texDesc.Width) || bufDesc.Height != (UINT)(texDesc.Height) || bufDesc.Format != texDesc.Format)
		{
			_buffer->Release();
			_buffer = nullptr;
		}
		else
			return true;
	}

	LOG_DEBUG("[{0}] Start!", _name);

	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags;
	HRESULT hr = InSource->GetHeapProperties(&heapProperties, &heapFlags);

	if (hr != S_OK)
	{
		LOG_ERROR("[{0}] GetHeapProperties result: {1:x}", _name.c_str(), hr);
		return false;
	}

	texDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
	texDesc.Width = texDesc.Width;
	texDesc.Height = texDesc.Height;

	hr = InDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texDesc, InState, nullptr, IID_PPV_ARGS(&_buffer));

	if (hr != S_OK)
	{
		LOG_ERROR("[{0}] CreateCommittedResource result: {1:x}", _name, hr);
		return false;
	}


	_buffer->SetName(L"RCAS_Buffer");
	_bufferState = InState;

	return true;
}

void RCAS_Dx12::SetBufferState(ID3D12GraphicsCommandList* InCommandList, D3D12_RESOURCE_STATES InState)
{
	if (_bufferState == InState)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = _buffer;
	barrier.Transition.StateBefore = _bufferState;
	barrier.Transition.StateAfter = InState;
	barrier.Transition.Subresource = 0;
	InCommandList->ResourceBarrier(1, &barrier);

	_bufferState = InState;
}

bool RCAS_Dx12::Dispatch(ID3D12Device* InDevice, ID3D12GraphicsCommandList* InCmdList, ID3D12Resource* InResource, ID3D12Resource* InMotionVectors, RcasConstants InConstants, ID3D12Resource* OutResource)
{
	if (!_init || InDevice == nullptr || InCmdList == nullptr || InResource == nullptr || OutResource == nullptr || InMotionVectors == nullptr)
		return false;

	LOG_DEBUG("[{0}] Start!", _name);

	_counter++;
	_counter = _counter % 2;

	if (_cpuSrvHandle[_counter].ptr == NULL)
		_cpuSrvHandle[_counter] = _srvHeap[_counter]->GetCPUDescriptorHandleForHeapStart();

	if (_cpuSrvHandle2[_counter].ptr == NULL)
	{
		_cpuSrvHandle2[_counter] = _cpuSrvHandle[_counter];
		_cpuSrvHandle2[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if (_cpuUavHandle[_counter].ptr == NULL)
	{
		_cpuUavHandle[_counter] = _cpuSrvHandle2[_counter];
		_cpuUavHandle[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if (_cpuCbvHandle[_counter].ptr == NULL)
	{
		_cpuCbvHandle[_counter] = _cpuUavHandle[_counter];
		_cpuCbvHandle[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if (_gpuSrvHandle[_counter].ptr == NULL)
		_gpuSrvHandle[_counter] = _srvHeap[_counter]->GetGPUDescriptorHandleForHeapStart();

	if (_gpuSrvHandle2[_counter].ptr == NULL)
	{
		_gpuSrvHandle2[_counter] = _gpuSrvHandle[_counter];
		_gpuSrvHandle2[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if (_gpuUavHandle[_counter].ptr == NULL)
	{
		_gpuUavHandle[_counter] = _gpuSrvHandle2[_counter];
		_gpuUavHandle[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	if (_gpuCbvHandle[_counter].ptr == NULL)
	{
		_gpuCbvHandle[_counter] = _gpuUavHandle[_counter];
		_gpuCbvHandle[_counter].ptr += InDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	auto inDesc = InResource->GetDesc();
	auto mvDesc = InMotionVectors->GetDesc();
	auto outDesc = OutResource->GetDesc();

	// Create SRV for Input Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = TranslateTypelessFormats(inDesc.Format);
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	InDevice->CreateShaderResourceView(InResource, &srvDesc, _cpuSrvHandle[_counter]);

	// Create SRV for Motion Texture
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2 = {};
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.Format = TranslateTypelessFormats(mvDesc.Format);
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = 1;

	InDevice->CreateShaderResourceView(InMotionVectors, &srvDesc2, _cpuSrvHandle2[_counter]);

	// Create UAV for Output Texture
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = TranslateTypelessFormats(outDesc.Format);
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	InDevice->CreateUnorderedAccessView(OutResource, nullptr, &uavDesc, _cpuUavHandle[_counter]);

	if (_constantBuffer == nullptr)
	{
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(InternalConstants));
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		auto result = InDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_constantBuffer));

		if (result != S_OK)
		{
			LOG_ERROR("[{0}] CreateCommittedResource error {1:x}", _name, (unsigned int)result);
			return false;
		}
	}

	InternalConstants constants{};
	constants.DisplayHeight = InConstants.DisplayHeight;
	constants.DisplayWidth = InConstants.DisplayWidth;
	constants.DynamicSharpenEnabled = Config::Instance()->MotionSharpnessEnabled.value_or_default() ? 1 : 0;
	constants.MotionSharpness = Config::Instance()->MotionSharpness.value_or_default();
	constants.MvScaleX = InConstants.MvScaleX;
	constants.MvScaleY = InConstants.MvScaleY;
	constants.Sharpness = InConstants.Sharpness;
	constants.Debug = Config::Instance()->MotionSharpnessDebug.value_or_default() ? 1 : 0;
	constants.Threshold = Config::Instance()->MotionThreshold.value_or_default();
	constants.ScaleLimit = Config::Instance()->MotionScaleLimit.value_or_default();
	constants.DisplaySizeMV = InConstants.DisplaySizeMV ? 1 : 0;

	if (InConstants.RenderWidth == 0 || InConstants.DisplayWidth == 0)
		constants.MotionTextureScale = 1.0f;
	else
		constants.MotionTextureScale = (float)InConstants.RenderWidth / (float)InConstants.DisplayWidth;

	// Copy the updated constant buffer data to the constant buffer resource
	BYTE* pCBDataBegin;
	CD3DX12_RANGE readRange(0, 0);  // We do not intend to read from this resource on the CPU
	auto result = _constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCBDataBegin));

	if (result != S_OK)
	{
		LOG_ERROR("[{0}] _constantBuffer->Map error {1:x}", _name, (unsigned int)result);
		return false;
	}

	if (pCBDataBegin == nullptr)
	{
		_constantBuffer->Unmap(0, nullptr);
		LOG_ERROR("[{0}] pCBDataBegin is null!", _name);
		return false;
	}

	memcpy(pCBDataBegin, &constants, sizeof(constants));
	_constantBuffer->Unmap(0, nullptr);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = sizeof(constants);
	InDevice->CreateConstantBufferView(&cbvDesc, _cpuCbvHandle[_counter]);

	ID3D12DescriptorHeap* heaps[] = { _srvHeap[_counter] };
	InCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	InCmdList->SetComputeRootSignature(_rootSignature);
	InCmdList->SetPipelineState(_pipelineState);

	InCmdList->SetComputeRootDescriptorTable(0, _gpuSrvHandle[_counter]);
	InCmdList->SetComputeRootDescriptorTable(1, _gpuSrvHandle2[_counter]);
	InCmdList->SetComputeRootDescriptorTable(2, _gpuUavHandle[_counter]);
	InCmdList->SetComputeRootDescriptorTable(3, _gpuCbvHandle[_counter]);

	UINT dispatchWidth = 0;
	UINT dispatchHeight = 0;

	dispatchWidth = (inDesc.Width + InNumThreadsX - 1) / InNumThreadsX;
	dispatchHeight = (inDesc.Height + InNumThreadsY - 1) / InNumThreadsY;

	InCmdList->Dispatch(dispatchWidth, dispatchHeight, 1);

	return true;
}

RCAS_Dx12::RCAS_Dx12(std::string InName, ID3D12Device* InDevice) : _name(InName), _device(InDevice)
{
	if (InDevice == nullptr)
	{
		LOG_ERROR("InDevice is nullptr!");
		return;
	}

	LOG_DEBUG("{0} start!", _name);

	// Describe and create the root signature
	// ---------------------------------------------------
	D3D12_DESCRIPTOR_RANGE descriptorRange[4];

	// SRV Range (Input Texture)
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = 0; // Assuming t0 register in HLSL for SRV
	descriptorRange[0].RegisterSpace = 0;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// SRV Range (Motion Texture)
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].BaseShaderRegister = 1; // Assuming t1 register in HLSL for SRV
	descriptorRange[1].RegisterSpace = 0;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// UAV Range (Output Texture)
	descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRange[2].NumDescriptors = 1;
	descriptorRange[2].BaseShaderRegister = 0; // Assuming u0 register in HLSL for UAV
	descriptorRange[2].RegisterSpace = 0;
	descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// CBV Range (Params)
	descriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorRange[3].NumDescriptors = 1;
	descriptorRange[3].BaseShaderRegister = 0; // Assuming b0 register in HLSL for CBV
	descriptorRange[3].RegisterSpace = 0;
	descriptorRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Define the root parameter (descriptor table)
	// ---------------------------------------------------
	D3D12_ROOT_PARAMETER rootParameters[4];

	// Root Parameter for SRV 1
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;						// One range (SRV)
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];		// Point to the SRV range
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;						// One range (SRV)
	rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[1];		// Point to the SRV range
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root Parameter for UAV
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;						// One range (UAV)
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[2];		// Point to the UAV range
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// Root Parameter for CBV
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;						// One range (CBV)
	rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRange[3];		// Point to the CBV range
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// A root signature is an array of root parameters
	// ---------------------------------------------------
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.NumParameters = 4;
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
			LOG_ERROR("[{0}] D3D12SerializeRootSignature error {1:x}", _name, (unsigned int)hr);
			break;
		}

		hr = InDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));

		if (FAILED(hr))
		{
			LOG_ERROR("[{0}] CreateRootSignature error {1:x}", _name, (unsigned int)hr);
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
		LOG_ERROR("[{0}] _rootSignature is null!", _name);
		return;
	}

	if (Config::Instance()->UsePrecompiledShaders.value_or_default())
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
		computePsoDesc.pRootSignature = _rootSignature;
		computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(reinterpret_cast<const void*>(rcas_cso), sizeof(rcas_cso));
		auto hr = InDevice->CreateComputePipelineState(&computePsoDesc, __uuidof(ID3D12PipelineState*), (void**)&_pipelineState);

		if (FAILED(hr))
		{
			LOG_ERROR("[{0}] CreateComputePipelineState error: {1:X}", _name, hr);
			return;
		}
	}
	else
	{
		// Compile shader blobs
		ID3DBlob* _recEncodeShader = RCAS_CompileShader(rcasCode.c_str(), "CSMain", "cs_5_0");

		if (_recEncodeShader == nullptr)
		{
			LOG_ERROR("[{0}] RCAS_CompileShader error!", _name);
			return;
		}

		// create pso objects
		if (!CreateComputeShader(InDevice, _rootSignature, &_pipelineState, _recEncodeShader))
		{
			LOG_ERROR("[{0}] CreateComputeShader error!", _name);
			return;
		}

		if (_recEncodeShader != nullptr)
		{
			_recEncodeShader->Release();
			_recEncodeShader = nullptr;
		}
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 4; // SRV x 2 + UAV + CBV
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	State::Instance().skipHeapCapture = true;

	auto hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[0]));

	if (FAILED(hr))
	{
		LOG_ERROR("[{0}] CreateDescriptorHeap[0] error {1:x}", _name, (unsigned int)hr);
		return;
	}

	hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[1]));

	if (FAILED(hr))
	{
		LOG_ERROR("[{0}] CreateDescriptorHeap[1] error {1:x}", _name, (unsigned int)hr);
		return;
	}

	hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[2]));

	if (FAILED(hr))
	{
		LOG_ERROR("[{0}] CreateDescriptorHeap[2] error {1:x}", _name, (unsigned int)hr);
		return;
	}

	hr = InDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_srvHeap[3]));

	State::Instance().skipHeapCapture = false;

	if (FAILED(hr))
	{
		LOG_ERROR("[{0}] CreateDescriptorHeap[3] error {1:x}", _name, (unsigned int)hr);
		return;
	}

	_init = _srvHeap[3] != nullptr;
}

RCAS_Dx12::~RCAS_Dx12()
{
	if (!_init || State::Instance().isShuttingDown)
		return;

	//ID3D12Fence* d3d12Fence = nullptr;

	//do
	//{
	//	if (_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d12Fence)) != S_OK)
	//		break;

	//	d3d12Fence->Signal(999);

	//	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	//	if (fenceEvent != NULL && d3d12Fence->SetEventOnCompletion(999, fenceEvent) == S_OK)
	//	{
	//		WaitForSingleObject(fenceEvent, INFINITE);
	//		CloseHandle(fenceEvent);
	//	}

	//} while (false);

	//if (d3d12Fence != nullptr)
	//{
	//	d3d12Fence->Release();
	//	d3d12Fence = nullptr;
	//}

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

	if (_srvHeap[0] != nullptr)
	{
		_srvHeap[0]->Release();
		_srvHeap[0] = nullptr;
	}

	if (_srvHeap[1] != nullptr)
	{
		_srvHeap[1]->Release();
		_srvHeap[1] = nullptr;
	}

	if (_srvHeap[2] != nullptr)
	{
		_srvHeap[2]->Release();
		_srvHeap[2] = nullptr;
	}

	if (_srvHeap[3] != nullptr)
	{
		_srvHeap[3]->Release();
		_srvHeap[3] = nullptr;
	}

	if (_buffer != nullptr)
	{
		_buffer->Release();
		_buffer = nullptr;
	}

	if (_constantBuffer != nullptr)
	{
		_constantBuffer->Release();
		_constantBuffer = nullptr;
	}
}
