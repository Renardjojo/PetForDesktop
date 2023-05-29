#pragma once

#include "Engine/FileReader.hpp"
#include "Engine/Log.hpp"

#include <stdlib.h>

#include <d3d12.h>       // for D3D12 interface
#include <d3dcompiler.h> // For D3DCompileFromFile
#include <wrl.h>         // for Microsoft::WRL::ComPtr

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Shader
{
protected:
    Window&                     m_window;
    ComPtr<ID3D12PipelineState> m_pPso;
    ComPtr<ID3D12RootSignature> m_rootSignature;

public:
    Shader(Window& window, LPCWSTR vertexPath, LPCWSTR fragmentPath) : m_window{window}
    {
        logf("Parse files: %s %s\n", vertexPath, fragmentPath);

        HRESULT          hr;
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        V(D3DCompileFromFile(vertexPath, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr))

        V(D3DCompileFromFile(fragmentPath, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader,
                             nullptr))

        createPipelineStateObject(window, vertexShader, pixelShader);

        log("Shader compilation done\n");
    }

    void use()
    {
        m_window.getCommandList()->SetPipelineState(m_pPso.Get());
    }

    void setBool(const char* name, bool value) const
    {
    }

    void setInt(const char* name, int value) const
    {
    }

    void setVec2(const char* name, float v1, float v2) const noexcept
    {
    }

    void setVec4(const char* name, float v1, float v2, float v3, float v4) const noexcept
    {
    }

    void setFloat(const char* name, float value) const
    {
    }

private:
    HRESULT createPipelineStateObject(Window& window, ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader)
    {
        HRESULT hr;

        // Create an empty root signature.
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        rootSignatureDesc.NumParameters     = 0;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pParameters       = nullptr;
        rootSignatureDesc.pStaticSamplers   = nullptr;

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        V_RETURN(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        V_RETURN(window.getDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                         IID_PPV_ARGS(&m_rootSignature)));

        D3D12_BLEND_DESC blendDesc                      = {};
        blendDesc.AlphaToCoverageEnable                 = FALSE;
        blendDesc.RenderTarget[0].BlendEnable           = TRUE;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode              = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode              = D3D12_CULL_MODE_BACK;

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout                        = {inputElementDescs, _countof(inputElementDescs)};
        psoDesc.pRootSignature                     = m_rootSignature.Get();
        psoDesc.VS = {reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize()};
        psoDesc.PS = {reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize()};
        psoDesc.RasterizerState                 = rasterizerDesc;
        psoDesc.BlendState                      = blendDesc;
        psoDesc.DepthStencilState.DepthEnable   = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask                      = UINT_MAX;
        psoDesc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets                = 1;
        psoDesc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count                = 1;

        V_RETURN(window.getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPso)))

        return 0;
    }
};