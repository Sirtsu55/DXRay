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
#include "DXRay/Common.h"
#include "DXRay/Device.h"
#include "DXRay/AccelStruct.h"

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
