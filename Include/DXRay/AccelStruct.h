#pragma once

#include "DXRay/DXRay.h"

namespace DXR
{

    struct IAccelDesc
    {
    public: // Methods
        /// @brief Get the prebuild info for the acceleration structure.
        const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& GetPrebuildInfo() const { return PrebuildInfo; }

        /// @brief Get the build description for the acceleration structure.
        const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& GetBuildDesc() const { return BuildDesc; }

        /// @brief Set the scratch buffer for the acceleration structure.
        void SetScratchBuffer(D3D12_GPU_VIRTUAL_ADDRESS scratchBuffer)
        {
            BuildDesc.ScratchAccelerationStructureData = scratchBuffer;
        }

        /// @brief Get the size of the scratch buffer needed to build the acceleration structure.
        UINT64 GetScratchBufferSize() const { return PrebuildInfo.ScratchDataSizeInBytes; }

        /// @brief Check if the acceleration structure has been allocated.
        /// Checks if BuildDesc.Inputs.NumDescs > 0, which is set by AllocateBottomAccelerationStructure or
        /// AllocateTopAccelerationStructure.
        bool HasBeenAllocated() const { return BuildDesc.Inputs.NumDescs > 0; }

    public: // Members
        /// @brief The flags to use when building the acceleration structure.
        /// These must be set before calling AllocateTopAccelerationStructure or AllocateBottomAccelerationStructure,
        /// because they are used to compute the prebuild info.
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags =
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;

        /// ------------------------------------------------------------------------
        /// ------------------ Bottom Level Acceleration Structure -----------------
        /// ------------------------------------------------------------------------

        /// @brief The geometry to create the acceleration structure for.
        /// This or pGeometries must be set for BLAS building. No need to fill both.
        /// The pointers in this vector must be valid until the acceleration structure is built.
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> Geometries = {};

        /// @brief The geometry to create the acceleration structure for, expressed as pointers.
        /// This or Geometries must be set for BLAS building. No need to fill both.
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC*> pGeometries = {};

        /// ------------------------------------------------------------------------
        /// ------------------- Top Level Acceleration Structure -------------------
        /// ------------------------------------------------------------------------

    private:
        /// @brief The prebuild info for the acceleration structure.
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};

        /// @brief The acceleration structure build information.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BuildDesc = {};

        friend class Device;
    };

} // namespace DXR
