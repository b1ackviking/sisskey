#include "GraphicsDevice.h"

#ifdef _WIN64
#include "GraphicsDeviceDX12.h"
#endif

#include "GraphicsDeviceVulkan.h"

#include <tuple>

namespace sisskey
{
	[[nodiscard]] std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(std::shared_ptr<void> WindowHandle, API api)
	{
		assert(WindowHandle);
#ifdef _WIN64
		if (api == API::DX12)
			return std::make_unique<GraphicsDeviceDX12>(std::get<0>(*std::static_pointer_cast<std::tuple<HWND, HINSTANCE>, void>(WindowHandle)));
#endif
		return std::make_unique<GraphicsDeviceVulkan>();
	}
}
