#include "Engine/Graphics/WindowDX12.hpp"

#include "Engine/Log.hpp"
#include "Game/GameData.hpp"

HRESULT Window::initGraphicAPI()
{
    HRESULT hr;
    HWND hwnd = glfwGetWin32Window(window);

    UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
    ComPtr<ID3D12Debug> debugController;

    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ComPtr<IDXGIAdapter1> adapter;

    V_RETURN(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    auto AdapterSelctionPred = [](IDXGIAdapter1* pAdapter) {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Basic Render Driver Adapter
            return FALSE;
        }
        else
        {
            // Check whether the adapter support dx12
            if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS((ID3D12Device**)0))))
            {
                return FALSE;
            }
            return TRUE;
        }
    };

    // select a adapter
    V_RETURN(getHardwareAdpter(factory.Get(), TRUE, AdapterSelctionPred, &adapter));
    // create device
    V_RETURN(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pd3dDevice)));

    m_iFencePoint = 0;
    V_RETURN(m_pd3dDevice->CreateFence(m_iFencePoint, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pd3dFence)));

    m_uRtvDescriptorSize       = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_uDsvDescriptorSize       = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_uCbvSrvUavDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    V_RETURN(createCommandObjects());
    V_RETURN(createSwapChain(hwnd, windowSize.x, windowSize.y, factory.Get()));
    V_RETURN(createRtvAndDsvDescriptorHeaps(0, 0));
    return 0;
}

HRESULT Window::createSwapChain(HWND hwnd, int width, int height, IDXGIFactory4* pDXGIFactory)
{
    HRESULT              hr;
    DXGI_SWAP_CHAIN_DESC dscd;

    // Release the previous Swap Chain so we can re-create new ones.
    m_pSwapChain = nullptr;

    dscd.BufferDesc.Width                   = width;
    dscd.BufferDesc.Height                  = height;
    dscd.BufferDesc.RefreshRate.Numerator   = 60;
    dscd.BufferDesc.RefreshRate.Denominator = 1;
    dscd.BufferDesc.Format                  = s_backBufferFormat;
    dscd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    dscd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    dscd.SampleDesc.Count                   = 1;
    dscd.SampleDesc.Quality                 = 0;
    dscd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dscd.BufferCount                        = s_iSwapChainBufferCount;
    dscd.OutputWindow                       = hwnd;
    dscd.Windowed                           = TRUE;
    dscd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    dscd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    V_RETURN(pDXGIFactory->CreateSwapChain(m_pd3dCommandQueue.Get(), &dscd, &m_pSwapChain));

    return hr;
}

HRESULT Window::createRtvAndDsvDescriptorHeaps(int extraRtvCount, int extraDsvCount)
{
    HRESULT hr;

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = s_iSwapChainBufferCount + extraRtvCount;
    rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask       = 0;
    V_RETURN(m_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRTVDescriptorHeap)));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1 + extraDsvCount;
    dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask       = 0;
    V_RETURN(m_pd3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDSVDescriptorHeap)));

    return hr;
}

HRESULT Window::getHardwareAdpter(_In_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter,
                  _In_ BOOL (*AdapterSelectionCallback)(IDXGIAdapter1*), _Out_ IDXGIAdapter1** ppAdapter)
{
    ComPtr<IDXGIFactory6> pFactory6;
    ComPtr<IDXGIAdapter1> adapter;
    HRESULT               hr = E_FAIL;

    *ppAdapter = nullptr;

    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pFactory6))))
    {
        for (int adapterIndex = 0;
             DXGI_ERROR_NOT_FOUND != pFactory6->EnumAdapterByGpuPreference(adapterIndex,
                                                                           bRequestHighPerformanceAdapter
                                                                               ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE
                                                                               : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                                                                           IID_PPV_ARGS(&adapter));
             ++adapterIndex)
        {

            if (AdapterSelectionCallback(adapter.Get()))
            {
                *ppAdapter = adapter.Get();
                adapter->AddRef();
                hr = S_OK;
                break;
            }
        }
    }
    else
    {
        for (int adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter);
             ++adapterIndex)
        {
            if (AdapterSelectionCallback(adapter.Get()))
            {
                *ppAdapter = adapter.Get();
                adapter->AddRef();
                hr = S_OK;
                break;
            }
        }
    }

    return hr;
}

HRESULT Window::createCommandObjects()
{
    HRESULT                  hr;
    D3D12_COMMAND_QUEUE_DESC dqd = {};
    dqd.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    dqd.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    V_RETURN(m_pd3dDevice->CreateCommandQueue(&dqd, IID_PPV_ARGS(&m_pd3dCommandQueue)));

    V_RETURN(m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dDirectCmdAlloc)));

    V_RETURN(m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dDirectCmdAlloc.Get(), nullptr,
                                             IID_PPV_ARGS(&m_pd3dCommandList)));

    m_pd3dCommandList->Close();

    return hr;
}

void Window::init(GameData& datas)
{
    initGLFW();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    initWindow(datas);
    initGraphicAPI();
    resizeRenderedBuffers(windowSize.x, windowSize.y);
}

Window::~Window()
{
    if (m_hFenceEvent)
        CloseHandle(m_hFenceEvent);
}

void Window::flushCommandQueue()
{
    HRESULT hr;
    m_iFencePoint += 1;

    V(m_pd3dCommandQueue->Signal(m_pd3dFence.Get(), m_iFencePoint));

    if (m_pd3dFence->GetCompletedValue() != m_iFencePoint)
    {
        if (!m_hFenceEvent)
            m_hFenceEvent = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);

        m_pd3dFence->SetEventOnCompletion(m_iFencePoint, m_hFenceEvent);

        WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

HRESULT Window::resizeRenderedBuffers(int width, int height)
{
    HRESULT              hr;
    int                  i;
    DXGI_SWAP_CHAIN_DESC scDesc;

    assert(m_pd3dDevice && "Device must be available!");
    assert(m_pd3dDirectCmdAlloc && "Command allocator must be available!");
    assert(m_pSwapChain && "Swap chain must be available!");

    // Flush before changing any resources.
    flushCommandQueue();

    V_RETURN(m_pd3dCommandList->Reset(m_pd3dDirectCmdAlloc.Get(), nullptr));

    // Release the previous resources we will be recreating.
    for (i = 0; i < s_iSwapChainBufferCount; ++i)
        m_pSwapChainBuffers[i] = nullptr;
    m_pDepthStencilBuffer = nullptr;

    m_pSwapChain->GetDesc(&scDesc);

    // Resize the swap chain.
    V_RETURN(m_pSwapChain->ResizeBuffers(s_iSwapChainBufferCount, width, height, s_backBufferFormat, scDesc.Flags));

    m_iCurrentFrameIndex = 0;

    // RTVs
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle = {m_pRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart()};
    for (i = 0; i < s_iSwapChainBufferCount; ++i)
    {
        m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainBuffers[i]));

        m_pd3dDevice->CreateRenderTargetView(m_pSwapChainBuffers[i].Get(), nullptr, rtvHeapHandle);

        rtvHeapHandle.ptr += m_uRtvDescriptorSize;
    }

    // Create depth-stencil buffer and the view.
    D3D12_RESOURCE_DESC dsd;
    dsd.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    dsd.Alignment        = 0;
    dsd.Width            = width;
    dsd.Height           = height;
    dsd.DepthOrArraySize = 1;
    dsd.MipLevels        = 1;

    dsd.Format             = DXGI_FORMAT_R24G8_TYPELESS;
    dsd.SampleDesc.Count   = 1;
    dsd.SampleDesc.Quality = 0;
    dsd.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dsd.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optValue;
    optValue.Format               = s_depthStencilFormat;
    optValue.DepthStencil.Depth   = 1.0f;
    optValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                       D3D12_MEMORY_POOL_UNKNOWN, 0, 0};

    V_RETURN(m_pd3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &dsd, D3D12_RESOURCE_STATE_COMMON,
                                                   &optValue, IID_PPV_ARGS(&m_pDepthStencilBuffer)));

    // Create descriptor to mip level 0 of entire resource using the format of the
    // resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC ddsvd;
    ddsvd.Flags              = D3D12_DSV_FLAG_NONE;
    ddsvd.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    ddsvd.Format             = s_depthStencilFormat;
    ddsvd.Texture2D.MipSlice = 0;
    m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &ddsvd,
                                         m_pDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Transition the resource from its initial state to be used as a depth
    // buffer.
    D3D12_RESOURCE_BARRIER dsvBarrier = {D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE};
    dsvBarrier.Transition.pResource   = m_pDepthStencilBuffer.Get();
    dsvBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dsvBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    dsvBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    m_pd3dCommandList->ResourceBarrier(1, &dsvBarrier);

    // Execute the resize commands.
    V_RETURN(m_pd3dCommandList->Close());
    ID3D12CommandList* cmdLists[] = {m_pd3dCommandList.Get()};
    m_pd3dCommandQueue->ExecuteCommandLists(1, cmdLists);

    // Wait until resize is complete.
    flushCommandQueue();

    // Update the viewport transform to cover the client area.
    m_ScreenViewport.TopLeftX = 0.f;
    m_ScreenViewport.TopLeftY = 0.f;
    m_ScreenViewport.Width    = 1.f * width;
    m_ScreenViewport.Height   = 1.f * height;
    m_ScreenViewport.MinDepth = 0.f;
    m_ScreenViewport.MaxDepth = 1.f;

    m_ScreenScissorRect = {0, 0, (LONG)width, (LONG)height};

    return hr;
}

void Window::initDrawContext()
{
    HRESULT hr;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution
    // on the GPU.
    V(m_pd3dDirectCmdAlloc->Reset());
    // A command list can be reset after it has been added to the command queue
    // via ExecuteCommandList.
    // Reusing the command list reuses memory.
    V(m_pd3dCommandList->Reset(m_pd3dDirectCmdAlloc.Get(), nullptr));

    // Prepare for the next frame
    D3D12_RESOURCE_BARRIER rdBarrier = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
    };
    rdBarrier.Transition.pResource   = m_pSwapChainBuffers[m_iCurrentFrameIndex].Get();
    rdBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    rdBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pd3dCommandList->ResourceBarrier(1, &rdBarrier);

    // Render the next frame
    m_pd3dCommandList->RSSetViewports(1, &m_ScreenViewport);
    m_pd3dCommandList->RSSetScissorRects(1, &m_ScreenScissorRect);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_iCurrentFrameIndex * m_uRtvDescriptorSize;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    const float clrColor[] = {0.0f, .0f, .0f, .0f};

    m_pd3dCommandList->ClearRenderTargetView(rtvHandle, clrColor, 0, nullptr);
    // No need to clear depth, we don't use it
    m_pd3dCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    m_pd3dCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

    // State transition
    rdBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rdBarrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    m_pd3dCommandList->ResourceBarrier(1, &rdBarrier);
}

void Window::renderFrame()
{
    HRESULT hr;

    // Done recording commands.
    V(m_pd3dCommandList->Close());
    ID3D12CommandList* cmdLists[] = {m_pd3dCommandList.Get()};
    m_pd3dCommandQueue->ExecuteCommandLists(1, cmdLists);

    m_pSwapChain->Present(0, 0);
    m_iCurrentFrameIndex = (m_iCurrentFrameIndex + 1) % s_iSwapChainBufferCount;

    flushCommandQueue();
}

void dxTrace(const wchar_t* file, unsigned long line, HRESULT hr, const wchar_t* proc)
{
    _com_error err(hr);
    logf("file: %s line: %ul, %s Error: %s/n", file, line, proc, (const char*)err.Description());
}