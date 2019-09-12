#include "GraphicsDevice.h"

#ifdef _WIN64
#include "GraphicsDeviceDX12.h"
#endif

#include "GraphicsDeviceVulkan.h"

namespace sisskey
{
	[[nodiscard]] std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(std::shared_ptr<Window> window, PresentMode mode, API api)
	{
		assert(window);
#ifdef _WIN64
		if (api == API::DX12)
			return std::make_unique<GraphicsDeviceDX12>(window, mode);
#endif
		return std::make_unique<GraphicsDeviceVulkan>(window, mode);
	}
}
