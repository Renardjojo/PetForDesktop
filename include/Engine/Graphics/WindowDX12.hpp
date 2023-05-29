#pragma once

#include "Engine/WindowGLFW.hpp"

#include <glfw/glfw3native.h>

#include <DirectXMath.h> // For XMVector, XMFLOAT3, XMFLOAT4
#include <comdef.h>      // for _com_error
#include <d3d12.h>       // for D3D12 interface
#include <dxgi1_6.h>     // for DXGI
#include <wrl.h>         // for Microsoft::WRL::ComPtr

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using Microsoft::WRL::ComPtr;
using namespace DirectX;

void dxTrace(const wchar_t* file, unsigned long line, HRESULT hr, const wchar_t* proc);

#define V_RETURN(op)                                                                                                   \
    if (FAILED(hr = (op)))                                                                                             \
    {                                                                                                                  \
        dxTrace(__FILEW__, __LINE__, hr, L#op);                                                                        \
        assert(0);                                                                                                     \
        return hr;                                                                                                     \
    }

#define V(op)                                                                                                          \
    if (FAILED(hr = (op)))                                                                                             \
    {                                                                                                                  \
        dxTrace(__FILEW__, __LINE__, hr, L#op);                                                                        \
        assert(0);                                                                                                     \
    }

class Window : public WindowGLFW
{
protected:
    ComPtr<ID3D12Device> m_pd3dDevice;
    INT64                m_iFencePoint;
    ComPtr<ID3D12Fence>  m_pd3dFence;
    HANDLE               m_hFenceEvent;

    UINT m_uRtvDescriptorSize;
    UINT m_uDsvDescriptorSize;
    UINT m_uCbvSrvUavDescriptorSize;

    // Command queue
    ComPtr<ID3D12CommandQueue>        m_pd3dCommandQueue;
    ComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
    ComPtr<ID3D12CommandAllocator>    m_pd3dDirectCmdAlloc;

    // Swap chain
    ComPtr<IDXGISwapChain>       m_pSwapChain;
    static constexpr DXGI_FORMAT s_backBufferFormat      = DXGI_FORMAT_R8G8B8A8_UNORM;
    static constexpr DXGI_FORMAT s_depthStencilFormat    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    static constexpr UINT        s_iSwapChainBufferCount = 2;

    ComPtr<ID3D12DescriptorHeap> m_pRTVDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> m_pDSVDescriptorHeap;

    ComPtr<ID3D12Resource> m_pSwapChainBuffers[s_iSwapChainBufferCount];
    ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

    UINT m_iCurrentFrameIndex = 0;

    D3D12_VIEWPORT m_ScreenViewport;
    D3D12_RECT     m_ScreenScissorRect;

protected:
    HRESULT initGraphicAPI();
    HRESULT createRtvAndDsvDescriptorHeaps(int extraRtvCount, int extraDsvCount);
    HRESULT createSwapChain(HWND hwnd, int width, int height, IDXGIFactory4* pDXGIFactory);
    HRESULT getHardwareAdpter(_In_ IDXGIFactory1* pFactory, _In_ BOOL bRequestHighPerformanceAdapter,
                              _In_ BOOL (*AdapterSelectionCallback)(IDXGIAdapter1*), _Out_ IDXGIAdapter1** ppAdapter);
    HRESULT createCommandObjects();
    void    flushCommandQueue();
    HRESULT resizeRenderedBuffers(int width, int height);

public:
    ~Window();

    void init(struct GameData& datas);

    void initDrawContext();
    void renderFrame();

    void setSize(const Vec2i in_windowSize) noexcept
    {
        if (in_windowSize == windowSize)
            return;

        WindowGLFW::setSize(in_windowSize);
        resizeRenderedBuffers(in_windowSize.x, in_windowSize.y);
    }
};