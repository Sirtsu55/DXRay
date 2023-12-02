#include "DXRay/Fence.h"

namespace DXR
{
    Fence::Fence(const ComPtr<ID3D12Device>& device, UINT64 initialValue) : mNextValue(initialValue + 1)
    {
        DXR_THROW_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

        mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        DXR_ASSERT(mFenceEvent != nullptr, "Failed to create fence event.");
    }

    Fence::~Fence()
    {
        CloseHandle(mFenceEvent);
    }

    void Fence::Wait(UINT64 value)
    {
        if (mFence->GetCompletedValue() < value)
        {
            DXR_THROW_FAILED(mFence->SetEventOnCompletion(value, mFenceEvent));
            WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
        }
    }

    void Fence::Signal(UINT64 value)
    {
        DXR_THROW_FAILED(mFence->Signal(value));

        // Update the next value
        mNextValue = max(mNextValue, value + 1);
    }

    void Fence::Signal(const ComPtr<ID3D12CommandQueue>& cmdQueue, UINT64 value)
    {
        DXR_THROW_FAILED(cmdQueue->Signal(mFence.Get(), value));

        // Update the next value
        mNextValue = max(mNextValue, value + 1);
    }

    UINT64 Fence::Signal(const ComPtr<ID3D12CommandQueue>& cmdQueue)
    {
        UINT64 waitVal = mNextValue;

        Signal(cmdQueue, mNextValue);

        // Return the next value, because it was incremented in the Signal() call.
        return waitVal;
    }

    UINT64 Fence::Signal()
    {
        UINT64 waitVal = mNextValue;

        Signal(mNextValue);

        // Return the value to wait for, because it was incremented in the Signal() call.
        return waitVal;
    }

    void Fence::WaitForCommandQueue(const ComPtr<ID3D12CommandQueue>& cmdQueue)
    {
        Wait(Signal(cmdQueue));
    }

    Fence Device::CreateFence(UINT64 initialValue)
    {
        return Fence(mDevice, initialValue);
    }

} // namespace DXR
