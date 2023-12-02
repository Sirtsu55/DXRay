#include "DXRay/Swapchain.h"

namespace DXR
{
    SwapChain::SwapChain(const ComPtr<IDXRDevice>& dev, const ComPtr<IDXRFactory>& factory,
                         const ComPtr<ID3D12CommandQueue>& cmdQueue, HWND hwnd, const XMUINT2& size,
                         UINT32 numBackBuffers, DXGI_FORMAT format)
        : mDevice(dev), mTearingSupported(false), mSize(size), mFormat(format)
    {
        BOOL allowTearing = FALSE;
        factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        mTearingSupported = (allowTearing == TRUE);

        mBackBuffers.resize(numBackBuffers);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = static_cast<UINT>(mBackBuffers.size());
        swapChainDesc.Width = mSize.x;
        swapChainDesc.Height = mSize.y;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: Allow for HDR
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Flags = mTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapChain;
        DXR_THROW_FAILED(
            factory->CreateSwapChainForHwnd(cmdQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));

        DXR_THROW_FAILED(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

        DXR_THROW_FAILED(swapChain.As(&mSwapChain));

        mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        // Create RTV descriptor heap

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = static_cast<UINT>(mBackBuffers.size());
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        DXR_THROW_FAILED(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRTVHeap)));

        // Get RTV descriptor size
        mRTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        UpdateRenderTargetViews();
    }

    SwapChain::~SwapChain()
    {
    }

    void SwapChain::Present(UINT32 syncInterval)
    {
        UINT presentFlags = mTearingSupported && syncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;
        DXR_THROW_FAILED(mSwapChain->Present(syncInterval, presentFlags));
        mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
    }

    void SwapChain::ResizeBuffers(const XMUINT2& size)
    {
        // Release the resources holding references to the swap chain's buffers.
        for (UINT i = 0; i < mBackBuffers.size(); i++) { mBackBuffers[i].Reset(); }

        // Resize the swap chain to the desired dimensions.
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        DXR_THROW_FAILED(mSwapChain->GetDesc1(&desc));
        DXR_THROW_FAILED(
            mSwapChain->ResizeBuffers(static_cast<UINT>(mBackBuffers.size()), size.x, size.y, desc.Format, desc.Flags));

        // Reset the frame index to the current back buffer index.
        mBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

        // Update the render target views for the new size of the swap chain.
        UpdateRenderTargetViews();
    }

    void SwapChain::UpdateRenderTargetViews()
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRTVHeap->GetCPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < mBackBuffers.size(); i++)
        {
            DXR_THROW_FAILED(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i])));
            mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, mRTVDescriptorSize);
        }
    }
} // namespace DXR
