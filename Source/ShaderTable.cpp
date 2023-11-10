#include "DXRay/ShaderTable.h"

namespace DXR
{
    void Device::CreateShaderTable(ShaderTable& table, D3D12_HEAP_TYPE heap, ComPtr<ID3D12StateObject>& pipeline)
    {
        DXR_ASSERT(heap != D3D12_HEAP_TYPE_DEFAULT, "Shader table must be in heap that is CPU accessible");

        UINT64 numRgen = table.mRayGenShaderNames.capacity();
        UINT64 numMiss = table.mMissShaderNames.capacity();
        UINT64 numHitGroup = table.mHitGroupShaderNames.capacity();
        UINT64 numCallable = table.mCallableShaderNames.capacity();

        UINT64 rgenAlignedSize =
            DXR_ALIGN(numRgen * table.mRayGenShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        UINT64 missAlignedSize =
            DXR_ALIGN(numMiss * table.mMissShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        UINT64 hitGroupAlignedSize =
            DXR_ALIGN(numHitGroup * table.mHitGroupRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
        UINT64 callableAlignedSize =
            DXR_ALIGN(numCallable * table.mCallableRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

        auto tableBufDesc = CD3DX12_RESOURCE_DESC::Buffer(rgenAlignedSize + missAlignedSize + hitGroupAlignedSize +
                                                          callableAlignedSize);

        table.mShaderTable = AllocateResource(tableBufDesc, D3D12_RESOURCE_STATE_COMMON, heap,
                                              DMA::ALLOCATION_FLAG_NONE, D3D12_HEAP_FLAG_NONE);

        CHAR* pData = reinterpret_cast<CHAR*>(MapAllocationForWrite(table.mShaderTable));

        table.mRayGenStartPtr = pData;
        pData += rgenAlignedSize;
        table.mMissStartPtr = pData;
        pData += missAlignedSize;
        table.mHitGroupStartPtr = pData;
        pData += hitGroupAlignedSize;
        table.mCallableStartPtr = pData;

        // Now that we have the pointers, we can fill in the shader table with identifiers

        ComPtr<ID3D12StateObjectProperties> pipelineProps;
        DXR_THROW_FAILED(pipeline->QueryInterface(IID_PPV_ARGS(&pipelineProps)));

        for (UINT64 i = 0; i < table.mRayGenShaderNames.size(); i++)
        {
            CHAR* idStart = table.mRayGenStartPtr + i * table.mRayGenShaderRecordSize;
            memcpy(idStart, pipelineProps->GetShaderIdentifier(table.mRayGenShaderNames[i].c_str()),
                   D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }

        for (UINT64 i = 0; i < table.mMissShaderNames.size(); i++)
        {
            CHAR* idStart = table.mMissStartPtr + i * table.mMissShaderRecordSize;
            memcpy(idStart, pipelineProps->GetShaderIdentifier(table.mMissShaderNames[i].c_str()),
                   D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }

        for (UINT64 i = 0; i < table.mHitGroupShaderNames.size(); i++)
        {
            CHAR* idStart = table.mHitGroupStartPtr + i * table.mHitGroupRecordSize;
            memcpy(idStart, pipelineProps->GetShaderIdentifier(table.mHitGroupShaderNames[i].c_str()),
                   D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }

        for (UINT64 i = 0; i < table.mCallableShaderNames.size(); i++)
        {
            CHAR* idStart = table.mCallableStartPtr + i * table.mCallableRecordSize;
            memcpy(idStart, pipelineProps->GetShaderIdentifier(table.mCallableShaderNames[i].c_str()),
                   D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }

        table.mNeedsReallocation = false;

        table.mRayGenShadersBuilt = table.mRayGenShaderNames.size();
        table.mMissShadersBuilt = table.mMissShaderNames.size();
        table.mHitGroupShadersBuilt = table.mHitGroupShaderNames.size();
        table.mCallableShadersBuilt = table.mCallableShaderNames.size();

        table.mRayGenShadersReserved = numRgen;
        table.mMissShadersReserved = numMiss;
        table.mHitGroupShadersReserved = numHitGroup;
        table.mCallableShadersReserved = numCallable;
    }

} // namespace DXR
