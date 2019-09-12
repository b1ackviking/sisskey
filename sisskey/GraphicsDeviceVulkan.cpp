#include "GraphicsDeviceVulkan.h"

namespace sisskey
{
	GraphicsDeviceVulkan::GraphicsDeviceVulkan(std::shared_ptr<Window> window, PresentMode mode)
		: GraphicsDevice(window, mode)
	{}

	GraphicsDeviceVulkan::~GraphicsDeviceVulkan() {}
}