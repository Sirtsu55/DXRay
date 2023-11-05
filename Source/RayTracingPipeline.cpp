#include "DXRay/Device.h"

namespace DXR
{
    RayTracingPipeline Device::CreateRayTracingPipeline(const RayTracingPipelineDesc& desc)
    {
        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
        shaderConfig.MaxAttributeSizeInBytes = desc.MaxAttributeSizeInBytes;
        shaderConfig.MaxPayloadSizeInBytes = desc.MaxPayloadSizeInBytes;

        D3D12_RAYTRACING_PIPELINE_CONFIG1 pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = desc.MaxRecursionDepth;
        pipelineConfig.Flags = desc.PipelineFlags;

        D3D12_STATE_OBJECT_CONFIG stateObjectConfig = {};
        stateObjectConfig.Flags = D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS | desc.StateObjectFlags;

        D3D12_STATE_SUBOBJECT subObjects[3] = {};
        subObjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        subObjects[0].pDesc = &shaderConfig;

        subObjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
        subObjects[1].pDesc = &pipelineConfig;

        subObjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
        subObjects[2].pDesc = &stateObjectConfig;

        D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
        stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        stateObjectDesc.NumSubobjects = 3;
        stateObjectDesc.pSubobjects = subObjects;

        RayTracingPipeline pipeline;
        DXR_THROW_FAILED(mDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&pipeline.mPipelineStateObject)));
        return pipeline;
    }
} // namespace DXR
