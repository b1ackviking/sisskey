#pragma once

#include <memory>

namespace sisskey
{
	class Window;
	class GraphicsDevice
	{
	public:
		enum class API
		{
			Vulkan,
			DX12
		};
		enum class PresentMode
		{
			Windowed,
			Borderless,
			Fullscreen
		};
	protected:
		GraphicsDevice() = default;
	public:
		virtual ~GraphicsDevice() = default;
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		[[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(std::shared_ptr<Window> window, PresentMode mode = PresentMode::Windowed, API api = API::Vulkan);

		virtual void Render() = 0;
	};
}