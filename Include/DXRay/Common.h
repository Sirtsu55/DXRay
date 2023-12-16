#pragma once

// Windows Header Files
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>

// DirectX Header Files
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>

// D3D12 Memory Allocator
#include <d3d12memalloc.h>

// DirectXMath Header Files
#include <DirectXMath.h>
using namespace DirectX;

// STL Headers
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace DXR
{
    using Microsoft::WRL::ComPtr;

    /// @brief Alias for the D3D12 device type so it's easier to change later
    /// if new versions are released.
    using IDXRDevice = ID3D12Device7;
    using IDXRAdapter = IDXGIAdapter1;
    using IDXRFactory = IDXGIFactory7;

} // namespace DXR

// D3D12 Memory Allocator alias
// Namespace is too long to type out, so shorten it to DMA: |D|3D12 |M|emory |A|llocator.
namespace DMA = D3D12MA;

// Helper Defines
#define DXR_ALIGN(num, alignment) (((num) + alignment - 1) & ~(alignment - 1))

// Enable debugging by default in debug builds, disable in release builds unless explicitly enabled.
#if !defined(DXR_DISABLE_DEBUG)
#define DXR_LOG_DEBUG(msg) OutputDebugStringA(msg)
#define DXR_ASSERT(cond, msg)                                                                                          \
    if (!(cond))                                                                                                       \
    {                                                                                                                  \
        DXR_LOG_DEBUG(msg);                                                                                            \
        __debugbreak();                                                                                                \
    }
#define DXR_THROW_FAILED(hresult)                                                                                      \
    if (FAILED(hresult))                                                                                               \
    {                                                                                                                  \
        __debugbreak();                                                                                                \
    }
#else
#define DXR_LOG_DEBUG(msg) ((void)0)
#define DXR_ASSERT(cond, msg) ((void)0)
#define DXR_THROW_FAILED(hresult) hresult
#endif
