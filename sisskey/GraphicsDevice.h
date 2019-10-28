#pragma once

#include "GraphicsDescriptors.h"

#include <memory>
#include <filesystem>

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
		// bool m_Fullscreen{ false }; // TODO: unused

	public:
		virtual ~GraphicsDevice() = default;
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;

		[[nodiscard]] static std::unique_ptr<GraphicsDevice> Create(std::shared_ptr<Window> window, PresentMode mode = PresentMode::Windowed, API api = API::Vulkan);

		virtual Graphics::handle CreateGraphicsPipeline(Graphics::GraphicsPipelineDesc& desc) = 0;
		virtual void DestroyGraphicsPipeline(Graphics::handle pipeline) = 0;

		virtual Graphics::buffer CreateBuffer(Graphics::GPUBufferDesc& desc, std::optional<Graphics::SubresourceData> initData) = 0;
		virtual void DestroyBuffer(Graphics::buffer buffer) = 0;

		// TODO:
		// virtual void BindStencilRef(std::uint32_t value, cmdlist index) = 0;

		// These are test methods
		virtual void Render() = 0;
		virtual void Render(Graphics::handle pipeline, Graphics::buffer vertexBuffer) = 0;

		[[nodiscard]] static constexpr Graphics::FORMAT GetBackBufferFormat() noexcept { return m_BackBufferFormat; }
		[[nodiscard]] static constexpr int GetBackBufferCount() noexcept { return m_BackBufferCount; }
		[[nodiscard]] static constexpr Graphics::FORMAT GetDepthStencilFormat() noexcept { return m_DepthStencilFormat; }

		// TODO: API
		[[nodiscard]] static Graphics::Shader LoadShader(std::filesystem::path path);
	};
}