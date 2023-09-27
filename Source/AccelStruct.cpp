#include "DXRay/Device.h"

namespace DXR
{
    ComPtr<DMA::Allocation> Device::AllocateBottomAccelerationStructure(BottomAccelDesc& desc)
    {
        // Check if we have all required data for the function
        DXR_ASSERT(desc.pGeometries.size() > 0 || desc.Geometries.size() > 0,
                   "No geometries provided for bottom level acceleration structure");

        // Check what kind of geometry we have
        bool ppGeoms = desc.pGeometries.size() > 0;

        // Reference to the inputs for shorter code
        auto& inputs = desc.BuildDesc.Inputs;
        auto& prebuild = desc.PrebuildInfo;

        // Fill in the inputs
        inputs.DescsLayout = ppGeoms ? D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS : D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.Flags = desc.Flags;
        inputs.InstanceDescs = 0;
        inputs.NumDescs = static_cast<UINT>(ppGeoms ? desc.pGeometries.size() : desc.Geometries.size());
        if (ppGeoms)
            inputs.ppGeometryDescs = desc.pGeometries.data();
        else
            inputs.pGeometryDescs = desc.Geometries.data();
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

        // Query the prebuild info
        mDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

        // Allocate the buffer
        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Alignment = 0;
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Flags =
            D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        resDesc.Width = prebuild.ResultDataMaxSizeInBytes;
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        return AllocateResource(resDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    }
} // namespace DXR
