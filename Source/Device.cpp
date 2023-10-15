#include "DXRay/Device.h"

namespace DXR
{
    Device::Device(ComPtr<DXRDevice> device, ComPtr<IDXGIAdapter> adapter, ComPtr<DMA::Allocator> allocator)
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

    void Device::SubAssignScratchBuffer(std::vector<BottomAccelDesc>& descs, ComPtr<DMA::Allocation>& alloc)
    {
        UINT64 offset = 0;
        UINT64 maxSize = alloc->GetSize();
        D3D12_GPU_VIRTUAL_ADDRESS baseAddress = alloc->GetResource()->GetGPUVirtualAddress();

        for (auto& desc : descs)
        {
            desc.BuildDesc.ScratchAccelerationStructureData = baseAddress + offset;
            offset += desc.PrebuildInfo.ScratchDataSizeInBytes;

            if (offset > maxSize)
                DXR_THROW_FAILED(E_OUTOFMEMORY); // Throw an error if we run out of memory
        }
    }

    UINT64 Device::GetRequiredScratchBufferSize(std::vector<BottomAccelDesc>& descs)
    {
        UINT64 size = 0;

        for (auto& desc : descs) { size += desc.GetScratchBufferSize(); }

        return size;
    }

    ComPtr<DMA::Allocation> Device::AllocateAndAssignScratchBuffer(std::vector<BottomAccelDesc>& descs)
    {
        UINT64 size = GetRequiredScratchBufferSize(descs);
        ComPtr<DMA::Allocation> scratchBuffer = AllocateScratchBuffer(size);
        SubAssignScratchBuffer(descs, scratchBuffer);
        return scratchBuffer;
    }

    ComPtr<DMA::Allocation> Device::AllocateScratchBuffer(UINT64 size)
    {
        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Alignment = 0;
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        resDesc.Width = size;
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        return AllocateResource(resDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);
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
        allocDesc.CustomPool = mPool; // if mPool is nullptr, the default pool will be used

        DXR_THROW_FAILED(mAllocator->CreateResource(&allocDesc, &desc, state, nullptr, &outAlloc, {}, nullptr));
        return outAlloc;
    }

} // namespace DXR
