#include "OS_Dx11.h"

#define A_CPU
// FSR compute shader is from : https://github.com/fholger/vrperfkit/

#include "precompile/BCDS_Shader_Dx11.h"
#include "precompile/BCUS_Shader_Dx11.h"

#include "../fsr1/ffx_fsr1.h"
#include "../fsr1/FSR_EASU_Shader_Dx11.h"

#include "../Config.h"

bool OS_Dx11::CreateBufferResource(ID3D11Device* InDevice, ID3D11Resource* InResource, uint32_t InWidth, uint32_t InHeight)
{
    if (InDevice == nullptr || InResource == nullptr)
        return false;

    ID3D11Texture2D* originalTexture = nullptr;
    auto result = InResource->QueryInterface(IID_PPV_ARGS(&originalTexture));
    if (result != S_OK)
        return false;

    D3D11_TEXTURE2D_DESC texDesc;
    originalTexture->GetDesc(&texDesc);

    if (_buffer != nullptr)
    {
        D3D11_TEXTURE2D_DESC bufDesc;
        _buffer->GetDesc(&bufDesc);

        if (bufDesc.Width != InWidth || bufDesc.Height != InHeight || bufDesc.Format != texDesc.Format)
        {
            _buffer->Release();
            _buffer = nullptr;
        }
        else
            return true;
    }

    LOG_DEBUG("[{0}] Start!", _name);

    texDesc.Width = InWidth;
    texDesc.Height = InHeight;
    texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    result = InDevice->CreateTexture2D(&texDesc, nullptr, &_buffer);
    if (result != S_OK)
    {
        LOG_ERROR("[{0}] CreateCommittedResource result: {1:x}", _name, result);
        return false;
    }

    return true;
}

bool OS_Dx11::InitializeViews(ID3D11Texture2D* InResource, ID3D11Texture2D* OutResource)
{
    if (!_init || InResource == nullptr || OutResource == nullptr)
        return false;

    D3D11_TEXTURE2D_DESC desc;

    if (InResource != _currentInResource || _srvInput == nullptr)
    {
        if (_srvInput != nullptr)
            _srvInput->Release();

        InResource->GetDesc(&desc);

        // Create SRV for input texture
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;

        auto  hr = _device->CreateShaderResourceView(InResource, &srvDesc, &_srvInput);
        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] _srvInput CreateShaderResourceView error {1:x}", _name, hr);
            return false;
        }

        _currentInResource = InResource;
    }

    if (OutResource != _currentOutResource || _uavOutput == nullptr)
    {
        if (_uavOutput != nullptr)
            _uavOutput->Release();

        OutResource->GetDesc(&desc);

        // Create UAV for output texture
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = desc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

        auto hr = _device->CreateUnorderedAccessView(OutResource, &uavDesc, &_uavOutput);
        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] CreateUnorderedAccessView error {1:x}", _name, hr);
            return false;
        }


        _currentOutResource = OutResource;
    }

    return true;
}

bool OS_Dx11::Dispatch(ID3D11Device* InDevice, ID3D11DeviceContext* InContext, ID3D11Texture2D* InResource, ID3D11Texture2D* OutResource)
{
    if (!_init || InDevice == nullptr || InContext == nullptr || InResource == nullptr || OutResource == nullptr)
        return false;

    LOG_DEBUG("[{0}] Start!", _name);

    _device = InDevice;

    if (!InitializeViews(InResource, OutResource))
        return false;

    D3D11_TEXTURE2D_DESC inDesc;
    InResource->GetDesc(&inDesc);

    // fsr upscaling
    if (Config::Instance()->OutputScalingUseFsr.value_or(true))
    {
        UpscaleShaderConstants constants{};

        FsrEasuCon(constants.const0, constants.const1, constants.const2, constants.const3,
                   Config::Instance()->CurrentFeature->TargetWidth(), Config::Instance()->CurrentFeature->TargetHeight(),
                   inDesc.Width, inDesc.Height,
                   Config::Instance()->CurrentFeature->DisplayWidth(), Config::Instance()->CurrentFeature->DisplayHeight());

        // Copy the updated constant buffer data to the constant buffer resource
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        auto hr = InContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] Map error {1:x}", _name, hr);
            return false;
        }

        memcpy(mappedResource.pData, &constants, sizeof(constants));
        InContext->Unmap(_constantBuffer, 0);
    }
    else
    {
        Constants constants{};
        constants.srcWidth = Config::Instance()->CurrentFeature->TargetWidth();
        constants.srcHeight = Config::Instance()->CurrentFeature->TargetHeight();
        constants.destWidth = Config::Instance()->CurrentFeature->DisplayWidth(); // static_cast<uint32_t>(outDesc.Width);
        constants.destHeight = Config::Instance()->CurrentFeature->DisplayHeight(); // outDesc.Height;

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        auto hr = InContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] Map error {1:x}", _name, hr);
            return false;
        }

        memcpy(mappedResource.pData, &constants, sizeof(constants));
        InContext->Unmap(_constantBuffer, 0);
    }


    // Set the compute shader and resources
    InContext->CSSetShader(_computeShader, nullptr, 0);
    InContext->CSSetConstantBuffers(0, 1, &_constantBuffer);
    InContext->CSSetShaderResources(0, 1, &_srvInput);
    InContext->CSSetUnorderedAccessViews(0, 1, &_uavOutput, nullptr);

    UINT dispatchWidth = 0;
    UINT dispatchHeight = 0;

    if (_upsample || Config::Instance()->OutputScalingUseFsr.value_or(true))
    {
        dispatchWidth = static_cast<UINT>((Config::Instance()->CurrentFeature->DisplayWidth() + InNumThreadsX - 1) / InNumThreadsX);
        dispatchHeight = (Config::Instance()->CurrentFeature->DisplayHeight() + InNumThreadsY - 1) / InNumThreadsY;
    }
    else
    {
        dispatchWidth = static_cast<UINT>((Config::Instance()->CurrentFeature->TargetWidth() + InNumThreadsX - 1) / InNumThreadsX);
        dispatchHeight = (Config::Instance()->CurrentFeature->TargetHeight() + InNumThreadsY - 1) / InNumThreadsY;
    }

    InContext->Dispatch(dispatchWidth, dispatchHeight, 1);

    // Unbind resources
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    InContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    InContext->CSSetShaderResources(0, 2, nullSRV);

    return true;
}

OS_Dx11::OS_Dx11(std::string InName, ID3D11Device* InDevice, bool InUpsample) : _name(InName), _device(InDevice), _upsample(InUpsample)
{
    if (InDevice == nullptr)
    {
        LOG_ERROR("InDevice is nullptr!");
        return;
    }

    LOG_DEBUG("{0} start!", _name);

    if (Config::Instance()->UsePrecompiledShaders.value_or(true) || Config::Instance()->OutputScalingUseFsr.value_or(true))
    {
        HRESULT hr;

        // fsr upscaling
        if (Config::Instance()->OutputScalingUseFsr.value_or(true))
        {
            hr = _device->CreateComputeShader(reinterpret_cast<const void*>(fsr_easu_cso), sizeof(fsr_easu_cso), nullptr, &_computeShader);
        }
        else
        {
            if (_upsample)
                hr = _device->CreateComputeShader(reinterpret_cast<const void*>(bcus_cso), sizeof(bcus_cso), nullptr, &_computeShader);
            else
                hr = _device->CreateComputeShader(reinterpret_cast<const void*>(bcds_cso), sizeof(bcds_cso), nullptr, &_computeShader);
        }

        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] CreateComputeShader error: {1:X}", _name, hr);
            return;
        }
    }
    else
    {
        // Compile shader blobs
        ID3DBlob* shaderBlob = OS_CompileShader(_upsample ? upsampleCode.c_str() : downsampleCode.c_str(), "CSMain", "cs_5_0");
        if (shaderBlob == nullptr)
        {
            LOG_ERROR("[{0}] CompileShader error!", _name);
            return;
        }

        // create pso objects
        auto hr = _device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &_computeShader);

        if (shaderBlob != nullptr)
        {
            shaderBlob->Release();
            shaderBlob = nullptr;
        }

        if (FAILED(hr))
        {
            LOG_ERROR("[{0}] CreateComputeShader error: {1:X}", _name, hr);
            return;
        }
    }

    // CBV
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(Constants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    auto result = InDevice->CreateBuffer(&cbDesc, nullptr, &_constantBuffer);
    if (result != S_OK)
    {
        LOG_ERROR("CreateBuffer error: {0:X}", (UINT)result);
        return;
    }

    // FSR upscaling
    if (Config::Instance()->OutputScalingUseFsr.value_or(true))
    {
        InNumThreadsX = 16;
        InNumThreadsY = 16;
    }
    _init = true;
}

OS_Dx11::~OS_Dx11()
{
    if (!_init)
        return;

    if (_computeShader != nullptr)
        _computeShader->Release();

    if (_constantBuffer != nullptr)
        _constantBuffer->Release();

    if (_srvInput != nullptr)
        _srvInput->Release();

    if (_uavOutput != nullptr)
        _uavOutput->Release();

    if (_buffer != nullptr)
        _buffer->Release();
}
