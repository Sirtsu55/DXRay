#pragma once

#include "DXRay/Common.h"

namespace DXR
{
    struct ShaderTable
    {
    public: // Methods
        /// @brief Add a RayGen shader to the shader table. Note allows multiple ray gen shaders, incase you want to
        /// dispatch many times with different ray gen shaders. Useful for if want to switch to different camera, to
        /// avoid GPU branching, which isn't a big thing, but still.
        /// @param name The name of the shader.
        /// @return The index of the shader in the shader table.
        UINT32 AddRayGenShader(const std::wstring& name)
        {
            mRayGenShaderNames.emplace_back(name);
            mNewShadersAdded = true;
            mNeedsReallocation = mRayGenShaderNames.size() > mRayGenShadersReserved;
            return (UINT32)mRayGenShaderNames.size() - 1;
        }

        /// @brief Add a Miss shader to the shader table.
        /// @param name The name of the shader.
        /// @return The index of the shader in the shader table.
        UINT32 AddMissShader(const std::wstring& name)
        {
            mMissShaderNames.emplace_back(name);
            mNewShadersAdded = true;
            mNeedsReallocation = mMissShaderNames.size() > mMissShadersReserved;
            return (UINT32)mMissShaderNames.size() - 1;
        }

        /// @brief Add a HitGroup shader to the shader table.
        /// @param name The name of the shader.
        /// @return The index of the shader in the shader table.
        UINT32 AddHitGroupShader(const std::wstring& name)
        {
            mHitGroupShaderNames.emplace_back(name);
            mNewShadersAdded = true;
            mNeedsReallocation = mHitGroupShaderNames.size() > mHitGroupShadersReserved;
            return (UINT32)mHitGroupShaderNames.size() - 1;
        }

        /// @brief Add a Callable shader to the shader table.
        /// @param name The name of the shader.
        /// @return The index of the shader in the shader table.
        UINT32 AddCallableShader(const std::wstring& name)
        {
            mCallableShaderNames.emplace_back(name);
            mNewShadersAdded = true;
            mNeedsReallocation = mCallableShaderNames.size() > mCallableShadersReserved;
            return (UINT32)mCallableShaderNames.size() - 1;
        }

        /// @brief Reserves space for new shaders so that new allocations arent made when adding shaders. Ray generation
        /// shaders are only one per shader table, so they are not reserved. This will also be the amount of space
        /// reserved in the allocated buffer, so if there are plans to add more shaders, it is best to reserve space
        /// here, otherwise reallocations may occur when building the shader table.
        /// @param reserveMiss The number of miss shaders to reserve space for.
        /// @param reserveHitGroup The number of hit group shaders to reserve space for.
        /// @param reserveCallable The number of callable shaders to reserve space for.
        void ReserveSpace(UINT32 reserveRgen = 0, UINT32 reserveMiss = 0, UINT32 reserveHitGroup = 0,
                          UINT32 reserveCallable = 0)
        {
            mRayGenShaderNames.reserve(reserveRgen);
            mMissShaderNames.reserve(reserveMiss);
            mHitGroupShaderNames.reserve(reserveHitGroup);
            mCallableShaderNames.reserve(reserveCallable);
        }

        /// @brief Get the D3D12_DISPATCH_RAYS_DESC for the shader table.
        /// @param rgen The index of the ray gen shader in the shader table
        /// @param width The width of the dispatch.
        /// @param height The height of the dispatch.
        /// @param depth The depth of the dispatch.
        /// @return The D3D12_DISPATCH_RAYS_DESC for the shader table.
        D3D12_DISPATCH_RAYS_DESC GetRaysDesc(UINT32 rgen, UINT32 width, UINT32 height, UINT32 depth = 1) const
        {
            D3D12_DISPATCH_RAYS_DESC desc = {};
            desc.Width = width;
            desc.Height = height;
            desc.Depth = depth;

            auto virtualAddress = mShaderTable->GetResource()->GetGPUVirtualAddress();

            desc.RayGenerationShaderRecord.StartAddress =
                virtualAddress +
                DXR_ALIGN(rgen * mRayGenShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
            desc.RayGenerationShaderRecord.SizeInBytes = mRayGenShaderRecordSize;

            virtualAddress += DXR_ALIGN(mRayGenShaderNames.capacity() * mRayGenShaderRecordSize,
                                        D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

            if (mMissShadersBuilt > 0)
            {
                desc.MissShaderTable.StartAddress = virtualAddress;
                desc.MissShaderTable.SizeInBytes = mMissShadersBuilt * mMissShaderRecordSize;
                desc.MissShaderTable.StrideInBytes = mMissShaderRecordSize;
            }

            virtualAddress += DXR_ALIGN(mMissShaderNames.capacity() * mMissShaderRecordSize,
                                        D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

            if (mHitGroupShadersBuilt > 0)
            {
                desc.HitGroupTable.StartAddress = virtualAddress;
                desc.HitGroupTable.SizeInBytes = mHitGroupShadersBuilt * mHitGroupRecordSize;
                desc.HitGroupTable.StrideInBytes = mHitGroupRecordSize;
            }

            virtualAddress += DXR_ALIGN(mHitGroupShaderNames.capacity() * mHitGroupRecordSize,
                                        D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

            if (mCallableShadersBuilt > 0)
            {
                desc.CallableShaderTable.StartAddress = virtualAddress;
                desc.CallableShaderTable.SizeInBytes = mCallableShadersBuilt * mCallableRecordSize;
                desc.CallableShaderTable.StrideInBytes = mCallableRecordSize;
            }

            return desc;
        }

        /// @brief Get the allocation for the shader table.
        /// @return The allocation for the shader table.
        ComPtr<DMA::Allocation> GetShaderTableAllocation() const { return mShaderTable; }

        /// @brief Set the size of the shader records for ray gen shaders, not including the shader identifier.
        /// @param size The size of the shader records.
        void SetRayGenShaderRecordSize(UINT32 size)
        {
            DXR_ASSERT(mRayGenStartPtr == nullptr, "Cannot set shader record size after shader table has been built.");
            mRayGenShaderRecordSize =
                DXR_ALIGN(size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        }

        /// @brief Set the size of the shader records for miss shaders, not including the shader identifier.
        /// @param size The size of the shader records.
        void SetMissShaderRecordSize(UINT32 size)
        {
            DXR_ASSERT(mMissStartPtr == nullptr, "Cannot set shader record size after shader table has been built.");
            mMissShaderRecordSize =
                DXR_ALIGN(size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        }

        /// @brief Set the size of the shader records for hit group shaders, not including the shader identifier.
        /// @param size The size of the shader records.
        void SetHitGroupShaderRecordSize(UINT32 size)
        {

            DXR_ASSERT(mHitGroupStartPtr == nullptr,
                       "Cannot set shader record size after shader table has been built.");
            mHitGroupRecordSize =
                DXR_ALIGN(size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        }

        /// @brief Set the size of the shader records for callable shaders, not including the shader identifier.
        /// @param size The size of the shader records.
        void SetCallableShaderRecordSize(UINT32 size)
        {
            DXR_ASSERT(mCallableStartPtr == nullptr,
                       "Cannot set shader record size after shader table has been built.");
            mCallableRecordSize =
                DXR_ALIGN(size, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        }

        /// @brief Set the data in the shader table for a ray gen shader
        /// @param index The index of the shader in the shader table, returned from AddRayGenShader.
        /// @param pLocalRootArguments The local root arguments for the shader.
        void SetRayGenShaderRecord(UINT32 index, const void* pLocalRootArguments)
        {
            DXR_ASSERT(mRayGenStartPtr != nullptr, "Cannot set shader record before shader table has been built.");
            DXR_ASSERT(index < mRayGenShaderNames.size(), "RayGen shader index out of bounds.");
            UINT8* dest = reinterpret_cast<UINT8*>(mRayGenStartPtr) + index * mRayGenShaderRecordSize;
            memcpy(dest + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, pLocalRootArguments, mRayGenShaderRecordSize);
        }

        /// @brief Set the data in the shader table for a miss shader
        /// @param index The index of the shader in the shader table, returned from AddMissShader.
        /// @param pLocalRootArguments The local root arguments for the shader.
        void SetMissShaderRecord(UINT32 index, const void* pLocalRootArguments)
        {
            DXR_ASSERT(mMissStartPtr != nullptr, "Cannot set shader record before shader table has been built.");
            DXR_ASSERT(index < mMissShaderNames.size(), "Miss shader index out of bounds.");
            UINT8* dest = reinterpret_cast<UINT8*>(mMissStartPtr) + index * mMissShaderRecordSize;
            memcpy(dest + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, pLocalRootArguments, mMissShaderRecordSize);
        }

        /// @brief Set the data in the shader table for a hit group shader
        /// @param index The index of the shader in the shader table, returned from AddHitGroupShader.
        /// @param pLocalRootArguments The local root arguments for the shader.
        void SetHitGroupShaderRecord(UINT32 index, const void* pLocalRootArguments)
        {
            DXR_ASSERT(mHitGroupStartPtr != nullptr, "Cannot set shader record before shader table has been built.");
            DXR_ASSERT(index < mHitGroupShaderNames.size(), "HitGroup shader index out of bounds.");
            UINT8* dest = reinterpret_cast<UINT8*>(mHitGroupStartPtr) + index * mHitGroupRecordSize;
            memcpy(dest + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, pLocalRootArguments, mHitGroupRecordSize);
        }

        /// @brief Set the data in the shader table for a callable shader
        /// @param index The index of the shader in the shader table, returned from AddCallableShader.
        /// @param pLocalRootArguments The local root arguments for the shader.
        void SetCallableShaderRecord(UINT32 index, const void* pLocalRootArguments)
        {
            DXR_ASSERT(mCallableStartPtr != nullptr, "Cannot set shader record before shader table has been built.");
            DXR_ASSERT(index < mCallableShaderNames.size(), "Callable shader index out of bounds.");
            UINT8* dest = reinterpret_cast<UINT8*>(mCallableStartPtr) + index * mCallableRecordSize;
            memcpy(dest + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, pLocalRootArguments, mCallableRecordSize);
        }

    public: // Members
        // The buffer resource for the shader table.
        ComPtr<DMA::Allocation> mShaderTable;

        // The pointer to the start of each shader table.
        CHAR* mRayGenStartPtr = nullptr;
        CHAR* mMissStartPtr = nullptr;
        CHAR* mHitGroupStartPtr = nullptr;
        CHAR* mCallableStartPtr = nullptr;

        // The size of the records for each shader type.
        UINT64 mRayGenShaderRecordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT64 mMissShaderRecordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT64 mHitGroupRecordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        UINT64 mCallableRecordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

        // The names of the shaders in the shader table.
        std::vector<std::wstring> mRayGenShaderNames;
        std::vector<std::wstring> mMissShaderNames;
        std::vector<std::wstring> mHitGroupShaderNames;
        std::vector<std::wstring> mCallableShaderNames;

        // Whether or not the shader table needs to be rebuilt.
        bool mNeedsReallocation = true;
        bool mNewShadersAdded = false;

        UINT32 mRayGenShadersBuilt = 0;
        UINT32 mMissShadersBuilt = 0;
        UINT32 mHitGroupShadersBuilt = 0;
        UINT32 mCallableShadersBuilt = 0;

        UINT32 mRayGenShadersReserved = 0;
        UINT32 mMissShadersReserved = 0;
        UINT32 mHitGroupShadersReserved = 0;
        UINT32 mCallableShadersReserved = 0;
    };

} // namespace DXR
