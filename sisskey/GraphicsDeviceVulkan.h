#pragma once
#include "GraphicsDevice.h"

#if defined(_WIN64)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#define VMA_ASSERT(expr) assert(expr)
#else
#define VMA_ASSERT(expr)
#endif // !NDEBUG
#include <vk_mem_alloc.h>

#include <optional>
#include <mutex>

namespace sisskey
{
	class GraphicsDeviceVulkan final : public GraphicsDevice
	{
		// TODO: organize
	private:
		VmaAllocator m_vma{};
		vk::UniqueInstance m_instance;
		vk::UniqueSurfaceKHR m_surface;
		vk::PhysicalDevice m_physDevice;
		vk::UniqueDevice m_device;
		vk::Queue m_PresentQueue;
		vk::Queue m_GraphicsQueue;

		vk::Queue m_CopyQueue;
		vk::UniqueCommandPool m_copyPool;
		vk::UniqueCommandBuffer m_copyCommandBuffer;
		vk::UniqueFence m_copyFence;
		std::vector<Graphics::buffer> m_copyBuffers;
		std::mutex m_copyMutex;
		
		vk::UniqueCommandPool m_pool;
		std::vector<vk::UniqueCommandBuffer> m_cmd;
		vk::UniqueFence m_fence;
		vk::UniqueSemaphore m_imageSemaphore; // TODO: allow interleaved frames
		vk::UniqueSemaphore m_renderSemaphore;

		void m_SetWidthHeight(); // TODO: should this be in a base class ??
		void m_CreateInstance();
		void m_CreateSurface();
		void m_CreatePhysicalDevice();
		void m_CreateLogicalDevice();
		void m_CreateSwapChain();
		void m_CreateAllocator();

		void m_CreateDepthStencilBuffer();

		void m_CreateRenderPass();
		void m_CreateFrameBuffers();

		struct QueueFamilyIndices
		{
			// NOTE: store both (1) index of a family and (2) index of a queue inside that family
			using family_and_index = std::optional<std::pair<std::uint32_t, std::uint32_t>>;
			family_and_index GraphicsQueue;
			family_and_index PresentQueue;
			family_and_index CopyQueue;

			constexpr bool Filled() const noexcept { return GraphicsQueue.has_value() && PresentQueue.has_value() && CopyQueue.has_value(); }
		};
		QueueFamilyIndices m_QueueFamilyIndices{};

		QueueFamilyIndices m_GetQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		bool m_CheckPhysicalDeviceExtensionSupport(vk::PhysicalDevice device);
		static constexpr std::array m_PhysicalDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
																VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
																VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, // needed for VK_KHR_dedicated_allocation
																// VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME // supported only on Windows :(
																VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
																};

		struct SwapChainSupportDetails
		{
			vk::SurfaceCapabilitiesKHR Capabilities;
			std::vector<vk::SurfaceFormatKHR> Formats;
			std::vector<vk::PresentModeKHR> PresentModes;
		};
		SwapChainSupportDetails m_GetSwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

		vk::SurfaceFormatKHR m_ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& Formats);
		vk::PresentModeKHR m_ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& PresentModes);

		vk::Extent2D m_SwapChainExtent;
		vk::UniqueSwapchainKHR m_swapchain;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat;
		std::vector<vk::UniqueImageView> m_sciv;

		vk::UniqueImage m_dsi;
		VmaAllocation alloc;
		vk::UniqueImageView m_dsv;

		vk::UniqueRenderPass m_rp;

		std::vector<vk::UniqueFramebuffer> m_fb;

		vk::UniquePipelineLayout m_pl;

		std::uint32_t m_currentBackBuffer;

#ifndef NDEBUG
		vk::DispatchLoaderDynamic m_loader;

		using vkUniqueDebugUtilsMessengerEXT = vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;
		vkUniqueDebugUtilsMessengerEXT m_messenger;

		static VKAPI_ATTR VkBool32 VKAPI_CALL m_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData);
#endif // !NDEBUG


	public:
		GraphicsDeviceVulkan(std::shared_ptr<Window> window, PresentMode mode);
		~GraphicsDeviceVulkan();

		void Begin(/*clear value*/) final;
		void End() final;

		void BindPipeline(Graphics::handle pipeline) final;
		void BindVertexBuffers(std::uint32_t start, const std::vector<Graphics::buffer>& buffers, const std::vector<std::uint64_t>& offsets) final;
		void BindViewports(const std::vector<Graphics::Viewport>& viewports) final;
		void BindScissorRects(const std::vector<Graphics::Rect>& scissors) final;
		void Draw(std::uint32_t count, std::uint32_t start) final;

		// These are test methods
		void Render() final;
		void Render(Graphics::handle pipeline, Graphics::buffer vertexBuffer) final;

		Graphics::handle CreateGraphicsPipeline(Graphics::GraphicsPipelineDesc& desc) final;
		void DestroyGraphicsPipeline(Graphics::handle pipeline) final;

		Graphics::buffer CreateBuffer(Graphics::GPUBufferDesc& desc, std::optional<Graphics::SubresourceData> initData) final;
		void DestroyBuffer(Graphics::buffer buffer) final;
	};
}