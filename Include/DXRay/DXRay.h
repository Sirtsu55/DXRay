#pragma once

//////////////////////////////////////////////////////////////////////////
// PRECOMPILED HEADER, DO NOT INCLUDE THIS IN DXR INTERNAL HEADER FILES //
// INCLUDE THIS ONLY IN EXTERNAL PROJECTS, IE. ONLY FOR USERS OF DXR    //
//////////////////////////////////////////////////////////////////////////

// All DXRay headers
#include "DXRay/Common.h"
#include "DXRay/Device.h"
#include "DXRay/AccelStruct.h"
#include "DXRay/ShaderTable.h"

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

namespace DXR
{
    /// @brief Creates a simple device with a factory and adapter.
    /// @param debug Whether to enable debug layers.
    /// @param breakSeverity The severity of messages to break on. Only works in debug builds. If breakSeverity is
    /// UINT_MAX, no messages will be broken on. If breakSeverity is D3D12_MESSAGE_SEVERITY_WARNING, Warning and higher
    /// severity messages will be broken on. Same applies for other severities.
    std::tuple<ComPtr<IDXRDevice>, ComPtr<IDXRAdapter>, ComPtr<IDXGIFactory7>> CreateSimpleDevice(
        bool debug = false, D3D12_MESSAGE_SEVERITY breakSeverity = static_cast<D3D12_MESSAGE_SEVERITY>(UINT_MAX));
} // namespace DXR
