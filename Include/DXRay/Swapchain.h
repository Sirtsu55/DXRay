#pragma once

#include "DXRay/Common.h"

namespace DXR
{
    class SwapChain
    {
    public:
        /// @brief Default constructor for the SwapChain class.
        SwapChain() = default;

        /// @brief Initializes a new instance of the SwapChain class.
        SwapChain(const ComPtr<IDXRDevice>& dev, const ComPtr<IDXRFactory>& factory,
                  const ComPtr<ID3D12CommandQueue>& cmdQueue, HWND hwnd, const XMUINT2& size, UINT32 numBackBuffers,
                  DXGI_FORMAT format);

        /// @brief Destroys the SwapChain instance.
        ~SwapChain();

        /// @brief Gets the swap chain.
        const ComPtr<IDXGISwapChain3>& GetSwapChain() const { return mSwapChain; }

        /// @brief Gets the swap chain's back buffer.
        const ComPtr<ID3D12Resource>& GetBackBuffer() const { return mBackBuffers[mBackBufferIndex]; }

        /// @brief Gets the swap chain's current back buffer index.
        UINT GetBackBufferIndex() const { return mBackBufferIndex; }

        /// @brief Gets the RTV descriptor heap.
        const ComPtr<ID3D12DescriptorHeap>& GetRTVHeap() const { return mRTVHeap; }

        /// @brief Gets the RTV descriptor size.
        UINT GetRTVDescriptorSize() const { return mRTVDescriptorSize; }

        /// @brief Presents the swap chain's back buffer to the screen.
        void Present(UINT32 syncInterval);

        /// @brief Resizes the swap chain's buffers.
        void ResizeBuffers(const XMUINT2& size);

    private:
        ComPtr<ID3D12Device> mDevice;
        ComPtr<IDXGISwapChain3> mSwapChain;
        std::vector<ComPtr<ID3D12Resource>> mBackBuffers;
        ComPtr<ID3D12DescriptorHeap> mRTVHeap;
        UINT mBackBufferIndex;
        UINT mRTVDescriptorSize;

        bool mTearingSupported;
        XMUINT2 mSize;

        DXGI_FORMAT mFormat;

        void UpdateRenderTargetViews();
    };
} // namespace DXR
