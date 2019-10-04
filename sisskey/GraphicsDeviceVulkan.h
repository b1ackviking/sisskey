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
#endif // !NDEBUG
#include <vk_mem_alloc.h>

#include <numeric>

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
		vk::UniqueCommandPool m_pool;
		std::vector<vk::UniqueCommandBuffer> m_cmd;
		vk::UniqueFence m_fence;
		vk::UniqueSemaphore m_sem;

		struct QueueFamilyIndices
		{
			std::uint32_t GraphicsFamily{ std::numeric_limits<std::uint32_t>::max() };
			std::uint32_t PresentFamily{ std::numeric_limits<std::uint32_t>::max() };
			std::uint32_t CopyFamily{ std::numeric_limits<std::uint32_t>::max() };

			constexpr bool Filled(void) const
			{
				constexpr std::uint32_t max = std::numeric_limits<std::uint32_t>::max();
				return GraphicsFamily != max && PresentFamily != max && CopyFamily != max;
			}
		};
		QueueFamilyIndices m_QueueFamilyIndices{};

		QueueFamilyIndices m_GetQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
		bool m_CheckPhysicalDeviceExtensionSupport(vk::PhysicalDevice device);
		static constexpr std::array m_PhysicalDeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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

	public:
		GraphicsDeviceVulkan(std::shared_ptr<Window> window, PresentMode mode);
		~GraphicsDeviceVulkan();

		void Render() override;
	};
}