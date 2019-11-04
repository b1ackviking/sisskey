#pragma once
#include "GraphicsDevice.h"

#ifdef NDEBUG
#define D3D12_IGNORE_SDK_LAYERS
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "D3D12MemAlloc.h"

#include <vector>
#include <mutex>

namespace sisskey
{
	// https://github.com/Microsoft/DirectXTK/wiki/ThrowIfFailed
	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	private:
		HRESULT result;

	public:
		com_exception(HRESULT hr) : result(hr) {}

		virtual const char* what() const override
		{
			constexpr auto bufferSize{ 64 };
			static char buffer[bufferSize] = {};
			// TODO: use {fmt} or C++20 std::format
			sprintf_s(buffer, bufferSize, "Failure with HRESULT of %08X",
					  static_cast<unsigned int>(result));
			return buffer;
		}
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw com_exception(hr);
	}

	class GraphicsDeviceDX12 final : public GraphicsDevice
	{
		// TODO: Organize this section
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory7> m_pFactory;
		Microsoft::WRL::ComPtr<ID3D12Device6> m_pDevice;

		struct ReleaseDeleter { template<typename T> void operator()(T* t) const { t->Release(); } };
		using UniqueD3D12MemoryAllocation = std::unique_ptr<D3D12MA::Allocation, ReleaseDeleter>;
		using UniqueD3D12MemoryAllocator = std::unique_ptr<D3D12MA::Allocator, ReleaseDeleter>;

		UniqueD3D12MemoryAllocator m_d3dma;

		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pFence;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> m_pCommandList;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCopyQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCopyAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> m_pCopyCommandList;
		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pCopyFence;
		HANDLE m_CopyFenceEvent{ INVALID_HANDLE_VALUE };
		UINT64 m_CopyFenceValue;
		std::mutex m_CopyMutex;
		std::vector<Graphics::buffer> m_copyBuffers;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[m_BackBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_pDepthStencilBuffer;
		UniqueD3D12MemoryAllocation m_DSAlloc;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_grs;
		
		D3D12_RECT m_ScissorRect;
		D3D12_VIEWPORT m_ViewPort;

		int m_RtvDescriptorSize;
		int m_DsvDescriptorSize;
		int m_CbvSrvDescriptorSize;
		UINT m_4xMsaaQuality;

		UINT64 m_CurrentFence{};
		void m_FlushCommandQueue();

		[[nodiscard]] ID3D12Resource* m_CurrentBackBuffer() const { return mSwapChainBuffer[m_pSwapChain->GetCurrentBackBufferIndex()].Get(); }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE m_CurrentBackBufferView() const;
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE m_DepthStencilView() const { return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart(); }

		std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>> m_EnumerateAdapters();
		std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>> m_EnumerateAdapterOutputs(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);
		std::vector<DXGI_MODE_DESC1> m_EnumerateDisplayModes(Microsoft::WRL::ComPtr<IDXGIOutput6> output);

		void m_CreateSwapChain();
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIOutput6> m_GetOutputFromWindow(HWND hWnd);
		[[nodiscard]] Microsoft::WRL::ComPtr<IDXGIAdapter4> m_GetAdapter();

	public:
		GraphicsDeviceDX12(std::shared_ptr<Window> window, PresentMode mode);
		~GraphicsDeviceDX12()
		{
			BOOL fullscreen;
			m_pSwapChain->GetFullscreenState(&fullscreen, nullptr);
			if (fullscreen)
				m_pSwapChain->SetFullscreenState(FALSE, nullptr);
			m_FlushCommandQueue();
			if(m_CopyFenceEvent != INVALID_HANDLE_VALUE)
				CloseHandle(m_CopyFenceEvent);
		}

		void Begin(/*clear value*/) final;
		void End() final;

		void BindPipeline(Graphics::PipelineHandle pipeline) final;
		void BindVertexBuffers(std::uint32_t start, const std::vector<Graphics::buffer>& buffers, const std::vector<std::uint64_t>& offsets, const std::vector<std::uint32_t>& strides) final;
		void BindViewports(const std::vector<Graphics::Viewport>& viewports) final;
		void BindScissorRects(const std::vector<Graphics::Rect>& scissors) final;
		void BindIndexBuffer(const Graphics::buffer& indexBuffer, std::uint64_t offsest, Graphics::INDEXBUFFER_FORMAT format) final;
		void Draw(std::uint32_t count, std::uint32_t start) final;
		void DrawIndexed(std::uint32_t count, std::uint32_t startVertex, std::uint32_t startIndex) final;

		// TODO:
		Graphics::PipelineHandle CreateGraphicsPipeline(Graphics::GraphicsPipelineDesc& desc) final;
		void DestroyGraphicsPipeline(Graphics::PipelineHandle pipeline) final;
		Graphics::buffer CreateBuffer(Graphics::GPUBufferDesc& desc, std::optional<Graphics::SubresourceData> initData) final;
		void DestroyBuffer(Graphics::buffer buffer) final;
	};
}