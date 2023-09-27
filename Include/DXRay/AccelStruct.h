#pragma once

#include "DXRay/DXRay.h"

namespace DXR
{
    /// @brief Contains the information needed to create a bottom-level acceleration structure.
    /// @note Steps to create a bottom-level acceleration structure:
    /// 1. Fill in either Geometries or pGeometries, and flags, other fields aren't required when allocating the
    /// acceleration structure.
    /// 2. Call AllocateBottomAccelerationStructure to get a handle to the acceleration structure.
    /// 3. Fill in ScratchBuffer field.
    /// 4. Call BuildBottomAccelerationStructure to build the acceleration structure.
    struct BottomAccelDesc
    {
        /// @brief The geometry to create the acceleration structure for.
        /// This or pGeometries must be set. No need to fill both.
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> Geometries = {};

        /// @brief The geometry to create the acceleration structure for, expressed as pointers.
        /// This or Geometries must be set. No need to fill both.
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC*> pGeometries = {};

        /// @brief The flags to use when building the acceleration structure.
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags =
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

    public:
        /// @brief Get the prebuild info for the acceleration structure.
        const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& GetPrebuildInfo() const { return PrebuildInfo; }

        /// @brief Get the build description for the acceleration structure.
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& GetBuildDesc() const { return BuildDesc; }

        /// @brief Set the scratch buffer for the acceleration structure.
        void SetScratchBuffer(D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer)
        {
            BuildDesc.ScratchAccelerationStructureData = scratchBuffer;
        }

    private:
        /// @brief The prebuild info for the acceleration structure.
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};

        /// @brief The acceleration structure build information.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BuildDesc = {};

        friend class Device;
    };
} // namespace DXR
