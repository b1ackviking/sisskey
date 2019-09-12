#pragma once
#include "GraphicsDevice.h"

#include <vulkan/vulkan.hpp>

namespace sisskey
{
	class GraphicsDeviceVulkan final : public GraphicsDevice
	{
	private:


	public:
		GraphicsDeviceVulkan(std::shared_ptr<Window> window, PresentMode mode);
		~GraphicsDeviceVulkan();

		void Render() override {}
	};
}