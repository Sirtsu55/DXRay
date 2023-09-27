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

// All DXRay headers
#include "DXRay/Device.h"
#include "DXRay/AccelStruct.h"

// Enable debugging by default in debug builds, disable in release builds unless explicitly enabled
#ifndef DXR_ENABLE_DEBUG || _DEBUG
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
