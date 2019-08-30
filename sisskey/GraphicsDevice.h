#pragma once

#include <memory>

namespace sisskey
{
	class GraphicsDevice
	{
	public:
		enum class API
		{
			Vulkan,
			DX12
		};
	protected:
		GraphicsDevice() = default;
	public:
		virtual ~GraphicsDevice() = default;
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		[[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(std::shared_ptr<void> WindowHandle, API api = API::Vulkan);

		virtual void Render() = 0;
	};
}