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

		[[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(API api = API::Vulkan);


	};
}