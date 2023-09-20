#include "DXRay/DXRayDevice.h"

DXRayDevice::DXRayDevice(const ComPtr<DXRDevice>& mDevice, const ComPtr<DMA::Allocator>& mAllocator)
    : mDevice(mDevice), mAllocator(mAllocator)
{
}

DXRayDevice::~DXRayDevice()
{
}
