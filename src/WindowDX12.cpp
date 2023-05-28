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
}

Window::~Window()
{
    if (m_hFenceEvent)
        CloseHandle(m_hFenceEvent);
}

void dxTrace(const wchar_t* file, unsigned long line, HRESULT hr, const wchar_t* proc)
{
    _com_error err(hr);
    logf("file: %s line: %ul, %s Error: %s/n", file, line, proc, (const char*)err.Description());
}