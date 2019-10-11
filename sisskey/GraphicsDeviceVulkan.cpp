#include "GraphicsDeviceVulkan.h"

#include "Window.h"

// #include <DirectXColors.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif // _MSC_VER
#include <spdlog/spdlog.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#include <set>
#include <cmath>

namespace sisskey
{
#ifndef NDEBUG
	VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsDeviceVulkan::m_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
																		VkDebugUtilsMessageTypeFlagsEXT messageType,
																		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
																		void* pUserData)
	{
		auto type = messageType == VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ? "VK_GENERAL" :
				messageType == VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ? "VK_PERFORMANCE" : "VK_VALIDATION";
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			spdlog::info("[{}] {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			spdlog::warn("[{}] {}", type, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			spdlog::error("[{}] {}", type, pCallbackData->pMessage);
			break;
		}
		return VK_FALSE;
	}
#endif // !NDEBUG

	namespace Graphics
	{
		inline vk::ColorComponentFlags VK_ColorWriteMask(COLOR_WRITE_ENABLE value)
		{
			vk::ColorComponentFlags flag{};

			if (value == COLOR_WRITE_ENABLE::ALL)
				return vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
			else
			{
				if ((value & COLOR_WRITE_ENABLE::RED) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= vk::ColorComponentFlagBits::eR;
				if ((value & COLOR_WRITE_ENABLE::GREEN) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= vk::ColorComponentFlagBits::eG;
				if ((value & COLOR_WRITE_ENABLE::BLUE) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= vk::ColorComponentFlagBits::eB;
				if ((value & COLOR_WRITE_ENABLE::ALPHA) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= vk::ColorComponentFlagBits::eA;
			}

			return flag;
		}

		constexpr inline vk::SamplerAddressMode VK_TextureAddressMode(TEXTURE_ADDRESS_MODE value)
		{
			switch (value)
			{
			case TEXTURE_ADDRESS_MODE::WRAP:		return vk::SamplerAddressMode::eRepeat;
			case TEXTURE_ADDRESS_MODE::MIRROR:		return vk::SamplerAddressMode::eMirroredRepeat;
			case TEXTURE_ADDRESS_MODE::CLAMP:		return vk::SamplerAddressMode::eClampToEdge;
			case TEXTURE_ADDRESS_MODE::BORDER:		return vk::SamplerAddressMode::eClampToBorder;
			case TEXTURE_ADDRESS_MODE::MIRROR_ONCE:	return vk::SamplerAddressMode::eMirrorClampToEdge;
			default:								return vk::SamplerAddressMode::eRepeat;
			}
		}

		constexpr inline vk::CompareOp VK_ComparisonFunc(COMPARISON_FUNC value)
		{
			switch (value)
			{
			case COMPARISON_FUNC::NEVER:			return vk::CompareOp::eNever;
			case COMPARISON_FUNC::LESS:				return vk::CompareOp::eLess;
			case COMPARISON_FUNC::EQUAL:			return vk::CompareOp::eEqual;
			case COMPARISON_FUNC::LESS_EQUAL:		return vk::CompareOp::eLessOrEqual;
			case COMPARISON_FUNC::GREATER:			return vk::CompareOp::eGreater;
			case COMPARISON_FUNC::NOT_EQUAL:		return vk::CompareOp::eNotEqual;
			case COMPARISON_FUNC::GREATER_EQUAL:	return vk::CompareOp::eGreaterOrEqual;
			case COMPARISON_FUNC::ALWAYS:			return vk::CompareOp::eAlways;
			default:								return vk::CompareOp::eNever;
			}
		}

		constexpr inline vk::PolygonMode VK_FillMode(FILL_MODE value)
		{
			switch (value)
			{
			case FILL_MODE::WIREFRAME:	return vk::PolygonMode::eLine;
			case FILL_MODE::SOLID:		return vk::PolygonMode::eFill;
			default:					return vk::PolygonMode::eLine;
			}
		}

		constexpr inline vk::CullModeFlags VK_CullMode(CULL_MODE value)
		{
			switch (value)
			{
			case CULL_MODE::NONE:	return vk::CullModeFlagBits::eNone;
			case CULL_MODE::FRONT:	return vk::CullModeFlagBits::eFront;
			case CULL_MODE::BACK:	return vk::CullModeFlagBits::eBack;
			default:				return vk::CullModeFlagBits::eNone;
			}
		}

		constexpr inline vk::StencilOp VK_StencilOp(STENCIL_OP value)
		{
			switch (value)
			{
			case STENCIL_OP::KEEP:		return vk::StencilOp::eKeep;
			case STENCIL_OP::ZERO:		return vk::StencilOp::eZero;
			case STENCIL_OP::REPLACE:	return vk::StencilOp::eReplace;
			case STENCIL_OP::INCR_SAT:	return vk::StencilOp::eIncrementAndClamp; // ??
			case STENCIL_OP::DECR_SAT:	return vk::StencilOp::eDecrementAndClamp; // ??
			case STENCIL_OP::INVERT:	return vk::StencilOp::eInvert;
			case STENCIL_OP::INCR:		return vk::StencilOp::eIncrementAndWrap;
			case STENCIL_OP::DECR:		return vk::StencilOp::eDecrementAndWrap;
			default:					return vk::StencilOp::eKeep;
			}
		}

		constexpr inline vk::BlendFactor VK_Blend(BLEND value)
		{
			switch (value)
			{
			case BLEND::ZERO:				return vk::BlendFactor::eZero;
			case BLEND::ONE:				return vk::BlendFactor::eOne;
			case BLEND::SRC_COLOR:			return vk::BlendFactor::eSrcColor;
			case BLEND::INV_SRC_COLOR:		return vk::BlendFactor::eOneMinusSrcColor;
			case BLEND::SRC_ALPHA:			return vk::BlendFactor::eSrcAlpha;
			case BLEND::INV_SRC_ALPHA:		return vk::BlendFactor::eOneMinusSrcAlpha;
			case BLEND::DEST_ALPHA:			return vk::BlendFactor::eDstAlpha;
			case BLEND::INV_DEST_ALPHA:		return vk::BlendFactor::eOneMinusDstAlpha;
			case BLEND::DEST_COLOR:			return vk::BlendFactor::eDstColor;
			case BLEND::INV_DEST_COLOR:		return vk::BlendFactor::eOneMinusDstColor;
			case BLEND::SRC_ALPHA_SAT:		return vk::BlendFactor::eSrcAlphaSaturate;
			case BLEND::BLEND_FACTOR:		return vk::BlendFactor::eConstantColor;
			case BLEND::INV_BLEND_FACTOR:	return vk::BlendFactor::eOneMinusConstantColor;
			case BLEND::SRC1_COLOR:			return vk::BlendFactor::eSrc1Color;
			case BLEND::INV_SRC1_COLOR:		return vk::BlendFactor::eOneMinusSrc1Color;
			case BLEND::SRC1_ALPHA:			return vk::BlendFactor::eSrc1Alpha;
			case BLEND::INV_SRC1_ALPHA:		return vk::BlendFactor::eOneMinusSrc1Alpha;
			default:						return vk::BlendFactor::eZero;
			}
		}

		constexpr inline vk::BlendOp VK_BlendOp(BLEND_OP value)
		{
			switch (value)
			{
			case BLEND_OP::ADD:				return vk::BlendOp::eAdd;
			case BLEND_OP::SUBTRACT:		return vk::BlendOp::eSubtract;
			case BLEND_OP::REV_SUBTRACT:	return vk::BlendOp::eReverseSubtract;
			case BLEND_OP::MIN:				return vk::BlendOp::eMin;
			case BLEND_OP::MAX:				return vk::BlendOp::eMax;
			default:						return vk::BlendOp::eAdd;
			}
		}

		constexpr inline vk::VertexInputRate VK_InputClassification(INPUT_CLASSIFICATION value)
		{
			switch (value)
			{
			case INPUT_CLASSIFICATION::INPUT_PER_VERTEX_DATA:	return vk::VertexInputRate::eVertex;
			case INPUT_CLASSIFICATION::INPUT_PER_INSTANCE_DATA:	return vk::VertexInputRate::eInstance;
			default:											return vk::VertexInputRate::eVertex;
			}
		}

		constexpr inline vk::Format VK_Format(FORMAT value)
		{
			switch (value)
			{
			case FORMAT::UNKNOWN:				return vk::Format::eUndefined;

			case FORMAT::R32G32B32A32_FLOAT:	return vk::Format::eR32G32B32A32Sfloat;
			case FORMAT::R32G32B32A32_UINT:		return vk::Format::eR32G32B32A32Uint;
			case FORMAT::R32G32B32A32_SINT:		return vk::Format::eR32G32B32A32Sint;

			case FORMAT::R32G32B32_FLOAT:		return vk::Format::eR32G32B32Sfloat;
			case FORMAT::R32G32B32_UINT:		return vk::Format::eR32G32B32Uint;
			case FORMAT::R32G32B32_SINT:		return vk::Format::eR32G32B32Sint;

			case FORMAT::R16G16B16A16_FLOAT:	return vk::Format::eR16G16B16A16Sfloat;
			case FORMAT::R16G16B16A16_UNORM:	return vk::Format::eR16G16B16A16Unorm;
			case FORMAT::R16G16B16A16_UINT:		return vk::Format::eR16G16B16A16Uint;
			case FORMAT::R16G16B16A16_SNORM:	return vk::Format::eR16G16B16A16Snorm;
			case FORMAT::R16G16B16A16_SINT:		return vk::Format::eR16G16B16A16Sint;

			case FORMAT::R32G32_FLOAT:			return vk::Format::eR32G32Sfloat;
			case FORMAT::R32G32_UINT:			return vk::Format::eR32G32Uint;
			case FORMAT::R32G32_SINT:			return vk::Format::eR32G32Sint;
			case FORMAT::R32G8X24_TYPELESS:		return vk::Format::eD32SfloatS8Uint;
			case FORMAT::D32_FLOAT_S8X24_UINT:	return vk::Format::eD32SfloatS8Uint;

			case FORMAT::R10G10B10A2_UNORM:		return vk::Format::eA2B10G10R10UnormPack32;
			case FORMAT::R10G10B10A2_UINT:		return vk::Format::eA2B10G10R10UintPack32;
			case FORMAT::R11G11B10_FLOAT:		return vk::Format::eB10G11R11UfloatPack32;

			case FORMAT::R8G8B8A8_UNORM:		return vk::Format::eR8G8B8A8Unorm;
			case FORMAT::R8G8B8A8_UNORM_SRGB:	return vk::Format::eR8G8B8A8Srgb;
			case FORMAT::R8G8B8A8_UINT:			return vk::Format::eR8G8B8A8Uint;
			case FORMAT::R8G8B8A8_SNORM:		return vk::Format::eR8G8B8A8Snorm;
			case FORMAT::R8G8B8A8_SINT:			return vk::Format::eR8G8B8A8Sint;
			case FORMAT::B8G8R8A8_UNORM:		return vk::Format::eB8G8R8A8Unorm;
			case FORMAT::B8G8R8A8_UNORM_SRGB:	return vk::Format::eB8G8R8A8Srgb;

			case FORMAT::R16G16_FLOAT:			return vk::Format::eR16G16Sfloat;
			case FORMAT::R16G16_UNORM:			return vk::Format::eR16G16Unorm;
			case FORMAT::R16G16_UINT:			return vk::Format::eR16G16Uint;
			case FORMAT::R16G16_SNORM:			return vk::Format::eR16G16Snorm;
			case FORMAT::R16G16_SINT:			return vk::Format::eR16G16Sint;

			case FORMAT::R32_TYPELESS:			return vk::Format::eD32Sfloat;
			case FORMAT::D32_FLOAT:				return vk::Format::eD32Sfloat;
			case FORMAT::R32_FLOAT:				return vk::Format::eR32Sfloat;
			case FORMAT::R32_UINT:				return vk::Format::eR32Uint;
			case FORMAT::R32_SINT:				return vk::Format::eR32Sint;
			case FORMAT::R24G8_TYPELESS:		return vk::Format::eD24UnormS8Uint;
			case FORMAT::D24_UNORM_S8_UINT:		return vk::Format::eD24UnormS8Uint;

			case FORMAT::R8G8_UNORM:			return vk::Format::eR8G8Unorm;
			case FORMAT::R8G8_UINT:				return vk::Format::eR8G8Uint;
			case FORMAT::R8G8_SNORM:			return vk::Format::eR8G8Snorm;
			case FORMAT::R8G8_SINT:				return vk::Format::eR8G8Sint;
			case FORMAT::R16_TYPELESS:			return vk::Format::eD16Unorm;
			case FORMAT::R16_FLOAT:				return vk::Format::eR16Sfloat;
			case FORMAT::D16_UNORM:				return vk::Format::eD16Unorm;
			case FORMAT::R16_UNORM:				return vk::Format::eR16Unorm;
			case FORMAT::R16_UINT:				return vk::Format::eR16Uint;
			case FORMAT::R16_SNORM:				return vk::Format::eR16Snorm;
			case FORMAT::R16_SINT:				return vk::Format::eR16Sint;

			case FORMAT::R8_UNORM:				return vk::Format::eR8Unorm;
			case FORMAT::R8_UINT:				return vk::Format::eR8Uint;
			case FORMAT::R8_SNORM:				return vk::Format::eR8Snorm;
			case FORMAT::R8_SINT:				return vk::Format::eR8Sint;

			case FORMAT::BC1_UNORM:				return vk::Format::eBc1RgbaUnormBlock;
			case FORMAT::BC1_UNORM_SRGB:		return vk::Format::eBc1RgbaSrgbBlock;
			case FORMAT::BC2_UNORM:				return vk::Format::eBc2UnormBlock;
			case FORMAT::BC2_UNORM_SRGB:		return vk::Format::eBc2SrgbBlock;
			case FORMAT::BC3_UNORM:				return vk::Format::eBc3UnormBlock;
			case FORMAT::BC3_UNORM_SRGB:		return vk::Format::eBc3SrgbBlock;
			case FORMAT::BC4_UNORM:				return vk::Format::eBc4UnormBlock;
			case FORMAT::BC4_SNORM:				return vk::Format::eBc4SnormBlock;
			case FORMAT::BC5_UNORM:				return vk::Format::eBc5UnormBlock;
			case FORMAT::BC5_SNORM:				return vk::Format::eBc5SnormBlock;
			case FORMAT::BC6H_UF16:				return vk::Format::eBc6HUfloatBlock;
			case FORMAT::BC6H_SF16:				return vk::Format::eBc6HSfloatBlock;
			case FORMAT::BC7_UNORM:				return vk::Format::eBc7UnormBlock;
			case FORMAT::BC7_UNORM_SRGB:		return vk::Format::eBc7SrgbBlock;
			default:							return vk::Format::eUndefined;
			}
		}

		constexpr inline FORMAT SK_Format(vk::Format value)
		{
			switch (value)
			{
			case vk::Format::eUndefined:				return FORMAT::UNKNOWN;

			case vk::Format::eR32G32B32A32Sfloat:		return FORMAT::R32G32B32A32_FLOAT;
			case vk::Format::eR32G32B32A32Uint:			return FORMAT::R32G32B32A32_UINT;
			case vk::Format::eR32G32B32A32Sint:			return FORMAT::R32G32B32A32_SINT;

			case vk::Format::eR32G32B32Sfloat:			return FORMAT::R32G32B32_FLOAT;
			case vk::Format::eR32G32B32Uint:			return FORMAT::R32G32B32_UINT;
			case vk::Format::eR32G32B32Sint:			return FORMAT::R32G32B32_SINT;

			case vk::Format::eR16G16B16A16Sfloat:		return FORMAT::R16G16B16A16_FLOAT;
			case vk::Format::eR16G16B16A16Unorm:		return FORMAT::R16G16B16A16_UNORM;
			case vk::Format::eR16G16B16A16Uint:			return FORMAT::R16G16B16A16_UINT;
			case vk::Format::eR16G16B16A16Snorm:		return FORMAT::R16G16B16A16_SNORM;
			case vk::Format::eR16G16B16A16Sint:			return FORMAT::R16G16B16A16_SINT;

			case vk::Format::eR32G32Sfloat:				return FORMAT::R32G32_FLOAT;
			case vk::Format::eR32G32Uint:				return FORMAT::R32G32_UINT;
			case vk::Format::eR32G32Sint:				return FORMAT::R32G32_SINT;
			// case vk::Format::eD32SfloatS8Uint:			return FORMAT::R32G8X24_TYPELESS;
			case vk::Format::eD32SfloatS8Uint:		return FORMAT::D32_FLOAT_S8X24_UINT;

			case vk::Format::eA2B10G10R10UnormPack32:	return FORMAT::R10G10B10A2_UNORM;
			case vk::Format::eA2B10G10R10UintPack32:	return FORMAT::R10G10B10A2_UINT;
			case vk::Format::eB10G11R11UfloatPack32:	return FORMAT::R11G11B10_FLOAT;

			case vk::Format::eR8G8B8A8Unorm:			return FORMAT::R8G8B8A8_UNORM;
			case vk::Format::eR8G8B8A8Srgb:				return FORMAT::R8G8B8A8_UNORM_SRGB;
			case vk::Format::eR8G8B8A8Uint:				return FORMAT::R8G8B8A8_UINT;
			case vk::Format::eR8G8B8A8Snorm:			return FORMAT::R8G8B8A8_SNORM;
			case vk::Format::eR8G8B8A8Sint:				return FORMAT::R8G8B8A8_SINT;
			case vk::Format::eB8G8R8A8Unorm:			return FORMAT::B8G8R8A8_UNORM;
			case vk::Format::eB8G8R8A8Srgb:				return FORMAT::B8G8R8A8_UNORM_SRGB;

			case vk::Format::eR16G16Sfloat:				return FORMAT::R16G16_FLOAT;
			case vk::Format::eR16G16Unorm:				return FORMAT::R16G16_UNORM;
			case vk::Format::eR16G16Uint:				return FORMAT::R16G16_UINT;
			case vk::Format::eR16G16Snorm:				return FORMAT::R16G16_SNORM;
			case vk::Format::eR16G16Sint:				return FORMAT::R16G16_SINT;

			// case vk::Format::eD32Sfloat:				return FORMAT::R32_TYPELESS;
			case vk::Format::eD32Sfloat:				return FORMAT::D32_FLOAT;
			case vk::Format::eR32Sfloat:				return FORMAT::R32_FLOAT;
			case vk::Format::eR32Uint:					return FORMAT::R32_UINT;
			case vk::Format::eR32Sint:					return FORMAT::R32_SINT;
			// case vk::Format::eD24UnormS8Uint:		return FORMAT::R24G8_TYPELESS;
			case vk::Format::eD24UnormS8Uint:			return FORMAT::D24_UNORM_S8_UINT;

			case vk::Format::eR8G8Unorm:				return FORMAT::R8G8_UNORM;
			case vk::Format::eR8G8Uint:					return FORMAT::R8G8_UINT;
			case vk::Format::eR8G8Snorm:				return FORMAT::R8G8_SNORM;
			case vk::Format::eR8G8Sint:					return FORMAT::R8G8_SINT;
			// case vk::Format::eD16Unorm:				return FORMAT::R16_TYPELESS;
			case vk::Format::eR16Sfloat:				return FORMAT::R16_FLOAT;
			case vk::Format::eD16Unorm:					return FORMAT::D16_UNORM;
			case vk::Format::eR16Unorm:					return FORMAT::R16_UNORM;
			case vk::Format::eR16Uint:					return FORMAT::R16_UINT;
			case vk::Format::eR16Snorm:					return FORMAT::R16_SNORM;
			case vk::Format::eR16Sint:					return FORMAT::R16_SINT;

			case vk::Format::eR8Unorm:					return FORMAT::R8_UNORM;
			case vk::Format::eR8Uint:					return FORMAT::R8_UINT;
			case vk::Format::eR8Snorm:					return FORMAT::R8_SNORM;
			case vk::Format::eR8Sint:					return FORMAT::R8_SINT;

			case vk::Format::eBc1RgbaUnormBlock:		return FORMAT::BC1_UNORM;
			case vk::Format::eBc1RgbaSrgbBlock:			return FORMAT::BC1_UNORM_SRGB;
			case vk::Format::eBc2UnormBlock:			return FORMAT::BC2_UNORM;
			case vk::Format::eBc2SrgbBlock:				return FORMAT::BC2_UNORM_SRGB;
			case vk::Format::eBc3UnormBlock:			return FORMAT::BC3_UNORM;
			case vk::Format::eBc3SrgbBlock:				return FORMAT::BC3_UNORM_SRGB;
			case vk::Format::eBc4UnormBlock:			return FORMAT::BC4_UNORM;
			case vk::Format::eBc4SnormBlock:			return FORMAT::BC4_SNORM;
			case vk::Format::eBc5UnormBlock:			return FORMAT::BC5_UNORM;
			case vk::Format::eBc5SnormBlock:			return FORMAT::BC5_SNORM;
			case vk::Format::eBc6HUfloatBlock:			return FORMAT::BC6H_UF16;
			case vk::Format::eBc6HSfloatBlock:			return FORMAT::BC6H_SF16;
			case vk::Format::eBc7UnormBlock:			return FORMAT::BC7_UNORM;
			case vk::Format::eBc7SrgbBlock:				return FORMAT::BC7_UNORM_SRGB;
			default:									return FORMAT::UNKNOWN;
			}
		}
	}

	void GraphicsDeviceVulkan::m_CreateInstance()
	{
		vk::ApplicationInfo appInfo{ u8"sisskey game", VK_MAKE_VERSION(1,0,0), u8"sisskey", VK_MAKE_VERSION(1,0,0), VK_API_VERSION_1_1 };
		std::vector<const char*> instanceExtentions;
		std::vector<const char*> instanceLayers;

		instanceExtentions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		instanceExtentions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME); // needed for VK_EXT_full_screen_exclusive device extension
#if defined(_WIN64)
		instanceExtentions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
		instanceExtentions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

#ifndef NDEBUG
		instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
		instanceExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // !NDEBUG

		vk::InstanceCreateInfo instanceInfo{ {}, &appInfo,
											static_cast<std::uint32_t>(instanceLayers.size()), instanceLayers.data(),
											static_cast<std::uint32_t>(instanceExtentions.size()), instanceExtentions.data() };

		m_instance = vk::createInstanceUnique(instanceInfo);

#ifndef NDEBUG
		m_loader.init(m_instance.get());

		auto severityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
		// | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

		auto typeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		m_messenger = m_instance->createDebugUtilsMessengerEXTUnique({ {}, severityFlags, typeFlags, m_DebugCallback }, nullptr, m_loader);
#endif // NDEBUG
	}

	void GraphicsDeviceVulkan::m_CreateSurface()
	{
#if defined(_WIN64)
		auto wnd = std::static_pointer_cast<std::tuple<HWND, HINSTANCE>, void>(m_pWindow->GetNativeHandle());

		vk::Win32SurfaceCreateInfoKHR surfaceInfo{ {}, std::get<HINSTANCE>(*wnd), std::get<HWND>(*wnd) };
		m_surface = m_instance->createWin32SurfaceKHRUnique(surfaceInfo);

#elif defined(__linux__)
		auto wnd = std::static_pointer_cast<std::tuple<xcb_connection_t*, xcb_window_t>, void>(m_pWindow->GetNativeHandle());
		vk::XcbSurfaceCreateInfoKHR xcbInfo{ {}, std::get<xcb_connection_t*>(*wnd), std::get<xcb_window_t>(*wnd) };
		m_surface = m_instance->createXcbSurfaceKHRUnique(xcbInfo);
#endif
	}

	void GraphicsDeviceVulkan::m_CreatePhysicalDevice()
	{
		auto pdevs = m_instance->enumeratePhysicalDevices();

		auto it = std::find_if(pdevs.begin(), pdevs.end(), [this](const auto& device)
								{
									m_QueueFamilyIndices = m_GetQueueFamilies(device, m_surface.get());
									bool e = m_CheckPhysicalDeviceExtensionSupport(device);
									bool s = false;
									if (e)
									{
										SwapChainSupportDetails scd = m_GetSwapChainSupport(device, m_surface.get());
										s = !scd.Formats.empty() && !scd.PresentModes.empty();
									}

									return m_QueueFamilyIndices.Filled() && e && s;
								});

		if (it == pdevs.end())
			throw std::runtime_error{ u8"No physical device" };

		m_physDevice = *it;
	}

	void GraphicsDeviceVulkan::m_CreateLogicalDevice()
	{
		std::multiset queueFamilies{ m_QueueFamilyIndices.GraphicsQueue->first, m_QueueFamilyIndices.CopyQueue->first, m_QueueFamilyIndices.PresentQueue->first };
		std::set uniqueFamilies(queueFamilies.begin(), queueFamilies.end()); // NOTE: be sure to invoke ctor with 2 iterators insted of creating set of iterators
		const float queuePriorities[]{ 1.f, 1.f, 1.f }; // NOTE: we need at most 3 value in order to create 3 queues

		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		for (auto familyIndex : uniqueFamilies)
		{
			vk::DeviceQueueCreateInfo queueInfo{ {}, familyIndex, static_cast<std::uint32_t>(queueFamilies.count(familyIndex)), queuePriorities };
			queueInfos.push_back(queueInfo);
		}

		auto pdevFeatures = m_physDevice.getFeatures();

		vk::DeviceCreateInfo deviceInfo{ {}, static_cast<std::uint32_t>(queueInfos.size()), queueInfos.data(),
										0, nullptr,
										static_cast<std::uint32_t>(m_PhysicalDeviceExtensions.size()), m_PhysicalDeviceExtensions.data(),
										&pdevFeatures };

		m_device = m_physDevice.createDeviceUnique(deviceInfo);

		m_GraphicsQueue = m_device->getQueue(m_QueueFamilyIndices.GraphicsQueue->first, m_QueueFamilyIndices.GraphicsQueue->second);
		m_CopyQueue = m_device->getQueue(m_QueueFamilyIndices.CopyQueue->first, m_QueueFamilyIndices.CopyQueue->second);
		m_PresentQueue = m_device->getQueue(m_QueueFamilyIndices.PresentQueue->first, m_QueueFamilyIndices.PresentQueue->second);
	}

	void GraphicsDeviceVulkan::m_CreateSwapChain()
	{
		SwapChainSupportDetails SwapChainDetails = m_GetSwapChainSupport(m_physDevice, m_surface.get());
		vk::SurfaceFormatKHR SurfaceFormat = m_ChooseSwapSurfaceFormat(SwapChainDetails.Formats);
		vk::PresentModeKHR PresentMode = m_ChooseSwapPresentMode(SwapChainDetails.PresentModes);

		// TODO: size ? fullscreen ? mode
		auto [w, h] = m_pWindow->GetSize();
		m_Width = w;
		m_Height = h;

		m_SwapChainExtent.width = std::max(SwapChainDetails.Capabilities.minImageExtent.width, std::min(SwapChainDetails.Capabilities.maxImageExtent.width, static_cast<uint32_t>(m_Width)));
		m_SwapChainExtent.height = std::max(SwapChainDetails.Capabilities.minImageExtent.height, std::min(SwapChainDetails.Capabilities.maxImageExtent.height, static_cast<uint32_t>(m_Height)));

		vk::SwapchainCreateInfoKHR swapchainInfo{ {}, m_surface.get(),
												// for mailbox we need at least 3 swapchain images (driver creates 5)
												(PresentMode == vk::PresentModeKHR::eMailbox && m_BackBufferCount == 2)
												? static_cast<std::uint32_t>(m_BackBufferCount + 1)
												: static_cast<std::uint32_t>(m_BackBufferCount),
												SurfaceFormat.format, SurfaceFormat.colorSpace, m_SwapChainExtent, 1u, vk::ImageUsageFlagBits::eColorAttachment,
												vk::SharingMode::eExclusive, VK_QUEUE_FAMILY_IGNORED, nullptr, SwapChainDetails.Capabilities.currentTransform,
												vk::CompositeAlphaFlagBitsKHR::eOpaque, PresentMode, VK_TRUE, m_swapchain.get() };
		/*
		// VK_EXT_full_screen_exclusive supported only on Windows :(
		// saved this code as a reminder that extension exists
		VkSurfaceFullScreenExclusiveInfoEXT fullscreenInfo{};
		fullscreenInfo.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
		fullscreenInfo.fullScreenExclusive = VkFullScreenExclusiveEXT::VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;
#ifdef _WIN64 // TODO: NEEDED ??
		VkSurfaceFullScreenExclusiveWin32InfoEXT win32fullscreenInfo{};
		win32fullscreenInfo.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
		win32fullscreenInfo.hmonitor = MonitorFromWindow(std::get<HWND>(*std::static_pointer_cast<std::tuple<HWND, HINSTANCE>, void>(m_pWindow->GetNativeHandle())), MONITOR_DEFAULTTOPRIMARY);
		fullscreenInfo.pNext = &win32fullscreenInfo;
#endif
		swapchainInfo.pNext = &fullscreenInfo;
		*/
		std::array queueFamilyIndices{ m_QueueFamilyIndices.GraphicsQueue->first, m_QueueFamilyIndices.PresentQueue->first };
		if (queueFamilyIndices[0] != queueFamilyIndices[1])
		{
			swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}

		m_swapchain = m_device->createSwapchainKHRUnique(swapchainInfo);

		m_SwapChainImages = m_device->getSwapchainImagesKHR(m_swapchain.get());
		assert(m_SwapChainImages.size() >= m_BackBufferCount); // Vulkan creates AT LEAST minImageCount images

		m_SwapChainImageFormat = SurfaceFormat.format;

		// explicitly delete old image views in case swapchain is recreated
		m_sciv.clear();

		for (const auto& img : m_SwapChainImages)
		{
			vk::ImageViewCreateInfo imageInfo{ {}, img, vk::ImageViewType::e2D, m_SwapChainImageFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
			m_sciv.push_back(m_device->createImageViewUnique(imageInfo));
		}

		// TODO: recreate dependent objects ??
	}

	void GraphicsDeviceVulkan::m_CreateAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo{ VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT, m_physDevice, m_device.get(), };
		vmaCreateAllocator(&allocatorInfo, &m_vma);
	}

	void GraphicsDeviceVulkan::m_CreateDepthStencilBuffer()
	{
		vk::ImageCreateInfo dsInfo{ {}, vk::ImageType::e2D, VK_Format(m_DepthStencilFormat), { m_SwapChainExtent.width, m_SwapChainExtent.height, 1 }, 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment };
		m_dsi = m_device->createImageUnique(dsInfo);

		// TODO: vma cpp?
		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vmaAllocateMemoryForImage(m_vma, m_dsi.get(), &allocInfo, &alloc, nullptr);

		vmaBindImageMemory(m_vma, alloc, m_dsi.get());

		vk::ImageViewCreateInfo dsivInfo{ {}, m_dsi.get(), vk::ImageViewType::e2D, VK_Format(m_DepthStencilFormat), {}, { vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 } };
		m_dsv = m_device->createImageViewUnique(dsivInfo);
	}

	void GraphicsDeviceVulkan::m_CreateRenderPass()
	{
		// Destroy previously created renderpass
		m_rp.reset();

		std::array<vk::AttachmentDescription, 2> att{};
		att[0].format = m_SwapChainImageFormat;
		att[0].loadOp = vk::AttachmentLoadOp::eClear;
		att[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;

		att[1].format = VK_Format(m_DepthStencilFormat);
		att[1].loadOp = vk::AttachmentLoadOp::eClear;
		att[1].storeOp = vk::AttachmentStoreOp::eDontCare;
		att[1].stencilLoadOp = vk::AttachmentLoadOp::eClear;
		att[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		att[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		std::array<vk::AttachmentReference, 1> colorRef{};
		colorRef[0].attachment = 0;
		colorRef[0].layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthRef{ 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };

		std::array<vk::SubpassDescription, 1> sp{};
		sp[0].colorAttachmentCount = static_cast<std::uint32_t>(colorRef.size());
		sp[0].pColorAttachments = colorRef.data();
		sp[0].pDepthStencilAttachment = &depthRef;

		vk::RenderPassCreateInfo rpInfo{ {},  static_cast<std::uint32_t>(att.size()), att.data(),
										static_cast<std::uint32_t>(sp.size()), sp.data() };

		m_rp = m_device->createRenderPassUnique(rpInfo);
	}

	void GraphicsDeviceVulkan::m_CreateFrameBuffers()
	{
		// Destroy previously created framebuffers
		m_fb.clear();

		// we need as many framebuffers as there are swapchain images
		for (int i{}; i < m_SwapChainImages.size(); ++i)
		{
			std::array imv{ m_sciv[i].get(), m_dsv.get() };

			vk::FramebufferCreateInfo fbInfo{ {}, m_rp.get(), static_cast<std::uint32_t>(imv.size()), imv.data(), m_SwapChainExtent.width, m_SwapChainExtent.height, 1 };
			m_fb.push_back(m_device->createFramebufferUnique(fbInfo));
		}
	}

	GraphicsDeviceVulkan::QueueFamilyIndices GraphicsDeviceVulkan::m_GetQueueFamilies(vk::PhysicalDevice pdev, vk::SurfaceKHR surface)
	{
		QueueFamilyIndices res;

		auto Families = pdev.getQueueFamilyProperties();

		for (int i{}; i < Families.size(); ++i)
		{
			for (std::uint32_t j{}; j < Families[i].queueCount; ++j)
			{
				if (!res.GraphicsQueue && Families[i].queueFlags & vk::QueueFlagBits::eGraphics)
					res.GraphicsQueue = { i, j };
				else if (!res.CopyQueue && Families[i].queueFlags & vk::QueueFlagBits::eTransfer)
					res.CopyQueue = { i, j };
				else if (!res.PresentQueue && pdev.getSurfaceSupportKHR(i, surface))
					res.PresentQueue = { i, j };

				if (res.Filled())
					return res;
			}
		}

		return res;
	}

	bool GraphicsDeviceVulkan::m_CheckPhysicalDeviceExtensionSupport(vk::PhysicalDevice device)
	{
		auto extensions = device.enumerateDeviceExtensionProperties();

		std::set<std::string> RequiredExtensions(m_PhysicalDeviceExtensions.begin(), m_PhysicalDeviceExtensions.end());

		for (const auto& extension : extensions)
			RequiredExtensions.erase(extension.extensionName);

		return RequiredExtensions.empty();
	}

	GraphicsDeviceVulkan::SwapChainSupportDetails GraphicsDeviceVulkan::m_GetSwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
	{
		SwapChainSupportDetails res;

		res.Capabilities = device.getSurfaceCapabilitiesKHR(surface);
		res.Formats = device.getSurfaceFormatsKHR(surface);
		res.PresentModes = device.getSurfacePresentModesKHR(surface);

		return res;
	}

	vk::SurfaceFormatKHR GraphicsDeviceVulkan::m_ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& Formats)
	{
		if (Formats.size() == 1 && Formats[0].format == vk::Format::eUndefined)
			return { Graphics::VK_Format(m_BackBufferFormat), vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear };

		for (const auto& Format : Formats)
			if (Format.format == VK_Format(m_BackBufferFormat) && Format.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
				return Format;

		return Formats[0];
	}

	vk::PresentModeKHR GraphicsDeviceVulkan::m_ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& PresentModes)
	{
		vk::PresentModeKHR BestMode = vk::PresentModeKHR::eFifo; // Always available

		for (const auto& PresentMode : PresentModes)
		{
			if (PresentMode == vk::PresentModeKHR::eMailbox && m_VSync)
				return PresentMode;
			else if (PresentMode == vk::PresentModeKHR::eImmediate && !m_VSync)
				BestMode = PresentMode;
		}

		return BestMode;
	}

	GraphicsDeviceVulkan::GraphicsDeviceVulkan(std::shared_ptr<Window> window, PresentMode mode)
		: GraphicsDevice(window, mode)
	{
		m_CreateInstance();
		m_CreateSurface();
		m_CreatePhysicalDevice();
		m_CreateLogicalDevice();
		m_CreateSwapChain();
		m_CreateAllocator();
		m_CreateDepthStencilBuffer();
		m_CreateRenderPass();
		m_CreateFrameBuffers();

		m_fence = m_device->createFenceUnique({});
		m_device->resetFences(m_fence.get());

		m_sem = m_device->createSemaphoreUnique({});

		vk::CommandPoolCreateInfo poolInfo{ vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_QueueFamilyIndices.GraphicsQueue->first };
		m_pool = m_device->createCommandPoolUnique(poolInfo);

		vk::CommandBufferAllocateInfo cmdInfo{ m_pool.get(), vk::CommandBufferLevel::ePrimary, 1 };
		m_cmd = m_device->allocateCommandBuffersUnique(cmdInfo);
	}

	GraphicsDeviceVulkan::~GraphicsDeviceVulkan()
	{
		m_GraphicsQueue.waitIdle();
		m_PresentQueue.waitIdle();
		m_CopyQueue.waitIdle();

		vmaFreeMemory(m_vma, alloc);
		vmaDestroyAllocator(m_vma);
	}

	void GraphicsDeviceVulkan::Render()
	{
		auto imageIndex = m_device->acquireNextImageKHR(m_swapchain.get(), std::numeric_limits<std::uint64_t>::max(), vk::Semaphore{}, m_fence.get());
		m_device->waitForFences(m_fence.get(), VK_TRUE, std::numeric_limits<std::uint64_t>::max());
		m_device->resetFences(m_fence.get());
		m_GraphicsQueue.waitIdle();

		vk::CommandBufferBeginInfo begin{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
		m_cmd[0]->begin(begin);

		std::array<vk::ClearValue, 2> cv{};
		// const float* c = DirectX::Colors::LightSteelBlue;
		// cv[0].color.setFloat32({ c[0], c[1], c[2], c[3] });
		// cv[0].color.setFloat32({ 1.f, 1.f, 0.f, 1.f });
		static float color = 0.0f;
		color += 0.03f;
		cv[0].color.setFloat32({sinf(color) * 0.5f + 0.5f,
								sinf(color + 3.141593f / 6.0f) * 0.5f + 0.5f,
								sinf(color + 2.0f * 3.141593f / 6.0f) * 0.5f + 0.5f,
								1.0f});
		cv[1].depthStencil.depth = 1.f;
		cv[1].depthStencil.stencil = 0;

		vk::RenderPassBeginInfo rpBegin{ m_rp.get(), m_fb[imageIndex.value].get(),
										{ { 0, 0 }, { m_SwapChainExtent.width, m_SwapChainExtent.height } },
										static_cast<std::uint32_t>(cv.size()), cv.data() };
		m_cmd[0]->beginRenderPass(rpBegin, vk::SubpassContents::eInline);
		vk::Viewport vp{ 0, 0, 1280, 720, .0f, 1.f };
		m_cmd[0]->setViewport(0, vp);

		m_cmd[0]->endRenderPass();

		m_cmd[0]->end();

		std::vector<vk::CommandBuffer> v;
		std::transform(m_cmd.begin(), m_cmd.end(), std::back_inserter(v), [](const vk::UniqueCommandBuffer& cb) { return cb.get(); });
		vk::SubmitInfo submit{};
		submit.commandBufferCount = static_cast<std::uint32_t>(v.size());
		submit.pCommandBuffers = v.data();
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &m_sem.get();
		m_GraphicsQueue.submit(submit, vk::Fence{});

		vk::PresentInfoKHR presentInfo{ 1, &m_sem.get(), 1, &m_swapchain.get(), &imageIndex.value};
		m_PresentQueue.presentKHR(presentInfo);
	}
}