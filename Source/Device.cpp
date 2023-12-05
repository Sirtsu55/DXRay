#include "DXRay/Device.h"

namespace DXR
{
    Device::Device(ComPtr<IDXRDevice> device, ComPtr<IDXRAdapter> adapter, ComPtr<DMA::Allocator> allocator)
        : mDevice(device), mAdapter(adapter), mAllocator(allocator)
    {
        if (mAllocator == nullptr)
        {
            DMA::ALLOCATOR_DESC desc = {};
            desc.pAdapter = mAdapter.Get();
            desc.pDevice = mDevice.Get();
            desc.Flags = DMA::ALLOCATOR_FLAG_NONE;
            DXR_THROW_FAILED(DMA::CreateAllocator(&desc, &mAllocator));
        }
    }

    Device::~Device()
    {
    }

    void* Device::MapAllocationForWrite(ComPtr<DMA::Allocation>& res)
    {
        void* mapped;
        CD3DX12_RANGE readRange(0, 0);
        DXR_THROW_FAILED(res->GetResource()->Map(0, &readRange, &mapped));
        return mapped;
    }

    ComPtr<ID3D12CommandQueue> Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandQueue> cmdQueue;

        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        DXR_THROW_FAILED(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue)));

        return cmdQueue;
    }

    ComPtr<ID3D12CommandAllocator> Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12CommandAllocator> cmdAlloc;

        DXR_THROW_FAILED(mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&cmdAlloc)));

        return cmdAlloc;
    }

    ComPtr<ID3D12GraphicsCommandList4> Device::CreateCommandList(D3D12_COMMAND_LIST_TYPE type)
    {
        ComPtr<ID3D12GraphicsCommandList4> cmdList;

        DXR_THROW_FAILED(mDevice->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmdList)));

        return cmdList;
    }

    ComPtr<ID3D12Fence> Device::CreateFence(UINT64 initialValue)
    {
        ComPtr<ID3D12Fence> fence;

        DXR_THROW_FAILED(mDevice->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        return fence;
    }

    ComPtr<DMA::Allocation> Device::AllocateResource(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES state,
                                                     D3D12_HEAP_TYPE heapType, DMA::ALLOCATION_FLAGS allocFlags,
                                                     D3D12_HEAP_FLAGS heapFlags)
    {
        ComPtr<DMA::Allocation> outAlloc = nullptr;

        DMA::ALLOCATION_DESC allocDesc = {};
        allocDesc.HeapType = heapType;
        allocDesc.Flags = allocFlags;
        allocDesc.ExtraHeapFlags = heapFlags;
        allocDesc.CustomPool = mPool == nullptr ? nullptr : mPool.Get();

        DXR_THROW_FAILED(mAllocator->CreateResource(&allocDesc, &desc, state, nullptr, &outAlloc, {}, nullptr));

        return outAlloc;
    }

} // namespace DXR
