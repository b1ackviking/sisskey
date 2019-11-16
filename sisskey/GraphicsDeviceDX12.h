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

		struct HandleDeleter { void operator()(HANDLE h) { CloseHandle(h); } };
		using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleDeleter>;

		struct ReleaseDeleter { template<typename T> void operator()(T* t) const { t->Release(); } };
		using UniqueD3D12MemoryAllocation = std::unique_ptr<D3D12MA::Allocation, ReleaseDeleter>;
		using UniqueD3D12MemoryAllocator = std::unique_ptr<D3D12MA::Allocator, ReleaseDeleter>;

		UniqueD3D12MemoryAllocator m_d3dma;

		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pFrameFence;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCopyQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCopyAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> m_pCopyCommandList;
		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pCopyFence;
		UniqueHandle m_CopyFenceEvent;
		UINT64 m_CopyFenceValue;
		std::mutex m_CopyMutex;
		std::vector<Graphics::buffer> m_copyBuffers;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[m_BackBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_pDepthStencilBuffer;
		UniqueD3D12MemoryAllocation m_DSAlloc;

		struct FrameResources
		{
			FrameResources(ID3D12Device6* device)
			{
				ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator)));
				{
					Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cl;
					ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator.Get(), // Associated command allocator
															   nullptr, // Initial PipelineStateObject
															   IID_PPV_ARGS(&cl)));
					ThrowIfFailed(cl.As(&CommandList));
				}

				ThrowIfFailed(CommandList->Close());

				FenceEvent = UniqueHandle{ CreateEventExW(NULL, FALSE, FALSE, EVENT_ALL_ACCESS) };
				FenceValue = 0;
			}

			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> CommandList;
			UniqueHandle FenceEvent;
			UINT64 FenceValue;
		};
		std::vector<FrameResources> m_frames;
		std::uint64_t frameIndex{};

		UINT m_RtvDescriptorSize;
		UINT m_DsvDescriptorSize;
		UINT m_CbvSrvDescriptorSize;
		UINT m_SamplerDescriptorSize;
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
		GraphicsDeviceDX12(std::shared_ptr<Window> window, Graphics::PresentMode mode);
		~GraphicsDeviceDX12()
		{
			BOOL fullscreen;
			m_pSwapChain->GetFullscreenState(&fullscreen, nullptr);
			if (fullscreen)
				m_pSwapChain->SetFullscreenState(FALSE, nullptr);
			m_FlushCommandQueue();
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

		Graphics::DescriptorSetLayout CreateDescriptorSetLayout(const std::vector<Graphics::DescriptorRange>& ranges, Graphics::SHADERSTAGE stage) final;
		void DestroyDescriptorSetLayout(Graphics::DescriptorSetLayout layout) final;
		Graphics::PipelineLayout CreatePipelineLayout(const std::vector<Graphics::DescriptorSetLayout>& descriptorLayouts) final;
		void DestroyPipelineLayout(Graphics::PipelineLayout pl) final;
		void BindPipelineLayout(Graphics::PipelineLayout pl) final;

		Graphics::handle CreateDescriptorHeap(const std::vector<Graphics::DescriptorRange>& ranges, std::uint32_t maxSets) final;
		void DestroyDescriptorHeap(Graphics::handle heap) final;
		void BindDescriptorHeaps(const std::vector<Graphics::handle>& heaps) final;

		void BindConstantBuffer(std::uint32_t range, std::uint32_t index, Graphics::DescriptorSet set, Graphics::buffer cb) final;

		std::vector<Graphics::DescriptorSet> CreateDescriptorSets(Graphics::handle heap, const std::vector<Graphics::DescriptorSetLayout>& layouts) final;
		void BindDescriptorSet(std::uint32_t index, Graphics::DescriptorSet ds, Graphics::PipelineLayout) final;
	};
}