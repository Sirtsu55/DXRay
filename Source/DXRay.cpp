#include "DXRay/DXRay.h"

namespace DXR
{
    std::tuple<ComPtr<IDXRDevice>, ComPtr<IDXRAdapter>, ComPtr<IDXGIFactory7>> CreateSimpleDevice(
        bool debug, D3D12_MESSAGE_SEVERITY breakSeverity)
    {
        ComPtr<IDXGIFactory7> factory = nullptr;
        ComPtr<IDXRAdapter> adapter = nullptr;
        ComPtr<IDXRDevice> device = nullptr;

        UINT dxgiFactoryFlags = 0;

        if (debug)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }

        DXR_THROW_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        // Choose the adapter based on the GPU preference.
        DXR_THROW_FAILED(
            factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));

        // Feature level 12.2 is required for raytracing
        DXR_THROW_FAILED_MSG(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)),
                             "No Suitable GPU found for Ray Tracing");

#ifndef NDEBUG // Validation break on severity, only on debug builds.
        if (debug && (static_cast<D3D12_MESSAGE_SEVERITY>(breakSeverity) != UINT_MAX))
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            if (SUCCEEDED(device.As(&infoQueue)))
            {
                switch (breakSeverity)
                {
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, TRUE);
                    [[fallthrough]];
                }
                case D3D12_MESSAGE_SEVERITY_INFO:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);
                    [[fallthrough]];
                }
                case D3D12_MESSAGE_SEVERITY_WARNING:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
                    [[fallthrough]];
                }
                case D3D12_MESSAGE_SEVERITY_ERROR:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                    [[fallthrough]];
                }
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                }
                }

                D3D12_MESSAGE_SEVERITY severities[] = {
                    D3D12_MESSAGE_SEVERITY_INFO,       D3D12_MESSAGE_SEVERITY_WARNING, D3D12_MESSAGE_SEVERITY_ERROR,
                    D3D12_MESSAGE_SEVERITY_CORRUPTION, D3D12_MESSAGE_SEVERITY_MESSAGE,
                };

                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumSeverities = _countof(severities);
                filter.DenyList.pSeverityList = severities;
                filter.DenyList.NumIDs = 0;
                filter.DenyList.pIDList = nullptr;

                DXR_THROW_FAILED(infoQueue->PushStorageFilter(&filter));
            }
        }
#endif

        return std::make_tuple(device, adapter, factory);
    }
} // namespace DXR
