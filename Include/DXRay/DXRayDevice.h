#pragma once

using Microsoft::WRL::ComPtr;
namespace DMA = D3D12MA;
class DXRayDevice : public IUnknown
{

    /// @brief Alias for the D3D12 device type so it's easier to change later
    /// if new versions are released
    using DXRDevice = ID3D12Device5;

public:
    /// @brief Create a DXRayDevice
    /// @param device The D3D12 device to use
    /// @param allocator The allocator to use for all allocations, if null a new allocator will be created
    DXRayDevice(ComPtr<DXRDevice> device, ComPtr<DMA::Allocator> allocator = nullptr);
    ~DXRayDevice();

    // Delete copy/move constructors and assignment operators

    DXRayDevice(DXRayDevice const&) = delete;
    DXRayDevice(DXRayDevice&&) = delete;
    DXRayDevice& operator=(DXRayDevice const&) = delete;
    DXRayDevice& operator=(DXRayDevice&&) = delete;

public:
    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    // @@@@@@@@@@@@@@@@@ Getters & Setters@@@@@@@@@@@@@@@@@@@@@
    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    /// @brief Get the D3D12 device
    /// @return The D3D12 device
    ComPtr<DXRDevice> GetD3D12Device() const { return mDevice; }

    /// @brief Get the allocator
    /// @return The allocator
    ComPtr<DMA::Allocator> GetAllocator() const { return mAllocator; }

private:
    // Member variables

    /// @brief The D3D12 device
    ComPtr<DXRDevice> mDevice;
    ComPtr<DMA::Allocator> mAllocator;
};