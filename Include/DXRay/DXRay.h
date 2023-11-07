#pragma once

//////////////////////////////////////////////////////////////////////////
// PRECOMPILED HEADER, DO NOT INCLUDE THIS IN DXR INTERNAL HEADER FILES //
// INCLUDE THIS ONLY IN EXTERNAL PROJECTS, IE. ONLY FOR USERS OF DXR    //
//////////////////////////////////////////////////////////////////////////

// Windows Header Files
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>

// DirectX Header Files
#include <directx/d3d12.h>
#include <directx/d3dx12.h>

// D3D12 Memory Allocator
#include <d3d12memalloc.h>

// STL Headers
#include <unordered_map>
#include <vector>

// All DXRay headers
#include "DXRay/Device.h"
#include "DXRay/AccelStruct.h"
#include "DXRay/RaytracingPipeline.h"

// Use the Agility SDK Binary Version of D3D12
#define DXRAY_AGILITY_SDK_IMPLEMENTATION                                                                               \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        __declspec(dllexport) extern const UINT D3D12SDKVersion = 711;                                                 \
    }                                                                                                                  \
                                                                                                                       \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8"AgilitySDK\\";                                   \
    }

// Helper Defines
#define DXR_ALIGN(num, alignment) (((num) + alignment - 1) & ~(alignment - 1))

// Enable debugging by default in debug builds, disable in release builds unless explicitly enabled
#if defined(DXR_ENABLE_DEBUG) || _DEBUG
#define DXR_LOG_DEBUG(msg) OutputDebugString(msg)
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
#define DXR_LOG_DEBUG(msg)
#define DXR_ASSERT(cond, msg)
#define DXR_THROW_FAILED(hresult)
#endif
