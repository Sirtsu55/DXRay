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

// Check if want to use the Agility SDK Binary Version of D3D12
#ifdef DXRAY_AGILITY_SDK_VERSION

#define DXRAY_AGILITY_SDK_IMPLEMENTATION                                                                               \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        __declspec(dllexport) extern const UINT D3D12SDKVersion = DXRAY_AGILITY_SDK_VERSION;                           \
    }                                                                                                                  \
                                                                                                                       \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        __declspec(dllexport) extern const char8_t* D3D12SDKPath = u8"" DXRAY_AGILITY_SDK_PATH;                        \
    }

#else
#define DXRAY_AGILITY_SDK_IMPLEMENTATION                                                                               \
    static_assert(false, "Turn DXRAY_USE_AGILITY_SDK option on in CMakeLists.txt to use Agility SDK")
#endif // DXRAY_AGILITY_SDK_VERSION
