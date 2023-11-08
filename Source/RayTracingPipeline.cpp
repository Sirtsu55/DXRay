#include "DXRay/Device.h"

namespace DXR
{
    RayTracingPipeline Device::CreateRayTracingPipeline(const PipelineDesc& desc)
    {
        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
        shaderConfig.MaxAttributeSizeInBytes = desc.MaxAttributeSizeInBytes;
        shaderConfig.MaxPayloadSizeInBytes = desc.MaxPayloadSizeInBytes;

        D3D12_RAYTRACING_PIPELINE_CONFIG1 pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = desc.MaxRecursionDepth;
        pipelineConfig.Flags = desc.PipelineFlags;

        D3D12_STATE_OBJECT_CONFIG stateObjectConfig = {};
        stateObjectConfig.Flags = D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS;

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

        RayTracingPipeline pipeline = {};
        pipeline.mPipelineStateDesc = desc;
        DXR_THROW_FAILED(mDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&pipeline.mPipelineStateObject)));
        return pipeline;
    }

    void Device::CreateStateCollection(StateCollection& collection)
    {
        D3D12_STATE_OBJECT_CONFIG stateObjectConfig = {};
        stateObjectConfig.Flags = D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS |
                                  D3D12_STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITIONS;

        D3D12_STATE_SUBOBJECT subObject = {};
        subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
        subObject.pDesc = &stateObjectConfig;
        collection.mSubobjects.push_back(subObject);

        D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
        stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_COLLECTION;
        stateObjectDesc.NumSubobjects = collection.mSubobjects.size();
        stateObjectDesc.pSubobjects = collection.mSubobjects.data();

        DXR_THROW_FAILED(mDevice->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&collection.mStateObject)));
    }

    void Device::ExpandRayTracingPipeline(RayTracingPipeline& pipeline, StateCollection& collection)
    {
        if (collection.mStateObject == nullptr)
        {
            CreateStateCollection(collection);
        }

        // Copy the pipeline state desc

        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
        shaderConfig.MaxAttributeSizeInBytes = pipeline.mPipelineStateDesc.MaxAttributeSizeInBytes;
        shaderConfig.MaxPayloadSizeInBytes = pipeline.mPipelineStateDesc.MaxPayloadSizeInBytes;

        D3D12_RAYTRACING_PIPELINE_CONFIG1 pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = pipeline.mPipelineStateDesc.MaxRecursionDepth;
        pipelineConfig.Flags = pipeline.mPipelineStateDesc.PipelineFlags;

        D3D12_STATE_OBJECT_CONFIG stateObjectConfig = {};
        stateObjectConfig.Flags = D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS;

        D3D12_EXISTING_COLLECTION_DESC collectionDesc = {};
        collectionDesc.pExistingCollection = collection.mStateObject.Get();
        collectionDesc.NumExports = 0;
        collectionDesc.pExports = nullptr;

        D3D12_STATE_SUBOBJECT subObjects[4] = {};
        subObjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        subObjects[0].pDesc = &shaderConfig;

        subObjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG1;
        subObjects[1].pDesc = &pipelineConfig;

        subObjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
        subObjects[2].pDesc = &stateObjectConfig;

        subObjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION;
        subObjects[3].pDesc = &collectionDesc;

        D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
        stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        stateObjectDesc.NumSubobjects = 4;
        stateObjectDesc.pSubobjects = subObjects;

        // Now add the collection to the pipeline

        ComPtr<ID3D12StateObject> newPipeline = nullptr;
        DXR_THROW_FAILED(mDevice->AddToStateObject(&stateObjectDesc, pipeline.mPipelineStateObject.Get(),
                                                   IID_PPV_ARGS(&newPipeline)));

        pipeline.mPipelineStateObject = newPipeline;
    }

} // namespace DXR
