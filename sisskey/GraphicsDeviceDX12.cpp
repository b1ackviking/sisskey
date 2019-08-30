#include "GraphicsDeviceDX12.h"

#include "d3dx12.h"
#include <DirectXColors.h>

#include <stdexcept>
#include <vector>
#include <string>
#include <cassert>

namespace sisskey
{
	GraphicsDeviceDX12::GraphicsDeviceDX12(HWND hWnd)
	{
		UINT FactoryCreateFlags{};
#ifndef NDEBUG
		{
			FactoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;
			Microsoft::WRL::ComPtr<ID3D12Debug3> Debug;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
			Debug->EnableDebugLayer();
		}
#endif // !NDEBUG

		// Create DXGI Factory
		{
			Microsoft::WRL::ComPtr<IDXGIFactory2> f2;
			ThrowIfFailed(CreateDXGIFactory2(FactoryCreateFlags, IID_PPV_ARGS(&f2)));
			ThrowIfFailed(f2.As(&m_pFactory));
		}

		// TODO: store this information
		// Enum adapters, outputs and present modes
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> a;
			std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>> adapters;
			for (UINT i{}; m_pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&a)) == S_OK; ++i)
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
				ThrowIfFailed(a.As(&adapter));

				DXGI_ADAPTER_DESC3 desc;
				ThrowIfFailed(adapter->GetDesc3(&desc));

				std::wstring text = L"***Adapter: ";
				text += desc.Description;
				text += L" Memory: ";
				text += std::to_wstring(desc.DedicatedVideoMemory / 1024 / 1024);
				text += L"\n";
				OutputDebugStringW(text.c_str());

				adapters.push_back(adapter);
			}

			std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>> outputs;
			for (const auto& adapter : adapters)
			{
				Microsoft::WRL::ComPtr<IDXGIOutput> o;
				for (UINT i{}; adapter->EnumOutputs(i, &o) == S_OK; ++i)
				{
					Microsoft::WRL::ComPtr<IDXGIOutput6> output;
					ThrowIfFailed(o.As(&output));

					DXGI_OUTPUT_DESC1 desc;
					ThrowIfFailed(output->GetDesc1(&desc));

					std::wstring text = L"***Output: ";
					text += desc.DeviceName;
					text += L"\n";
					OutputDebugStringW(text.c_str());

					outputs.push_back(output);
				}
			}

			for (const auto& output : outputs)
			{
				UINT flags{ DXGI_ENUM_MODES_INTERLACED };
				UINT count{};

				ThrowIfFailed(output->GetDisplayModeList1(m_BackBufferFormat, flags, &count, nullptr));
				std::vector<DXGI_MODE_DESC1> modes(count);
				ThrowIfFailed(output->GetDisplayModeList1(m_BackBufferFormat, flags, &count, modes.data()));

				for (const auto& mode : modes)
				{
					std::wstring text = L"Width = " +
										std::to_wstring(mode.Width) +
										L" " +
										L"Height = " +
										std::to_wstring(mode.Height) +
										L" " +
										L"Refresh = " +
										std::to_wstring(mode.RefreshRate.Numerator) +
										L"/" +
										std::to_wstring(mode.RefreshRate.Denominator) +
										L"\n";
					OutputDebugStringW(text.c_str());
				}
			}
		}
		
		// TODO: select adapter??
		// Create Device
		{
			Microsoft::WRL::ComPtr<ID3D12Device> dev;
			HRESULT hr = D3D12CreateDevice(nullptr, // default adapter
										   D3D_FEATURE_LEVEL_12_0,
										   IID_PPV_ARGS(&dev));
			// Fallback to WARP device.
			if (FAILED(hr))
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
				ThrowIfFailed(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
				ThrowIfFailed(D3D12CreateDevice(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dev)));
			}
			ThrowIfFailed(dev.As(&m_pDevice));
		}

		// Create Fence
		{
			Microsoft::WRL::ComPtr<ID3D12Fence> f;
			ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&f)));
			ThrowIfFailed(f.As(&m_pFence));
		}

		// Store increments
		m_RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Check 4xMSAA support
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = m_BackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		ThrowIfFailed(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
		m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
		
		// Create DirectCommandQueue, CommandAllocator and GraphicsCommandList
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
		ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pDirectCmdListAlloc)));
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cl;
			ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pDirectCmdListAlloc.Get(), // Associated command allocator
													   nullptr, // Initial PipelineStateObject
													   IID_PPV_ARGS(&cl)));
			ThrowIfFailed(cl.As(&m_pCommandList));
		}

		// TODO: separate method
		// Release the previous swapchain we will be recreating.
		m_pSwapChain.Reset();
		{
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
			DXGI_SWAP_CHAIN_DESC1 sd;
			sd.Width = 0;
			sd.Height = 0;
			// Note: the only supported formats with DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL are:
			// DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM
			sd.Format = m_BackBufferFormat;
			sd.Stereo = FALSE;
			// Multisampling is unsupported with DXGI_SWAP_EFFECT_FLIP_*
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount = m_SwapChainBufferCount; // 2-16
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_scaling
			sd.Scaling = DXGI_SCALING_STRETCH;
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // TODO: discard?
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_alpha_mode
			sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // TODO: Check tearing support

			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_fullscreen_desc
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd;
			// TODO: Get refresh rate from present modes
			fsd.RefreshRate.Numerator = 60;
			fsd.RefreshRate.Denominator = 1;
			fsd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			// https://docs.microsoft.com/ru-ru/previous-versions/windows/desktop/legacy/bb173066(v=vs.85)
			fsd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			fsd.Windowed = TRUE; // TODO: ??

			// https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforhwnd
			Microsoft::WRL::ComPtr<IDXGISwapChain1> sc;
			ThrowIfFailed(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &sd, &fsd, nullptr /* TODO: restrict to output?? */, &sc));
			ThrowIfFailed(sc.As(&m_pSwapChain));

			// Get width and height
			ThrowIfFailed(m_pSwapChain->GetDesc1(&sd));
			m_Width = sd.Width;
			m_Height = sd.Height;
		}

		// Create descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = m_SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle{ m_pRtvHeap->GetCPUDescriptorHandleForHeapStart() };
		for (int i{}; i < m_SwapChainBufferCount; ++i)
		{
			// Get the ith buffer in the swap chain.
			ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
			// Create an RTV to it.
			m_pDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			// Next entry in heap.
			rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
		}

		// TODO: offscreen rendering
		// TODO: final step does not need depth/stencil
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect
		// To use multisampling with DXGI_SWAP_EFFECT_SEQUENTIAL or DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		// you must perform the multisampling in a separate render target.
		// For example, create a multisampled texture by calling ID3D11Device::CreateTexture2D
		// with a filled D3D11_TEXTURE2D_DESC structure (BindFlags member set to D3D11_BIND_RENDER_TARGET and SampleDesc member with multisampling parameters).
		// Next call ID3D11Device::CreateRenderTargetView to create a render-target view for the texture, and render your scene into the texture.
		// Finally call ID3D11DeviceContext::ResolveSubresource to resolve the multisampled texture into your non-multisampled swap chain.

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = m_DepthStencilFormat;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		
		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_DepthStencilFormat;
		// TODO: Reverse Z projection
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> dsb;
			ThrowIfFailed(m_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&dsb)));
			ThrowIfFailed(dsb.As(&m_pDepthStencilBuffer));
		}
		// Create descriptor to mip level 0 of entire resource using the
		// format of the resource.
		m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_DepthStencilView());
		// Transition the resource from its initial state to be used as a depth buffer.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		// Set ViewPort and Scissors
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;
		m_ViewPort.Width = static_cast<float>(m_Width);
		m_ViewPort.Height = static_cast<float>(m_Height);
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.MaxDepth = 1.0f;
		m_pCommandList->RSSetViewports(1, &m_ViewPort);

		m_ScissorRect = { 0, 0, m_Width / 2, m_Height / 2 };
		m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

		// Commit initialization commands
		ThrowIfFailed(m_pCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
		m_pCommandQueue->ExecuteCommandLists(1, cmdsLists);
		m_FlushCommandQueue();
	}

	void GraphicsDeviceDX12::Render()
	{
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished
		// execution on the GPU.
		ThrowIfFailed(m_pDirectCmdListAlloc->Reset());
		// A command list can be reset after it has been added to the 
		// command queue via ExecuteCommandList. Reusing the command list reuses memory.
		ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));
		// Indicate a state transition on the resource usage.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		// Set the viewport and scissor rect. This needs to be reset
		// whenever the command list is reset.
		m_pCommandList->RSSetViewports(1, &m_ViewPort);
		m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);
		// Clear the back buffer and depth buffer.
		m_pCommandList->ClearRenderTargetView(m_CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
		m_pCommandList->ClearDepthStencilView(m_DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		// Specify the buffers we are going to render to.
		m_pCommandList->OMSetRenderTargets(1, &m_CurrentBackBufferView(), true, &m_DepthStencilView());
		// Indicate a state transition on the resource usage.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		
		// Done recording commands.
		ThrowIfFailed(m_pCommandList->Close());
		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
		m_pCommandQueue->ExecuteCommandLists(1, cmdsLists);
		// swap the back and front buffers
		ThrowIfFailed(m_pSwapChain->Present(0, 0));
		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % m_SwapChainBufferCount;
		// Wait until frame commands are complete. This waiting is
		// inefficient and is done for simplicity. Later we will show how to
		// organize our rendering code so we do not have to wait per frame.
		m_FlushCommandQueue();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDeviceDX12::m_CurrentBackBufferView() const
	{
		// CD3DX12 constructor to offset to the RTV of the current back buffer.
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), // handle start
											 m_CurrBackBuffer, // index to offset
											 m_RtvDescriptorSize); // byte size of descriptor
	}

	void GraphicsDeviceDX12::m_FlushCommandQueue()
	{
		// Advance the fence value to mark commands up to this fence point.
		m_CurrentFence++;
		// Add an instruction to the command queue to set a new fence point.
		// Because we are on the GPU timeline, the new fence point won’t be
		// set until the GPU finishes processing all the commands prior to
		// this Signal().
		ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), m_CurrentFence));
		// Wait until the GPU has completed commands up to this fence point.
		if (m_pFence->GetCompletedValue() < m_CurrentFence)
		{
			if (HANDLE eventHandle = CreateEventExW(nullptr, false, false, EVENT_ALL_ACCESS); eventHandle)
			{
				// Fire event when GPU hits current fence. 
				ThrowIfFailed(m_pFence->SetEventOnCompletion(m_CurrentFence, eventHandle));
				// Wait until the GPU hits current fence event is fired.
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
			else
				throw std::runtime_error{ u8"Failed to create EventHandle for DX12 Fence" };
		}
	}
}