#pragma once

#include "Engine/FileReader.hpp"
#include "Engine/Graphics/WindowDX12.hpp"
#include "Engine/Log.hpp"

#include "yaml-cpp/yaml.h"

#include <stdlib.h>

#include <d3d12.h>       // for D3D12 interface
#include <d3dcompiler.h> // For D3DCompileFromFile
#include <wrl.h>         // for Microsoft::WRL::ComPtr

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// see https://github.com/septag/glslcc to convert glsl to hlsl with reflection
struct ShaderData
{
    struct ShaderInput
    {
        std::string name;
    };

    struct ShaderTexture
    {
        int binding;
    };

    struct ShaderUniformBuffer
    {
        int binding;
    };

    std::vector<ShaderInput>         inputs;
    std::vector<ShaderTexture>       shaderTexture;
    std::vector<ShaderUniformBuffer> uniformBuffers;
};

class Shader
{
protected:
    Window&                     m_window;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12RootSignature> m_rootSignature;

public:
    wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
    {
        wchar_t* wString = new wchar_t[4096];
        MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
        return wString;
    }

    Shader(Window& window, const char* vertexPath, const char* fragmentPath) : m_window{window}
    {
        logf("Parse files: %s %s\n", vertexPath, fragmentPath);

        HRESULT          hr;
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
        ComPtr<ID3DBlob> errorBlob;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        hr = D3DCompileFromFile((LPWSTR)convertCharArrayToLPCWSTR(vertexPath), nullptr, nullptr, "main", "vs_5_0",
                                compileFlags, 0, vertexShader.GetAddressOf(), &errorBlob);
        checkCompileErrors(vertexShader, errorBlob, hr);
        ShaderData vsDatas = ParseShaderData(vertexPath);

        hr = D3DCompileFromFile((LPWSTR)convertCharArrayToLPCWSTR(fragmentPath), nullptr, nullptr, "main", "ps_5_0",
                                compileFlags, 0, pixelShader.GetAddressOf(), &errorBlob);
        checkCompileErrors(pixelShader, errorBlob, hr);
        ShaderData fsDatas = ParseShaderData(fragmentPath);

        createPipelineStateObject(window, vertexShader, pixelShader, vsDatas, fsDatas);

        log("Shader compilation done\n");
    }

    void use()
    {
        m_window.getCommandList()->SetPipelineState(m_pipelineState.Get());
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
    ShaderData ParseShaderData(std::string path)
    {
        path += ".yaml";

        ShaderData data;

        YAML::Node fileData = YAML::LoadFile(path);
        if (!fileData)
        {
            errorAndExit(std::string("Could not find setting file here: ") + path);
        }

        // Inputs
        {
            YAML::Node section = fileData["inputs"];
            if (section)
            {
                data.inputs.reserve(section.size());

                for (YAML::const_iterator it = section.begin(); it != section.end(); ++it)
                {
                    ShaderData::ShaderInput input;
                    const YAML::Node        node = (*it)["name"];
                    input.name                   = node.Scalar();
                    data.inputs.emplace_back(std::move(input));
                }
            }
        }

        // Textures
        {
            YAML::Node section = fileData["textures"];
            if (section)
            {
                data.shaderTexture.reserve(section.size());

                for (YAML::const_iterator it = section.begin(); it != section.end(); ++it)
                {
                    ShaderData::ShaderTexture texture;
                    const YAML::Node          node = (*it)["binding"];
                    texture.binding                = node.as<int>();
                    data.shaderTexture.emplace_back(std::move(texture));
                }
            }
        }

        // uniform_buffers
        {
            YAML::Node section = fileData["uniform_buffers"];
            if (section)
            {
                data.uniformBuffers.reserve(section.size());

                for (YAML::const_iterator it = section.begin(); it != section.end(); ++it)
                {
                    ShaderData::ShaderUniformBuffer buffer;
                    const YAML::Node                node = (*it)["binding"];
                    buffer.binding                       = node.as<int>();
                    data.uniformBuffers.emplace_back(std::move(buffer));
                }
            }
        }

        return data;
    }

    void addTextureToDescriptor(std::vector<ShaderData::ShaderTexture>& data,
                                std::vector<D3D12_DESCRIPTOR_RANGE>&    pd3dDescriptorRanges,
                                std::vector<D3D12_ROOT_PARAMETER>&      pd3dRootParameters,
                                std::vector<D3D12_STATIC_SAMPLER_DESC>& pd3dSamplerDescs,
                                D3D12_SHADER_VISIBILITY                 visibility)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            D3D12_DESCRIPTOR_RANGE pd3dDescriptorRange            = {};
            pd3dDescriptorRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            pd3dDescriptorRange.NumDescriptors                    = 1;
            pd3dDescriptorRange.BaseShaderRegister                = data[i].binding;
            pd3dDescriptorRange.RegisterSpace                     = 0;
            pd3dDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            pd3dDescriptorRanges.emplace_back(std::move(pd3dDescriptorRange));

            D3D12_ROOT_PARAMETER pd3dRootParameter                = {};
            pd3dRootParameter.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            pd3dRootParameter.DescriptorTable.NumDescriptorRanges = 1;
            pd3dRootParameter.DescriptorTable.pDescriptorRanges   = &pd3dDescriptorRanges.back();
            pd3dRootParameter.ShaderVisibility                    = visibility;
            pd3dRootParameters.emplace_back(std::move(pd3dRootParameter));

            D3D12_STATIC_SAMPLER_DESC pd3dSamplerDesc = {};
            pd3dSamplerDesc.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            pd3dSamplerDesc.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pd3dSamplerDesc.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pd3dSamplerDesc.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            pd3dSamplerDesc.MipLODBias                = 0;
            pd3dSamplerDesc.MaxAnisotropy             = 1;
            pd3dSamplerDesc.ComparisonFunc            = D3D12_COMPARISON_FUNC_ALWAYS;
            pd3dSamplerDesc.MinLOD                    = 0;
            pd3dSamplerDesc.MaxLOD                    = D3D12_FLOAT32_MAX;
            pd3dSamplerDesc.ShaderRegister            = data[i].binding;
            pd3dSamplerDesc.RegisterSpace             = 0;
            pd3dSamplerDesc.ShaderVisibility          = visibility;
            pd3dSamplerDescs.emplace_back(std::move(pd3dSamplerDesc));
        }
    }

    void addUniformBlockToDescriptor(std::vector<ShaderData::ShaderUniformBuffer>& data,
                                     std::vector<D3D12_ROOT_PARAMETER>&            pd3dRootParameters,
                                     D3D12_SHADER_VISIBILITY                       visibility)
    {
        for (size_t i = 0; i < data.size(); i++)
        {
            D3D12_ROOT_PARAMETER pd3dRootParameter      = {};
            pd3dRootParameter.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
            pd3dRootParameter.Descriptor.ShaderRegister = data[i].binding;
            pd3dRootParameter.Descriptor.RegisterSpace  = 0;
            pd3dRootParameter.ShaderVisibility          = visibility;
            pd3dRootParameters.emplace_back(std::move(pd3dRootParameter));
        }
    }

    HRESULT createRootSignature(Window& window, ShaderData& vsData, ShaderData& fsData)
    {
        HRESULT hr;

        std::vector<D3D12_DESCRIPTOR_RANGE>    pd3dDescriptorRanges;
        std::vector<D3D12_STATIC_SAMPLER_DESC> pd3dSamplerDescs;
        std::vector<D3D12_ROOT_PARAMETER>      pd3dRootParameters;
        int                                    textureCount = vsData.shaderTexture.size() + fsData.shaderTexture.size();
        int uniformBufferCount = vsData.uniformBuffers.size() + fsData.uniformBuffers.size();
        pd3dDescriptorRanges.reserve(textureCount);
        pd3dRootParameters.reserve(textureCount + uniformBufferCount);

        addTextureToDescriptor(vsData.shaderTexture, pd3dDescriptorRanges, pd3dRootParameters, pd3dSamplerDescs,
                               D3D12_SHADER_VISIBILITY_VERTEX);
        addTextureToDescriptor(fsData.shaderTexture, pd3dDescriptorRanges, pd3dRootParameters, pd3dSamplerDescs,
                               D3D12_SHADER_VISIBILITY_PIXEL);

        addUniformBlockToDescriptor(vsData.uniformBuffers, pd3dRootParameters, D3D12_SHADER_VISIBILITY_VERTEX);
        addUniformBlockToDescriptor(fsData.uniformBuffers, pd3dRootParameters, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // Create an empty root signature.
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {0};
        rootSignatureDesc.Flags                     = d3dRootSignatureFlags;
        rootSignatureDesc.NumParameters             = pd3dRootParameters.size();
        rootSignatureDesc.pParameters               = &pd3dRootParameters[0];
        rootSignatureDesc.NumStaticSamplers         = pd3dSamplerDescs.size();
        rootSignatureDesc.pStaticSamplers           = &pd3dSamplerDescs[0];

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        V_RETURN(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        V_RETURN(window.getDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                                         IID_PPV_ARGS(&m_rootSignature)));
    }

    HRESULT createPipelineStateObject(Window& window, ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader,
                                      ShaderData& vsData, ShaderData& fsData)
    {
        HRESULT hr;

        createRootSignature(window, vsData, fsData);

        D3D12_BLEND_DESC blendDesc                      = {0};
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
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {0};
        psoDesc.pRootSignature                     = m_rootSignature.Get();
        psoDesc.InputLayout                        = {inputElementDescs, _countof(inputElementDescs)};
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

        V_RETURN(window.getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)))

        return 0;
    }

    void checkCompileErrors(ComPtr<ID3DBlob>& shader, ComPtr<ID3DBlob>& errorBlob, HRESULT hr)
    {
        if (FAILED(hr))
        {
            if (errorBlob)
            {
                errorAndExit(std::string("Shader compilation error:\n") + (char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            V(hr)

            if (shader)
                shader->Release();
        }
    }
};