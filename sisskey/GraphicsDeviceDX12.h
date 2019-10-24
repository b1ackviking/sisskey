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

#include <vector>

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
		static constexpr DXGI_FORMAT m_DepthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };

		Microsoft::WRL::ComPtr<IDXGIFactory7> m_pFactory;
		Microsoft::WRL::ComPtr<ID3D12Device6> m_pDevice;
		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pFence;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pDirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> m_pCommandList;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
		Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[m_BackBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_pDepthStencilBuffer;
		
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
		}

		// These are test methods
		void Render() final;
		void Render(Graphics::handle pipeline) final {}

		// TODO:
		Graphics::handle CreateGraphicsPipeline(Graphics::GraphicsPipelineDesc& desc) final { return Graphics::handle{}; }
		void DestroyGraphicsPipeline(Graphics::handle pipeline) final {}
	};
}