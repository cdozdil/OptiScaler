#include "MenuDx11.h"

#include <Util.h>
#include <Logger.h>
#include <Config.h>
#include "MenuBase.h"
#include <objects/WrappedSwapChain.h>

#include <dxgi1_6.h>
#include <include/imgui/imgui_impl_dx11.h>
#include <include/imgui/imgui_impl_win32.h>

// device
static ID3D11Device* _device = nullptr;
static ID3D11DeviceContext* _deviceContext = nullptr;
static ID3D11RenderTargetView* _renderTarget = nullptr;

// status
static bool _isInited = false;

// mutexes
static std::mutex _cleanMutex;

void MenuDx11::Init(IDXGISwapChain* pSwapChain, ID3D11DeviceContext* InDevCtx)
{
    _deviceContext == InDevCtx;
    _deviceContext->GetDevice(&_device);

    ID3D11Texture2D* pBackBuffer = NULL;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    if (pBackBuffer != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);

        D3D11_RENDER_TARGET_VIEW_DESC desc = { };
        desc.Format = static_cast<DXGI_FORMAT>(GetCorrectDXGIFormat(sd.BufferDesc.Format));
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        _device->CreateRenderTargetView(pBackBuffer, &desc, &_renderTarget);
        pBackBuffer->Release();

        _isInited = true;
    }
}

void MenuDx11::Cleanup(bool shutDown)
{
    if (!_isInited)
        return;

    if (!shutDown)
        LOG_FUNC();

    if (_renderTarget != nullptr)
    {
        _renderTarget->Release();
        _renderTarget = nullptr;
    }

    if (_device != nullptr)
    {
        _device->Release();
        _device = nullptr;
    }

    _isInited = false;
}

void MenuDx11::Render(IDXGISwapChain* pSwapChain)
{
    bool drawMenu = false;

    do
    {
        if (!MenuBase::IsInited() || !_isInited)
            break;

        // Draw only when menu activated
        if (!MenuBase::IsVisible())
            break;

        if (_device == nullptr)
            break;

        drawMenu = true;

    } while (false);

    if (!drawMenu)
    {
        MenuBase::HideMenu();
        return;
    }

    LOG_FUNC();

    if (ImGui::GetIO().BackendRendererUserData == nullptr)
    {
        ImGui_ImplDX11_Init(_device, _deviceContext);
    }

    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        MenuBase::RenderMenu();

        ImGui::Render();

        _deviceContext->OMSetRenderTargets(1, &_renderTarget, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

void MenuDx11::ShutDown()
{
    if (!Config::Instance()->IsRunningOnDXVK)
    {
        if (_isInited && MenuBase::IsInited() && ImGui::GetIO().BackendRendererUserData)
            ImGui_ImplDX11_Shutdown();

        MenuBase::Shutdown();

        if (_isInited)
            MenuDx11::Cleanup(true);
    }

    _isInited = false;
}

void MenuDx11::PrepareTimeObjects(ID3D11Device* InDevice)
{
    // Create Disjoint Query
    D3D11_QUERY_DESC disjointQueryDesc = {};
    disjointQueryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

    // Create Timestamp Queries
    D3D11_QUERY_DESC timestampQueryDesc = {};
    timestampQueryDesc.Query = D3D11_QUERY_TIMESTAMP;

    for (int i = 0; i < MenuDx11::QUERY_BUFFER_COUNT; i++)
    {
        InDevice->CreateQuery(&disjointQueryDesc, &MenuDx11::disjointQueries[i]);
        InDevice->CreateQuery(&timestampQueryDesc, &MenuDx11::startQueries[i]);
        InDevice->CreateQuery(&timestampQueryDesc, &MenuDx11::endQueries[i]);
    }
}

void MenuDx11::BeforeUpscale(ID3D11DeviceContext* InDevCtx)
{
    // In the render loop:
    MenuDx11::previousFrameIndex = (MenuDx11::currentFrameIndex + MenuDx11::QUERY_BUFFER_COUNT - 2) % MenuDx11::QUERY_BUFFER_COUNT;

    // Record the queries in the current frame
    InDevCtx->Begin(MenuDx11::disjointQueries[MenuDx11::currentFrameIndex]);
    InDevCtx->End(MenuDx11::startQueries[MenuDx11::currentFrameIndex]);
}

void MenuDx11::AfterUpscale(ID3D11DeviceContext* InDevCtx)
{
    InDevCtx->End(MenuDx11::endQueries[MenuDx11::currentFrameIndex]);
    InDevCtx->End(MenuDx11::disjointQueries[MenuDx11::currentFrameIndex]);

    MenuDx11::dx11UpscaleTrig[MenuDx11::currentFrameIndex] = true;
}

void MenuDx11::CalculateTime()
{
    if (MenuBase::IsInited() && MenuBase::IsVisible())
    {
        // Retrieve the results from the previous frame
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        if (_deviceContext->GetData(MenuDx11::disjointQueries[MenuDx11::previousFrameIndex], &disjointData, sizeof(disjointData), 0) == S_OK)
        {
            if (!disjointData.Disjoint && disjointData.Frequency > 0)
            {
                UINT64 startTime = 0, endTime = 0;
                if (_deviceContext->GetData(MenuDx11::startQueries[MenuDx11::previousFrameIndex], &startTime, sizeof(UINT64), 0) == S_OK &&
                    _deviceContext->GetData(MenuDx11::endQueries[MenuDx11::previousFrameIndex], &endTime, sizeof(UINT64), 0) == S_OK)
                {
                    double elapsedTimeMs = (endTime - startTime) / static_cast<double>(disjointData.Frequency) * 1000.0;
                    Config::Instance()->upscaleTimes.push_back(elapsedTimeMs);
                    Config::Instance()->upscaleTimes.pop_front();
                }
            }
        }
    }

    MenuDx11::dx11UpscaleTrig[MenuDx11::currentFrameIndex] = false;
    MenuDx11::currentFrameIndex = (MenuDx11::currentFrameIndex + 1) % MenuDx11::QUERY_BUFFER_COUNT;
}
