#pragma once

// Init graphic API headers
#if defined(USE_DX12_API)
#define GLFW_EXPOSE_NATIVE_WIN32

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

#elif defined(USE_OPENGL_API)
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>