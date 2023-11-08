#pragma once

namespace DXR
{
    using Microsoft::WRL::ComPtr;

    struct PipelineDesc
    {
        UINT MaxRecursionDepth = 1;
        UINT MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
        UINT MaxPayloadSizeInBytes = 256;

        /// @brief Ray Tracing pipeline config flags.
        D3D12_RAYTRACING_PIPELINE_FLAGS PipelineFlags = D3D12_RAYTRACING_PIPELINE_FLAG_NONE;
    };

    /// @brief DXIL Library Wrapper, it contains the shader blob and the exported symbols.
    struct ShaderLibrary
    {
    public: // Methods
        /// @brief Default constructor.
        ShaderLibrary() = default;

        /// @brief Construct a new Shader Library object.
        ShaderLibrary(ComPtr<ID3DBlob> shaderBlob) : mShaderBlob(shaderBlob)
        {
            mDesc.DXILLibrary.BytecodeLength = mShaderBlob->GetBufferSize();
            mDesc.DXILLibrary.pShaderBytecode = mShaderBlob->GetBufferPointer();
        }

        /// @brief Add an export to the shader library. If all the symbols are needed, don't call this function, because
        /// the D3D12 API will automatically export all the symbols.
        /// @param name The name of the export. Use this for referencing a given shader/subobject.
        /// @param exportName The name of the export to rename. If this is not null, it will export the symbol with this
        /// name in the library and rename it to the name parameter.
        inline void AddExport(LPCWSTR name, LPCWSTR exportName = nullptr)
        {
            D3D12_EXPORT_DESC exportDesc = {};
            exportDesc.Name = name;
            exportDesc.ExportToRename = exportName;
            exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;

            mExportedSymbols.push_back(exportDesc);
        }

        /// @brief Get the shader library description.
        /// @return The shader library description.
        inline D3D12_DXIL_LIBRARY_DESC GetDesc()
        {
            mDesc.NumExports = static_cast<UINT>(mExportedSymbols.size());
            mDesc.pExports = mExportedSymbols.data();

            return mDesc;
        }

    private: // Members
        /// @brief The shader blob.
        ComPtr<ID3DBlob> mShaderBlob = nullptr;

        /// @brief The shader library description.
        D3D12_DXIL_LIBRARY_DESC mDesc = {};

        /// @brief Exports of the shader library.
        std::vector<D3D12_EXPORT_DESC> mExportedSymbols = {};
    };

    /// @brief A Collection state object that can be incrementally added to the RayTracingPipeline.
    /// All of the objects that are added to this MUST be alive until this object is alive, since it doesn't store a
    /// hard reference to them. It is useful when you don't have all the shaders at once and don't want to recompile the
    /// whole pipeline. D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG,
    /// D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG and D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG
    /// will be inherited from the RayTracingPipeline, that this will link to, which is the whole point of this class.
    /// Tailored for effieciency and minimizing allocations. Unlike CD3DX12_STATE_OBJECT_DESC, this doesn't use
    /// std::list, but std::vector.
    struct StateCollection
    {
    public: // Constructors
        /// @brief Default constructor.
        StateCollection() = default;

        ~StateCollection()
        {
            for (auto& subobjectData : mSubobjectData) { delete subobjectData; }
        }

    public: // Methods
        /// @brief Add a Shader Library to the collection. The user MUST ensure that the shader library is alive
        /// when the StateCollection is alive.
        /// @param shaderLibrary The shader library to add.
        /// @return The index of the shader library in the collection.
        UINT32 AddShaderLibrary(ShaderLibrary& shaderLibrary)
        {
            // Create the subobject. Yes, this is inefficient, we can preallocate some space, but that just adds extra
            // complexity.
            D3D12_DXIL_LIBRARY_DESC* libraryDesc = new D3D12_DXIL_LIBRARY_DESC();
            *libraryDesc = shaderLibrary.GetDesc();

            D3D12_STATE_SUBOBJECT subobject = {};
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            subobject.pDesc = libraryDesc;

            mSubobjects.push_back(subobject);
            mSubobjectData.push_back(libraryDesc);

            return static_cast<UINT32>(mSubobjects.size() - 1);
        }

        /// @brief Add a subobject to the collection.
        /// @param rootSignature The root signature to add.
        /// @return The index of the root signature in the collection.
        UINT32 AddGlobalRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
        {
            D3D12_GLOBAL_ROOT_SIGNATURE* globalRootSignature = new D3D12_GLOBAL_ROOT_SIGNATURE();
            globalRootSignature->pGlobalRootSignature = rootSignature.Get();

            D3D12_STATE_SUBOBJECT subobject = {};
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
            subobject.pDesc = globalRootSignature;

            mSubobjects.push_back(subobject);
            mSubobjectData.push_back(globalRootSignature);

            return static_cast<UINT32>(mSubobjects.size() - 1);
        }

        /// @brief Add a subobject to the collection.
        /// @param rootSignature The root signature to add.
        /// @return The index of the root signature in the collection.
        UINT32 AddLocalRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
        {
            D3D12_LOCAL_ROOT_SIGNATURE* localRootSignature = new D3D12_LOCAL_ROOT_SIGNATURE();
            localRootSignature->pLocalRootSignature = rootSignature.Get();

            D3D12_STATE_SUBOBJECT subobject = {};
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
            subobject.pDesc = localRootSignature;

            mSubobjects.push_back(subobject);
            mSubobjectData.push_back(localRootSignature);

            return static_cast<UINT32>(mSubobjects.size() - 1);
        }

        /// @brief Add a hit group to the collection.
        /// @param hitGroupName The name of the hit group.
        /// @param closestHitSymbol The name of the closest hit symbol, it should be exported by a shader library.
        /// @param anyHitSymbol The name of the any hit symbol, it should be exported by a shader library.
        /// @param intersectionSymbol The name of the intersection symbol, it should be exported by a shader library.
        /// @return The index of the hit group in the collection.
        UINT32 AddHitGroup(LPCWSTR hitGroupName, LPCWSTR closestHitSymbol, LPCWSTR anyHitSymbol = nullptr,
                           LPCWSTR intersectionSymbol = nullptr)
        {
            D3D12_HIT_GROUP_DESC* hitGroupDesc = new D3D12_HIT_GROUP_DESC();
            hitGroupDesc->HitGroupExport = hitGroupName;
            hitGroupDesc->ClosestHitShaderImport = closestHitSymbol;
            hitGroupDesc->AnyHitShaderImport = anyHitSymbol;
            hitGroupDesc->IntersectionShaderImport = intersectionSymbol;
            hitGroupDesc->Type =
                intersectionSymbol ? D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE : D3D12_HIT_GROUP_TYPE_TRIANGLES;

            D3D12_STATE_SUBOBJECT subobject = {};
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            subobject.pDesc = hitGroupDesc;

            mSubobjects.push_back(subobject);
            mSubobjectData.push_back(hitGroupDesc);

            return static_cast<UINT32>(mSubobjects.size() - 1);
        }

        /// @brief Add an association to the collection.
        /// @param subobjectIndex The index of the subobject to associate.
        /// @param associations The associations to add.
        /// @param copy If true, the associations will be copied, otherwise they will be referenced. If true, then new
        /// heap allocations will be made in this case. If referenced, the user MUST ensure that the associations
        /// AND the strings in the vector are alive when the StateCollection is compiled.
        void AddAssociations(UINT32 subobjectIndex, std::vector<LPCWSTR> associations, bool copy = true)
        {
            D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* association = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION();
            association->NumExports = static_cast<UINT>(associations.size());
            association->pExports = associations.data(); // modified if copy is true
            association->pSubobjectToAssociate = &mSubobjects[subobjectIndex];

            D3D12_STATE_SUBOBJECT subobject = {};
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
            subobject.pDesc = association;

            // If we are copying, we need to copy the associations, so that the user can delete them.
            if (copy)
            {
                // Copy the associations, so that the user can delete them.
                LPCWSTR* associationsCopy = new LPCWSTR[associations.size()];

                // Get the size of all the strings.
                UINT32 size = 0;
                for (auto& association : associations) { size += static_cast<UINT32>(wcslen(association)); }

                // Copy the strings.
                WCHAR* associationsCopyPtr = new WCHAR[size + 1];

                UINT32 offset = 0;
                for (int i = 0; i < associations.size(); i++)
                {
                    wcscpy_s(associationsCopyPtr + offset, size + 1 - offset, associations[i]);
                    associationsCopy[i] = associationsCopyPtr + offset;
                    offset += static_cast<UINT32>(wcslen(associations[i])) + 1;
                }

                mSubobjectData.push_back(associationsCopyPtr);
                mSubobjectData.push_back(associationsCopy);

                // Modify the association to point to the copied strings.
                association->pExports = associationsCopy;
            }

            mSubobjects.push_back(subobject);
            mSubobjectData.push_back(association);
        }

    public: // Members
        D3D12_STATE_OBJECT_FLAGS StateObjectFlags = D3D12_STATE_OBJECT_FLAG_NONE;

    private: // Members
        /// @brief The subobjects that compose the collection.
        std::vector<D3D12_STATE_SUBOBJECT> mSubobjects = {};

        /// @brief The subobject data that are pointed by the subobjects.
        /// Does not map 1:1 with mSubobjects
        std::vector<void*> mSubobjectData = {};

        /// @brief The collection state object.
        ComPtr<ID3D12StateObject> mStateObject = nullptr;

        // Friend Device, so that it can access the subobjects.
        friend class Device;
    };

    /// @brief A description of a ray tracing pipeline. This can be used to launch a ray tracing.
    struct RayTracingPipeline
    {
    public: // Methods
        /// @brief Get the pipeline state object.
        /// @return The pipeline state object.
        ComPtr<ID3D12StateObject> GetPipelineStateObject() const { return mPipelineStateObject; }

        PipelineDesc GetDesc() const { return mPipelineStateDesc; }

    public: // Members
        /// @brief The pipeline state object.
        ComPtr<ID3D12StateObject> mPipelineStateObject = nullptr;

        /// @brief The pipeline state object description.
        PipelineDesc mPipelineStateDesc = {};

        // Friend Device, so that it can modify the pipeline state object.
        friend class Device;
    };
} // namespace DXR
