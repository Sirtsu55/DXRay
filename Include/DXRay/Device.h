#pragma once
#include "DXRay/AccelStruct.h"
#include "DXRay/RayTracingPipeline.h"

// Namespace is too long to type out, so shorten it to DMA: |D|3D12 |M|emory |A|llocator
namespace DMA = D3D12MA;

namespace DXR
{
    // Ease of use of ComPtr
    using Microsoft::WRL::ComPtr;

    /// @brief A wrapper around the D3D12 to use DXR.
    class Device
    {
        /// @brief Alias for the D3D12 device type so it's easier to change later
        /// if new versions are released
        using DXRDevice = ID3D12Device7;
        using DXRAdapter = IDXGIAdapter;

    public:
        /// @brief Create a Device
        /// @param device The D3D12 device to use
        /// @param allocator The allocator to use for all allocations, if null a new allocator will be created
        /// @note General Tips:
        /// - To get complete error messages, enable the debug layer and the info queue, DXRay only throws exceptions
        /// based on HRESULTs
        /// - If you want to use a custom allocator, create it before creating the device
        /// - If you want to use a custom pool, set it after creating the device
        Device(ComPtr<DXRDevice> device, ComPtr<IDXGIAdapter> adapter, ComPtr<DMA::Allocator> allocator = nullptr);
        ~Device();

        // Delete copy/move constructors and assignment operators

        Device(Device const&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device const&) = delete;
        Device& operator=(Device&&) = delete;

    public:
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@ Getters & Setters @@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Get the D3D12 device
        /// @return The D3D12 device
        ComPtr<DXRDevice> GetD3D12Device() const { return mDevice; }

        /// @brief Get the allocator
        /// @return The allocator
        ComPtr<DMA::Allocator> GetAllocator() const { return mAllocator; }

        /// @brief Allocate a big scratch buffer that will be used for all acceleration structures
        /// This is the recommended function to call, because it will take into account the alignment requirements
        /// @param descs The descriptions of the acceleration structures which will be assigned a region of the scratch
        UINT64 GetRequiredScratchBufferSize(std::vector<AccelerationStructureDesc>& descs);

        /// @brief Get the pool
        /// @return The pool, or null if no pool is set
        DMA::Pool* GetPool() const { return mPool; }

        /// @brief Set the pool
        /// @param pool The pool to use for all allocations
        /// @note This will override the existing pool if one is already set
        void SetPool(DMA::Pool* pool) { mPool = pool; }

        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Utilities @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Allocate a resource
        /// @param desc The description of the resource
        /// @param state The initial state of the resource
        /// @param heapType The type of heap to use, default is DEFAULT (GPU only)
        /// @param allocFlags The allocation flags to use, default is NONE
        /// @param heapFlags The heap flags to use, default is NONE
        /// @return The new resource
        ComPtr<DMA::Allocation> AllocateResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES state,
                                                 D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT,
                                                 DMA::ALLOCATION_FLAGS allocFlags = DMA::ALLOCATION_FLAG_NONE,
                                                 D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE);

        /// @brief Map a resource for only writing. It is always recommended to map persistently if the resource is
        /// located in CPU visible memory to avoid Map/Unmap overhead.
        /// @param resource The resource to map
        /// @return The mapped pointer
        void* MapAllocationForWrite(ComPtr<DMA::Allocation>& res);

        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Accel Struct @@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Allocate a new acceleration structure. When allocating a bottom level acceleration structure,
        /// the returned allocation's GPU virtual address is used for instance description acceleration structure field.
        /// @param desc The description of the acceleration structure. Will be modified and used when building the
        /// acceleration structure.
        /// @return The new acceleration structure.
        ComPtr<DMA::Allocation> AllocateAccelerationStructure(AccelerationStructureDesc& desc);

        /// @brief Build an acceleration structure
        /// @param desc The description of the acceleration structure to build
        /// @param cmdList The command list to use for building
        void BuildAccelerationStructure(AccelerationStructureDesc& desc, ComPtr<ID3D12GraphicsCommandList4>& cmdList);

        /// @brief Allocate a scratch buffer for building a bottom level acceleration structure. It will take into
        /// account the alignment requirements.
        /// @param desc The description of the acceleration structure which will be assigned a region of the scratch
        /// buffer
        /// @param alloc The scratch buffer to use. This allocation should be of the size returned by
        /// GetRequiredScratchBufferSize(...) to ensure that it is large enough.
        void AssignScratchBuffer(std::vector<AccelerationStructureDesc>& descs, ComPtr<DMA::Allocation>& alloc);

        /// @brief Allocate a big scratch buffer that will be used for all acceleration structures and assign regions
        /// to each acceleration structure. This is a convenience function that calls GetRequiredScratchBufferSize(...),
        /// AllocateScratchBuffer(...) and AssignScratchBuffer(...) in that order.
        /// @param descs The descriptions of the acceleration structures which will be assigned a region of the scratch
        /// buffer
        /// @return The new scratch buffer
        ComPtr<DMA::Allocation> AllocateAndAssignScratchBuffer(std::vector<AccelerationStructureDesc>& descs);

        /// @brief Allocate a scratch buffer of the given size
        /// @param size The size of the scratch buffer
        /// @return The new scratch buffer
        ComPtr<DMA::Allocation> AllocateScratchBuffer(UINT64 size);

        /// @brief Allocate a instance buffer for a top level acceleration structure. These must then be filled with
        /// valid instance descriptions when building the acceleration structure. Can be discarded after building, but
        /// better if reused every frame.
        /// @param numInstances The number of instances to allocate space for
        /// @return The new instance buffer
        /// @note This function will allocate the buffer in GPU_UPLOAD heap type, so it can be directly copied to.
        /// This means that the system should support ReBAR, otherwise the allocation will fail. This is why AgilitySDK
        /// is downloaded with the project, to ensure that the system supports ReBAR. If you want to use a different
        /// heap type, use AllocateResource(...) instead and manage the copy yourself.
        ComPtr<DMA::Allocation> AllocateInstanceBuffer(UINT64 numInstances);

        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@ Ray Tracing Pipeline @@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Create a ray tracing pipeline
        /// @param desc The description of the pipeline, modifiable even after construction, but not implemented yet.
        /// Open an issue if needed
        RayTracingPipeline CreateRayTracingPipeline(const PipelineDesc& desc);

        /// @brief Create a StateCollection, You should call Add... to add all the subobjects before calling this
        /// function. This function will compile all the subobjects to create a state collection that can be used to
        /// expand a pipeline. This can take some time, so it is recommended to do this on a separate thread if
        /// performance is critical.
        /// @return The new StateCollection
        void CreateStateCollection(StateCollection& collection);

        /// @brief Expand a ray tracing pipeline with the given collection. Everything will be exported from the
        /// collection and added to the pipeline. If StateCollection isn't compiled, it will be compiled on this thread.
        /// @param pipeline The pipeline to expand, will be modified and replaced with a new RTPSO
        /// @param collection The collection to expand with. This can be precompiled or not.
        /// If not compiled, the compilation process happens on this thread and this function may take some time.
        void ExpandRayTracingPipeline(RayTracingPipeline& pipeline, StateCollection& collection);

    private: // Internal methods
        /// @brief Allocate a bottom level acceleration structure, used by AllocateAccelerationStructure(...) if the
        /// type is bottom level
        ComPtr<DMA::Allocation> InternalAllocateBottomAccelerationStructure(AccelerationStructureDesc& desc);

        /// @brief Allocate a top level acceleration structure, used by AllocateAccelerationStructure(...) if the type
        /// is top level
        ComPtr<DMA::Allocation> InternalAllocateTopAccelerationStructure(AccelerationStructureDesc& desc);

    private:
        /// @brief The D3D12 device
        ComPtr<DXRDevice> mDevice = nullptr;

        /// @brief The DXGI adapter
        ComPtr<DXRAdapter> mAdapter = nullptr;

        /// @brief The allocator to use for all allocations
        ComPtr<DMA::Allocator> mAllocator = nullptr;

        /// @brief The pool to use for all allocations
        DMA::Pool* mPool = nullptr;
    };
} // namespace DXR
