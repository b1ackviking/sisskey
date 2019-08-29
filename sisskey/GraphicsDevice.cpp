#include "GraphicsDevice.h"

#ifdef _WIN64
#include "GraphicsDeviceDX12.h"
#endif

#include "GraphicsDeviceVulkan.h"

namespace sisskey
{
	[[nodiscard]] std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(API api)
	{
#ifdef _WIN64
		if (api == API::DX12)
			return std::make_unique<GraphicsDeviceDX12>();
#endif
		return std::make_unique<GraphicsDeviceVulkan>();
	}
}
