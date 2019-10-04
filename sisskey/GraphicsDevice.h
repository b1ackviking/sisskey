#pragma once

#include "GraphicsDescriptors.h"

#include <memory>

namespace sisskey
{
	// Forward declaration
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
		GraphicsDevice(std::shared_ptr<Window> window, PresentMode mode)
			: m_pWindow{ window }
			, m_PresentMode{ mode }
		{}
		static constexpr Graphics::FORMAT m_BackBufferFormat{ Graphics::FORMAT::B8G8R8A8_UNORM };
		static constexpr int m_BackBufferCount{ 2 }; // 2-16
		static constexpr Graphics::FORMAT m_DepthStencilFormat{ Graphics::FORMAT::D32_FLOAT_S8X24_UINT };

		std::shared_ptr<Window> m_pWindow;
		int m_Width{};
		int m_Height{};

		PresentMode m_PresentMode{ PresentMode::Windowed };
		bool m_TearingSupport{ false };
		bool m_VSync{ true };

	public:
		virtual ~GraphicsDevice() = default;
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		[[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(std::shared_ptr<Window> window, PresentMode mode = PresentMode::Windowed, API api = API::Vulkan);

		virtual void Render() = 0;

		[[nodiscard]] static constexpr Graphics::FORMAT GetBackBufferFormat() noexcept { return m_BackBufferFormat; }
		[[nodiscard]] static constexpr int GetBackBufferCount() noexcept { return m_BackBufferCount; }
	};
}