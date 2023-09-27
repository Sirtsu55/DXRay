#pragma once
#include "DXRay/AccelStruct.h"

namespace DXR
{
    // Ease of use of ComPtr
    using Microsoft::WRL::ComPtr;

    // Namespace is too long to type out, so shorten it to DMA: |D|3D12 |M|emory |A|llocator
    namespace DMA = D3D12MA;

    /// @brief A wrapper around the D3D12 to use DXR.
    class Device
    {
        /// @brief Alias for the D3D12 device type so it's easier to change later
        /// if new versions are released
        using DXRDevice = ID3D12Device5;
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
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@ Getters & Setters @@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Get the D3D12 device
        /// @return The D3D12 device
        ComPtr<DXRDevice> GetD3D12Device() const { return mDevice; }

        /// @brief Get the allocator
        /// @return The allocator
        ComPtr<DMA::Allocator> GetAllocator() const { return mAllocator; }

        /// @brief Get the pool
        /// @return The pool, or null if no pool is set
        DMA::Pool* GetPool() const { return mPool; }

        /// @brief Set the pool
        /// @param pool The pool to use for all allocations
        /// @note This will override the existing pool if one is already set
        void SetPool(DMA::Pool* pool) { mPool = pool; }

        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@ Accel Struct @@@@@@@@@@@@@@@@@@@@@
        // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        /// @brief Allocate a new bottom level acceleration structure
        /// @param desc The description of the acceleration structure

        /// @brief Allocate a new bottom level acceleration structure
        /// @param desc The description of the acceleration structure. Will be modified and used when building the
        /// acceleration structure.
        /// @return The new acceleration structure.
        ComPtr<DMA::Allocation> AllocateBottomAccelerationStructure(BottomAccelDesc& desc);

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
