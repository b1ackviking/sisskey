#include "GraphicsDevice.h"

#ifdef _WIN64
#include "GraphicsDeviceDX12.h"
#endif

#include "GraphicsDeviceVulkan.h"

#include <fstream>

namespace sisskey
{
	std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(std::shared_ptr<Window> window, PresentMode mode, API api)
	{
		assert(window);
#ifdef _WIN64
		if (api == API::DX12)
			return std::make_unique<GraphicsDeviceDX12>(window, mode);
#endif
		return std::make_unique<GraphicsDeviceVulkan>(window, mode);
	}

	Graphics::Shader GraphicsDevice::LoadShader(std::filesystem::path path)
	{
		std::ifstream file{ path, std::ios::binary | std::ios::ate };
		if (!file.is_open())
			throw std::runtime_error{ u8"shader file not found" };

		// "usually works"...
		sisskey::Graphics::Shader data(file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(data.data(), data.size());

		return data;
	}
}
