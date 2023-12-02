#include "DXRay/DXRay.h"

namespace DXR
{
    std::tuple<ComPtr<IDXRDevice>, ComPtr<IDXRAdapter>, ComPtr<IDXGIFactory7>> CreateSimpleDevice(
        bool debug, D3D12_MESSAGE_SEVERITY breakSeverity)
    {
        ComPtr<IDXGIFactory7> factory;
        ComPtr<IDXRAdapter> adapter;
        ComPtr<IDXRDevice> device;

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
        DXR_THROW_FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device)));

#ifndef NDEBUG // Validation break on severity, only on debug builds.
        if (breakSeverity != UINT_MAX)
        {
            ComPtr<ID3D12InfoQueue> infoQueue;
            if (SUCCEEDED(device.As(&infoQueue)))
            {
                switch (breakSeverity)
                {
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_MESSAGE, TRUE);
                }
                case D3D12_MESSAGE_SEVERITY_INFO:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, TRUE);
                }
                case D3D12_MESSAGE_SEVERITY_WARNING:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
                }
                case D3D12_MESSAGE_SEVERITY_ERROR:
                {

                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                }
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                }
                }

                infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

                D3D12_MESSAGE_SEVERITY severities[] = {breakSeverity};

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
