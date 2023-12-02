#pragma once

using Microsoft::WRL::ComPtr;

namespace DXR
{
    class Fence
    {
    public:
        /// @brief Constructor
        /// @param device Device to create the fence for.
        /// @param initialValue Initial value of the fence.
        Fence(const ComPtr<ID3D12Device>& device, UINT64 initialValue = 0);

        /// @brief Destructor
        ~Fence();

        /// @brief Gets the completed value of the fence.
        UINT64 GetCompletedValue() const { return mFence->GetCompletedValue(); }

        /// @brief Gets the next value of the fence.
        UINT64 GetNextValue() const { return mNextValue; }

        /// @brief Wait for a value on the GPU.
        void Wait(UINT64 value);

        /// @brief Signal the fence.
        void Signal(UINT64 value);

        /// @brief Wait for a value on the GPU.
        void Signal(const ComPtr<ID3D12CommandQueue>& cmdQueue, UINT64 value);

        /// @brief Signal the fence with the next value.
        /// @param cmdQueue Command queue to signal the fence on.
        /// @return The value that was signaled / the value to wait for next.
        UINT64 Signal(const ComPtr<ID3D12CommandQueue>& cmdQueue);

        /// @brief Signal the fence with the next value.
        /// @return The value that was signaled / the value to wait for next.
        UINT64 Signal();

        /// @brief Wait for the all the operations on the GPU to complete.
        /// @param cmdQueue Command queue to wait on.
        void WaitForCommandQueue(const ComPtr<ID3D12CommandQueue>& cmdQueue);

    private:
        /// @brief The fence object.
        ComPtr<ID3D12Fence> mFence = nullptr;

        /// @brief The fence event.
        HANDLE mFenceEvent = nullptr;

        /// @brief The next value to signal.
        UINT64 mNextValue = 0;
    };
} // namespace DXR
