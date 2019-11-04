#include "GraphicsDeviceDX12.h"
#include "Window.h"

#include "d3dx12.h"
#include <DirectXColors.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275)
#endif // _MSC_VER
#include <spdlog/spdlog.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#include <stdexcept>
#include <array>
#include <string>
#include <cassert>

namespace sisskey
{
	namespace Graphics
	{
		constexpr inline D3D12_PRIMITIVE_TOPOLOGY_TYPE DX12_PrimitiveTopologyType(PRIMITIVE_TOPOLOGY value)
		{
			switch (value)
			{
			case PRIMITIVE_TOPOLOGY::UNDEFINED:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			case PRIMITIVE_TOPOLOGY::TRIANGLELIST:
			case PRIMITIVE_TOPOLOGY::TRIANGLESTRIP:	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			case PRIMITIVE_TOPOLOGY::POINTLIST:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case PRIMITIVE_TOPOLOGY::LINELIST:
			case PRIMITIVE_TOPOLOGY::LINESTRIP:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case PRIMITIVE_TOPOLOGY::PATCHLIST:		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			default:								return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			}
		}

		constexpr inline D3D12_PRIMITIVE_TOPOLOGY DX12_PrimitiveTopology(PRIMITIVE_TOPOLOGY value)
		{
			switch (value)
			{
			case PRIMITIVE_TOPOLOGY::UNDEFINED:		return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			case PRIMITIVE_TOPOLOGY::TRIANGLELIST:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PRIMITIVE_TOPOLOGY::TRIANGLESTRIP:	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			case PRIMITIVE_TOPOLOGY::POINTLIST:		return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PRIMITIVE_TOPOLOGY::LINELIST:		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case PRIMITIVE_TOPOLOGY::LINESTRIP:		return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PRIMITIVE_TOPOLOGY::PATCHLIST:		return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST; // ??
			default:								return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}

		constexpr inline UINT DX12_ColorWriteMask(COLOR_WRITE_ENABLE value)
		{
			UINT flag{};

			if (value == COLOR_WRITE_ENABLE::ALL)
				return D3D12_COLOR_WRITE_ENABLE_ALL;
			else
			{
				if ((value & COLOR_WRITE_ENABLE::RED) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= D3D12_COLOR_WRITE_ENABLE_RED;
				if ((value & COLOR_WRITE_ENABLE::GREEN) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= D3D12_COLOR_WRITE_ENABLE_GREEN;
				if ((value & COLOR_WRITE_ENABLE::BLUE) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= D3D12_COLOR_WRITE_ENABLE_BLUE;
				if ((value & COLOR_WRITE_ENABLE::ALPHA) != COLOR_WRITE_ENABLE::DISABLE)
					flag |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
			}

			return flag;
		}

		constexpr inline D3D12_RESOURCE_STATES DX12_ResourceStates(RESOURCE_STATES value)
		{
			return static_cast<D3D12_RESOURCE_STATES>(value);
		}

		constexpr inline D3D12_FILTER DX12_Filter(FILTER value)
		{
			switch (value)
			{
			case FILTER::MIN_MAG_MIP_POINT:							return D3D12_FILTER_MIN_MAG_MIP_POINT;
			case FILTER::MIN_MAG_POINT_MIP_LINEAR:					return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			case FILTER::MIN_POINT_MAG_LINEAR_MIP_POINT:			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FILTER::MIN_POINT_MAG_MIP_LINEAR:					return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			case FILTER::MIN_LINEAR_MAG_MIP_POINT:					return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			case FILTER::MIN_LINEAR_MAG_POINT_MIP_LINEAR:			return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FILTER::MIN_MAG_LINEAR_MIP_POINT:					return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			case FILTER::MIN_MAG_MIP_LINEAR:						return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			case FILTER::ANISOTROPIC:								return D3D12_FILTER_ANISOTROPIC;
			case FILTER::COMPARISON_MIN_MAG_MIP_POINT:				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			case FILTER::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:		return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			case FILTER::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:	return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FILTER::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:		return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			case FILTER::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:		return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			case FILTER::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FILTER::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:		return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			case FILTER::COMPARISON_MIN_MAG_MIP_LINEAR:				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			case FILTER::COMPARISON_ANISOTROPIC:					return D3D12_FILTER_COMPARISON_ANISOTROPIC;
			case FILTER::MINIMUM_MIN_MAG_MIP_POINT:					return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
			case FILTER::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:			return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
			case FILTER::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:	return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FILTER::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
			case FILTER::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
			case FILTER::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:	return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FILTER::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:			return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
			case FILTER::MINIMUM_MIN_MAG_MIP_LINEAR:				return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
			case FILTER::MINIMUM_ANISOTROPIC:						return D3D12_FILTER_MINIMUM_ANISOTROPIC;
			case FILTER::MAXIMUM_MIN_MAG_MIP_POINT:					return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
			case FILTER::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:			return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
			case FILTER::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:	return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case FILTER::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
			case FILTER::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
			case FILTER::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:	return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			case FILTER::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:			return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
			case FILTER::MAXIMUM_MIN_MAG_MIP_LINEAR:				return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
			case FILTER::MAXIMUM_ANISOTROPIC:						return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
			default:												return D3D12_FILTER_MIN_MAG_MIP_POINT;
			}
		}

		constexpr inline D3D12_TEXTURE_ADDRESS_MODE DX12_TextureAddressMode(TEXTURE_ADDRESS_MODE value)
		{
			switch (value)
			{
			case TEXTURE_ADDRESS_MODE::WRAP:		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			case TEXTURE_ADDRESS_MODE::MIRROR:		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			case TEXTURE_ADDRESS_MODE::CLAMP:		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			case TEXTURE_ADDRESS_MODE::BORDER:		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			case TEXTURE_ADDRESS_MODE::MIRROR_ONCE:	return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			default:								return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			}
		}

		constexpr inline D3D12_COMPARISON_FUNC DX12_ComparisonFunc(COMPARISON_FUNC value)
		{
			switch (value)
			{
			case COMPARISON_FUNC::NEVER:			return D3D12_COMPARISON_FUNC_NEVER;
			case COMPARISON_FUNC::LESS:				return D3D12_COMPARISON_FUNC_LESS;
			case COMPARISON_FUNC::EQUAL:			return D3D12_COMPARISON_FUNC_EQUAL;
			case COMPARISON_FUNC::LESS_EQUAL:		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case COMPARISON_FUNC::GREATER:			return D3D12_COMPARISON_FUNC_GREATER;
			case COMPARISON_FUNC::NOT_EQUAL:		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case COMPARISON_FUNC::GREATER_EQUAL:	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case COMPARISON_FUNC::ALWAYS:			return D3D12_COMPARISON_FUNC_ALWAYS;
			default:								return D3D12_COMPARISON_FUNC_NEVER;
			}
		}

		constexpr inline D3D12_FILL_MODE DX12_FillMode(FILL_MODE value)
		{
			switch (value)
			{
			case FILL_MODE::WIREFRAME:	return D3D12_FILL_MODE_WIREFRAME;
			case FILL_MODE::SOLID:		return D3D12_FILL_MODE_SOLID;
			default:					return D3D12_FILL_MODE_WIREFRAME;
			}
		}

		constexpr inline D3D12_CULL_MODE DX12_CullMode(CULL_MODE value)
		{
			switch (value)
			{
			case CULL_MODE::NONE:	return D3D12_CULL_MODE_NONE;
			case CULL_MODE::FRONT:	return D3D12_CULL_MODE_FRONT;
			case CULL_MODE::BACK:	return D3D12_CULL_MODE_BACK;
			default:				return D3D12_CULL_MODE_NONE;
			}
		}

		constexpr inline D3D12_DEPTH_WRITE_MASK DX12_DepthWriteMask(DEPTH_WRITE_MASK value)
		{
			switch (value)
			{
			case DEPTH_WRITE_MASK::ZERO:	return D3D12_DEPTH_WRITE_MASK_ZERO;
			case DEPTH_WRITE_MASK::ALL:		return D3D12_DEPTH_WRITE_MASK_ALL;
			default:						return D3D12_DEPTH_WRITE_MASK_ZERO;
			}
		}

		constexpr inline D3D12_STENCIL_OP DX12_StencilOp(STENCIL_OP value)
		{
			switch (value)
			{
			case STENCIL_OP::KEEP:		return D3D12_STENCIL_OP_KEEP;
			case STENCIL_OP::ZERO:		return D3D12_STENCIL_OP_ZERO;
			case STENCIL_OP::REPLACE:	return D3D12_STENCIL_OP_REPLACE;
			case STENCIL_OP::INCR_SAT:	return D3D12_STENCIL_OP_INCR_SAT;
			case STENCIL_OP::DECR_SAT:	return D3D12_STENCIL_OP_DECR_SAT;
			case STENCIL_OP::INVERT:	return D3D12_STENCIL_OP_INVERT;
			case STENCIL_OP::INCR:		return D3D12_STENCIL_OP_INCR;
			case STENCIL_OP::DECR:		return D3D12_STENCIL_OP_DECR;
			default:					return D3D12_STENCIL_OP_KEEP;
			}
		}

		constexpr inline D3D12_BLEND DX12_Blend(BLEND value)
		{
			switch (value)
			{
			case BLEND::ZERO:				return D3D12_BLEND_ZERO;
			case BLEND::ONE:				return D3D12_BLEND_ONE;
			case BLEND::SRC_COLOR:			return D3D12_BLEND_SRC_COLOR;
			case BLEND::INV_SRC_COLOR:		return D3D12_BLEND_INV_SRC_COLOR;
			case BLEND::SRC_ALPHA:			return D3D12_BLEND_SRC_ALPHA;
			case BLEND::INV_SRC_ALPHA:		return D3D12_BLEND_INV_SRC_ALPHA;
			case BLEND::DEST_ALPHA:			return D3D12_BLEND_DEST_ALPHA;
			case BLEND::INV_DEST_ALPHA:		return D3D12_BLEND_INV_DEST_ALPHA;
			case BLEND::DEST_COLOR:			return D3D12_BLEND_DEST_COLOR;
			case BLEND::INV_DEST_COLOR:		return D3D12_BLEND_INV_DEST_COLOR;
			case BLEND::SRC_ALPHA_SAT:		return D3D12_BLEND_SRC_ALPHA_SAT;
			case BLEND::BLEND_FACTOR:		return D3D12_BLEND_BLEND_FACTOR;
			case BLEND::INV_BLEND_FACTOR:	return D3D12_BLEND_INV_BLEND_FACTOR;
			case BLEND::SRC1_COLOR:			return D3D12_BLEND_SRC1_COLOR;
			case BLEND::INV_SRC1_COLOR:		return D3D12_BLEND_INV_SRC1_COLOR;
			case BLEND::SRC1_ALPHA:			return D3D12_BLEND_SRC1_ALPHA;
			case BLEND::INV_SRC1_ALPHA:		return D3D12_BLEND_INV_SRC1_ALPHA;
			default:						return D3D12_BLEND_ZERO;
			}
		}

		constexpr inline D3D12_BLEND_OP DX12_BlendOp(BLEND_OP value)
		{
			switch (value)
			{
			case BLEND_OP::ADD:				return D3D12_BLEND_OP_ADD;
			case BLEND_OP::SUBTRACT:		return D3D12_BLEND_OP_SUBTRACT;
			case BLEND_OP::REV_SUBTRACT:	return D3D12_BLEND_OP_REV_SUBTRACT;
			case BLEND_OP::MIN:				return D3D12_BLEND_OP_MIN;
			case BLEND_OP::MAX:				return D3D12_BLEND_OP_MAX;
			default:						return D3D12_BLEND_OP_ADD;
			}
		}
		
		constexpr inline D3D12_INPUT_CLASSIFICATION DX12_InputClassification(INPUT_CLASSIFICATION value)
		{
			switch (value)
			{
			case INPUT_CLASSIFICATION::INPUT_PER_VERTEX_DATA:	return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			case INPUT_CLASSIFICATION::INPUT_PER_INSTANCE_DATA:	return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			default:											return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			}
		}

		constexpr inline DXGI_FORMAT DX12_Format(FORMAT value)
		{
			switch (value)
			{
			case FORMAT::UNKNOWN:				return DXGI_FORMAT_UNKNOWN;

			case FORMAT::R32G32B32A32_FLOAT:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case FORMAT::R32G32B32A32_UINT:		return DXGI_FORMAT_R32G32B32A32_UINT;
			case FORMAT::R32G32B32A32_SINT:		return DXGI_FORMAT_R32G32B32A32_SINT;

			case FORMAT::R32G32B32_FLOAT:		return DXGI_FORMAT_R32G32B32_FLOAT;
			case FORMAT::R32G32B32_UINT:		return DXGI_FORMAT_R32G32B32_UINT;
			case FORMAT::R32G32B32_SINT:		return DXGI_FORMAT_R32G32B32_SINT;

			case FORMAT::R16G16B16A16_FLOAT:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case FORMAT::R16G16B16A16_UNORM:	return DXGI_FORMAT_R16G16B16A16_UNORM;
			case FORMAT::R16G16B16A16_UINT:		return DXGI_FORMAT_R16G16B16A16_UINT;
			case FORMAT::R16G16B16A16_SNORM:	return DXGI_FORMAT_R16G16B16A16_SNORM;
			case FORMAT::R16G16B16A16_SINT:		return DXGI_FORMAT_R16G16B16A16_SINT;

			case FORMAT::R32G32_FLOAT:			return DXGI_FORMAT_R32G32_FLOAT;
			case FORMAT::R32G32_UINT:			return DXGI_FORMAT_R32G32_UINT;
			case FORMAT::R32G32_SINT:			return DXGI_FORMAT_R32G32_SINT;
			case FORMAT::R32G8X24_TYPELESS:		return DXGI_FORMAT_R32G8X24_TYPELESS;
			case FORMAT::D32_FLOAT_S8X24_UINT:	return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

			case FORMAT::R10G10B10A2_UNORM:		return DXGI_FORMAT_R10G10B10A2_UNORM;
			case FORMAT::R10G10B10A2_UINT:		return DXGI_FORMAT_R10G10B10A2_UINT;
			case FORMAT::R11G11B10_FLOAT:		return DXGI_FORMAT_R11G11B10_FLOAT;

			case FORMAT::R8G8B8A8_UNORM:		return DXGI_FORMAT_R8G8B8A8_UNORM;
			case FORMAT::R8G8B8A8_UNORM_SRGB:	return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case FORMAT::R8G8B8A8_UINT:			return DXGI_FORMAT_R8G8B8A8_UINT;
			case FORMAT::R8G8B8A8_SNORM:		return DXGI_FORMAT_R8G8B8A8_SNORM;
			case FORMAT::R8G8B8A8_SINT:			return DXGI_FORMAT_R8G8B8A8_SINT;
			case FORMAT::B8G8R8A8_UNORM:		return DXGI_FORMAT_B8G8R8A8_UNORM;
			case FORMAT::B8G8R8A8_UNORM_SRGB:	return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

			case FORMAT::R16G16_FLOAT:			return DXGI_FORMAT_R16G16_FLOAT;
			case FORMAT::R16G16_UNORM:			return DXGI_FORMAT_R16G16_UNORM;
			case FORMAT::R16G16_UINT:			return DXGI_FORMAT_R16G16_UINT;
			case FORMAT::R16G16_SNORM:			return DXGI_FORMAT_R16G16_SNORM;
			case FORMAT::R16G16_SINT:			return DXGI_FORMAT_R16G16_SINT;

			case FORMAT::R32_TYPELESS:			return DXGI_FORMAT_R32_TYPELESS;
			case FORMAT::D32_FLOAT:				return DXGI_FORMAT_D32_FLOAT;
			case FORMAT::R32_FLOAT:				return DXGI_FORMAT_R32_FLOAT;
			case FORMAT::R32_UINT:				return DXGI_FORMAT_R32_UINT;
			case FORMAT::R32_SINT:				return DXGI_FORMAT_R32_SINT;
			case FORMAT::R24G8_TYPELESS:		return DXGI_FORMAT_R24G8_TYPELESS;
			case FORMAT::D24_UNORM_S8_UINT:		return DXGI_FORMAT_D24_UNORM_S8_UINT;

			case FORMAT::R8G8_UNORM:			return DXGI_FORMAT_R8G8_UNORM;
			case FORMAT::R8G8_UINT:				return DXGI_FORMAT_R8G8_UINT;
			case FORMAT::R8G8_SNORM:			return DXGI_FORMAT_R8G8_SNORM;
			case FORMAT::R8G8_SINT:				return DXGI_FORMAT_R8G8_SINT;
			case FORMAT::R16_TYPELESS:			return DXGI_FORMAT_R16_TYPELESS;
			case FORMAT::R16_FLOAT:				return DXGI_FORMAT_R16_FLOAT;
			case FORMAT::D16_UNORM:				return DXGI_FORMAT_D16_UNORM;
			case FORMAT::R16_UNORM:				return DXGI_FORMAT_R16_UNORM;
			case FORMAT::R16_UINT:				return DXGI_FORMAT_R16_UINT;
			case FORMAT::R16_SNORM:				return DXGI_FORMAT_R16_SNORM;
			case FORMAT::R16_SINT:				return DXGI_FORMAT_R16_SINT;

			case FORMAT::R8_UNORM:				return DXGI_FORMAT_R8_UNORM;
			case FORMAT::R8_UINT:				return DXGI_FORMAT_R8_UINT;
			case FORMAT::R8_SNORM:				return DXGI_FORMAT_R8_SNORM;
			case FORMAT::R8_SINT:				return DXGI_FORMAT_R8_SINT;

			case FORMAT::BC1_UNORM:				return DXGI_FORMAT_BC1_UNORM;
			case FORMAT::BC1_UNORM_SRGB:		return DXGI_FORMAT_BC1_UNORM_SRGB;
			case FORMAT::BC2_UNORM:				return DXGI_FORMAT_BC2_UNORM;
			case FORMAT::BC2_UNORM_SRGB:		return DXGI_FORMAT_BC2_UNORM_SRGB;
			case FORMAT::BC3_UNORM:				return DXGI_FORMAT_BC3_UNORM;
			case FORMAT::BC3_UNORM_SRGB:		return DXGI_FORMAT_BC3_UNORM_SRGB;
			case FORMAT::BC4_UNORM:				return DXGI_FORMAT_BC4_UNORM;
			case FORMAT::BC4_SNORM:				return DXGI_FORMAT_BC4_SNORM;
			case FORMAT::BC5_UNORM:				return DXGI_FORMAT_BC5_UNORM;
			case FORMAT::BC5_SNORM:				return DXGI_FORMAT_BC5_SNORM;
			case FORMAT::BC6H_UF16:				return DXGI_FORMAT_BC6H_UF16;
			case FORMAT::BC6H_SF16:				return DXGI_FORMAT_BC6H_SF16;
			case FORMAT::BC7_UNORM:				return DXGI_FORMAT_BC7_UNORM;
			case FORMAT::BC7_UNORM_SRGB:		return DXGI_FORMAT_BC7_UNORM_SRGB;
			default:							return DXGI_FORMAT_UNKNOWN;
			}
		}

		constexpr inline D3D12_SUBRESOURCE_DATA DX12_SubresourceData(const SubresourceData& pInitialData)
		{
			D3D12_SUBRESOURCE_DATA data{};
			data.pData = pInitialData.pSysMem;
			data.RowPitch = pInitialData.SysMemPitch;
			data.SlicePitch = pInitialData.SysMemSlicePitch;

			return data;
		}


		constexpr inline FORMAT SK_Format(DXGI_FORMAT value)
		{
			switch (value)
			{
			case DXGI_FORMAT_UNKNOWN:				return FORMAT::UNKNOWN;

			case DXGI_FORMAT_R32G32B32A32_FLOAT:	return FORMAT::R32G32B32A32_FLOAT;
			case DXGI_FORMAT_R32G32B32A32_UINT:		return FORMAT::R32G32B32A32_UINT;
			case DXGI_FORMAT_R32G32B32A32_SINT:		return FORMAT::R32G32B32A32_SINT;

			case DXGI_FORMAT_R32G32B32_FLOAT:		return FORMAT::R32G32B32_FLOAT;
			case DXGI_FORMAT_R32G32B32_UINT:		return FORMAT::R32G32B32_UINT;
			case DXGI_FORMAT_R32G32B32_SINT:		return FORMAT::R32G32B32_SINT;

			case DXGI_FORMAT_R16G16B16A16_FLOAT:	return FORMAT::R16G16B16A16_FLOAT;
			case DXGI_FORMAT_R16G16B16A16_UNORM:	return FORMAT::R16G16B16A16_UNORM;
			case DXGI_FORMAT_R16G16B16A16_UINT:		return FORMAT::R16G16B16A16_UINT;
			case DXGI_FORMAT_R16G16B16A16_SNORM:	return FORMAT::R16G16B16A16_SNORM;
			case DXGI_FORMAT_R16G16B16A16_SINT:		return FORMAT::R16G16B16A16_SINT;

			case DXGI_FORMAT_R32G32_FLOAT:			return FORMAT::R32G32_FLOAT;
			case DXGI_FORMAT_R32G32_UINT:			return FORMAT::R32G32_UINT;
			case DXGI_FORMAT_R32G32_SINT:			return FORMAT::R32G32_SINT;
			case DXGI_FORMAT_R32G8X24_TYPELESS:		return FORMAT::R32G8X24_TYPELESS;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:	return FORMAT::D32_FLOAT_S8X24_UINT;

			case DXGI_FORMAT_R10G10B10A2_UNORM:		return FORMAT::R10G10B10A2_UNORM;
			case DXGI_FORMAT_R10G10B10A2_UINT:		return FORMAT::R10G10B10A2_UINT;
			case DXGI_FORMAT_R11G11B10_FLOAT:		return FORMAT::R11G11B10_FLOAT;

			case DXGI_FORMAT_R8G8B8A8_UNORM:		return FORMAT::R8G8B8A8_UNORM;
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:	return FORMAT::R8G8B8A8_UNORM_SRGB;
			case DXGI_FORMAT_R8G8B8A8_UINT:			return FORMAT::R8G8B8A8_UINT;
			case DXGI_FORMAT_R8G8B8A8_SNORM:		return FORMAT::R8G8B8A8_SNORM;
			case DXGI_FORMAT_R8G8B8A8_SINT:			return FORMAT::R8G8B8A8_SINT;
			case DXGI_FORMAT_B8G8R8A8_UNORM:		return FORMAT::B8G8R8A8_UNORM;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:	return FORMAT::B8G8R8A8_UNORM_SRGB;

			case DXGI_FORMAT_R16G16_FLOAT:			return FORMAT::R16G16_FLOAT;
			case DXGI_FORMAT_R16G16_UNORM:			return FORMAT::R16G16_UNORM;
			case DXGI_FORMAT_R16G16_UINT:			return FORMAT::R16G16_UINT;
			case DXGI_FORMAT_R16G16_SNORM:			return FORMAT::R16G16_SNORM;
			case DXGI_FORMAT_R16G16_SINT:			return FORMAT::R16G16_SINT;

			case DXGI_FORMAT_R32_TYPELESS:			return FORMAT::R32_TYPELESS;
			case DXGI_FORMAT_D32_FLOAT:				return FORMAT::D32_FLOAT;
			case DXGI_FORMAT_R32_FLOAT:				return FORMAT::R32_FLOAT;
			case DXGI_FORMAT_R32_UINT:				return FORMAT::R32_UINT;
			case DXGI_FORMAT_R32_SINT:				return FORMAT::R32_SINT;
			case DXGI_FORMAT_R24G8_TYPELESS:		return FORMAT::R24G8_TYPELESS;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:		return FORMAT::D24_UNORM_S8_UINT;

			case DXGI_FORMAT_R8G8_UNORM:			return FORMAT::R8G8_UNORM;
			case DXGI_FORMAT_R8G8_UINT:				return FORMAT::R8G8_UINT;
			case DXGI_FORMAT_R8G8_SNORM:			return FORMAT::R8G8_SNORM;
			case DXGI_FORMAT_R8G8_SINT:				return FORMAT::R8G8_SINT;
			case DXGI_FORMAT_R16_TYPELESS:			return FORMAT::R16_TYPELESS;
			case DXGI_FORMAT_R16_FLOAT:				return FORMAT::R16_FLOAT;
			case DXGI_FORMAT_D16_UNORM:				return FORMAT::D16_UNORM;
			case DXGI_FORMAT_R16_UNORM:				return FORMAT::R16_UNORM;
			case DXGI_FORMAT_R16_UINT:				return FORMAT::R16_UINT;
			case DXGI_FORMAT_R16_SNORM:				return FORMAT::R16_SNORM;
			case DXGI_FORMAT_R16_SINT:				return FORMAT::R16_SINT;

			case DXGI_FORMAT_R8_UNORM:				return FORMAT::R8_UNORM;
			case DXGI_FORMAT_R8_UINT:				return FORMAT::R8_UINT;
			case DXGI_FORMAT_R8_SNORM:				return FORMAT::R8_SNORM;
			case DXGI_FORMAT_R8_SINT:				return FORMAT::R8_SINT;

			case DXGI_FORMAT_BC1_UNORM:				return FORMAT::BC1_UNORM;
			case DXGI_FORMAT_BC1_UNORM_SRGB:		return FORMAT::BC1_UNORM_SRGB;
			case DXGI_FORMAT_BC2_UNORM:				return FORMAT::BC2_UNORM;
			case DXGI_FORMAT_BC2_UNORM_SRGB:		return FORMAT::BC2_UNORM_SRGB;
			case DXGI_FORMAT_BC3_UNORM:				return FORMAT::BC3_UNORM;
			case DXGI_FORMAT_BC3_UNORM_SRGB:		return FORMAT::BC3_UNORM_SRGB;
			case DXGI_FORMAT_BC4_UNORM:				return FORMAT::BC4_UNORM;
			case DXGI_FORMAT_BC4_SNORM:				return FORMAT::BC4_SNORM;
			case DXGI_FORMAT_BC5_UNORM:				return FORMAT::BC5_UNORM;
			case DXGI_FORMAT_BC5_SNORM:				return FORMAT::BC5_SNORM;
			case DXGI_FORMAT_BC6H_UF16:				return FORMAT::BC6H_UF16;
			case DXGI_FORMAT_BC6H_SF16:				return FORMAT::BC6H_SF16;
			case DXGI_FORMAT_BC7_UNORM:				return FORMAT::BC7_UNORM;
			case DXGI_FORMAT_BC7_UNORM_SRGB:		return FORMAT::BC7_UNORM_SRGB;
			default:								return FORMAT::UNKNOWN;
			}
		}

		constexpr inline RESOURCE_STATES SK_ResourceStates(D3D12_RESOURCE_STATES value)
		{
			return static_cast<RESOURCE_STATES>(value);
		}

		constexpr inline TextureDesc SK_TextureDesc(const D3D12_RESOURCE_DESC& desc)
		{
			TextureDesc retVal;

			retVal.Format = SK_Format(desc.Format);
			retVal.Width = static_cast<unsigned int>(desc.Width);
			retVal.Height = desc.Height;
			retVal.MipLevels = desc.MipLevels;

			return retVal;
		}
	}

	// https://www.3dgep.com/learning-directx-12-1/#Query_DirectX_12_Adapter
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-d3d12createdevice
	Microsoft::WRL::ComPtr<IDXGIAdapter4> GraphicsDeviceDX12::m_GetAdapter()
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter4> ret;

		// Try to select a discrete GPU that supports DX12
		// and has maximum VRAM
		SIZE_T maxDedicatedVideoMemory{};
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		for (UINT i{}; m_pFactory->EnumAdapters1(i, &adapter) == S_OK; ++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			// Check to see if the adapter can create a D3D12 device without actually 
			// creating it. The adapter with the largest dedicated video memory is favored.
			if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) &&
				desc.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
				ThrowIfFailed(adapter.As(&ret));
			}
		}

		// Fallback to WARP adapter
		if (!ret)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			ThrowIfFailed(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
			ThrowIfFailed(pWarpAdapter.As(&ret));
			spdlog::info("Fallback to WARP adapter");
		}

		return ret;
	}

	GraphicsDeviceDX12::GraphicsDeviceDX12(std::shared_ptr<Window> window, PresentMode mode)
		: GraphicsDevice(window, mode)
	{
#ifndef NDEBUG
		{
			Microsoft::WRL::ComPtr<ID3D12Debug> Debug;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
			Debug->EnableDebugLayer();
		}
#endif // !NDEBUG

		// Create DXGI Factory
		{
			Microsoft::WRL::ComPtr<IDXGIFactory2> f2;
			UINT FactoryCreateFlags{};
#ifndef NDEBUG
			FactoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // !NDEBUG
			ThrowIfFailed(CreateDXGIFactory2(FactoryCreateFlags, IID_PPV_ARGS(&f2)));
			ThrowIfFailed(f2.As(&m_pFactory));
		}

		// Create Device
		// https://www.3dgep.com/learning-directx-12-1/#Create_the_DirectX_12_Device
		{
			Microsoft::WRL::ComPtr<ID3D12Device> dev;
			ThrowIfFailed(D3D12CreateDevice(m_GetAdapter().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dev)));
			ThrowIfFailed(dev.As(&m_pDevice));

#ifndef NDEBUG
			Microsoft::WRL::ComPtr<ID3D12InfoQueue> pInfoQueue;
			ThrowIfFailed(m_pDevice.As(&pInfoQueue));
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			// Suppress whole categories of messages
			std::array<D3D12_MESSAGE_CATEGORY, 0> Categories; // suppress no categories

			// Suppress messages based on their severity level
			std::array Severities
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			std::array DenyIds // TODO: check if it's needed
			{
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter{};
			NewFilter.DenyList.NumCategories = static_cast<UINT>(Categories.size());
			NewFilter.DenyList.pCategoryList = Categories.data();
			NewFilter.DenyList.NumSeverities = static_cast<UINT>(Severities.size());
			NewFilter.DenyList.pSeverityList = Severities.data();
			NewFilter.DenyList.NumIDs = static_cast<UINT>(DenyIds.size());
			NewFilter.DenyList.pIDList = DenyIds.data();

			ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
#endif // !NDEBUG
		}

		// Create D3D12MemoryAllocator
		{
			D3D12MA::ALLOCATOR_DESC allocatorDesc{};
			allocatorDesc.pDevice = m_pDevice.Get();

			D3D12MA::Allocator* allocator;
			HRESULT hr = D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
			m_d3dma = UniqueD3D12MemoryAllocator{ allocator };
		}

		// Create Fence
		{
			Microsoft::WRL::ComPtr<ID3D12Fence> f;
			ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&f)));
			ThrowIfFailed(f.As(&m_pFence));
		}

		// Store increments
		m_RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Check 4xMSAA support
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = DX12_Format(m_BackBufferFormat);
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		ThrowIfFailed(m_pDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
		m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
		
		// Create DirectCommandQueue, CommandAllocator and GraphicsCommandList
		D3D12_COMMAND_QUEUE_DESC queueDesc{};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
		ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pDirectCmdListAlloc)));
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cl;
			ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pDirectCmdListAlloc.Get(), // Associated command allocator
													   nullptr, // Initial PipelineStateObject
													   IID_PPV_ARGS(&cl)));
			ThrowIfFailed(cl.As(&m_pCommandList));
		}

		// Create CopyQueue, CopyCommandAllocator and CopyCommandList
		D3D12_COMMAND_QUEUE_DESC copyQueueDesc{};
		copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		ThrowIfFailed(m_pDevice->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&m_pCopyQueue)));
		ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_pCopyAllocator)));
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cl;
			ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_pCopyAllocator.Get(), // Associated command allocator
													   nullptr, // Initial PipelineStateObject
													   IID_PPV_ARGS(&cl)));
			ThrowIfFailed(cl.As(&m_pCopyCommandList));

			ThrowIfFailed(m_pCopyCommandList->Close());
			ThrowIfFailed(m_pCopyAllocator->Reset());
			ThrowIfFailed(m_pCopyCommandList->Reset(m_pCopyAllocator.Get(), nullptr));

			ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pCopyFence)));
			m_CopyFenceEvent = CreateEventExW(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
			m_CopyFenceValue = 1;
		}

		m_CreateSwapChain();

		// Create descriptor heaps
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = m_BackBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle{ m_pRtvHeap->GetCPUDescriptorHandleForHeapStart() };
		for (int i{}; i < m_BackBufferCount; ++i)
		{
			// Get the ith buffer in the swap chain.
			ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
			// Create an RTV to it.
			m_pDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			// Next entry in heap.
			rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
		}

		// TODO: offscreen rendering
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect
		// To use multisampling with DXGI_SWAP_EFFECT_SEQUENTIAL or DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		// you must perform the multisampling in a separate render target.
		// For example, create a multisampled texture by calling ID3D11Device::CreateTexture2D
		// with a filled D3D11_TEXTURE2D_DESC structure (BindFlags member set to D3D11_BIND_RENDER_TARGET and SampleDesc member with multisampling parameters).
		// Next call ID3D11Device::CreateRenderTargetView to create a render-target view for the texture, and render your scene into the texture.
		// Finally call ID3D11DeviceContext::ResolveSubresource to resolve the multisampled texture into your non-multisampled swap chain.

		// Create the depth/stencil buffer and view.
		// Reverse Z ??
		{
			D3D12MA::ALLOCATION_DESC allocDesc{};
			allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
			Microsoft::WRL::ComPtr<ID3D12Resource> dsb;
			D3D12MA::Allocation* a;
			ThrowIfFailed(m_d3dma->CreateResource(&allocDesc,
												  &CD3DX12_RESOURCE_DESC::Tex2D(Graphics::DX12_Format(m_DepthStencilFormat),
																				m_Width, m_Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
												  D3D12_RESOURCE_STATE_COMMON, &CD3DX12_CLEAR_VALUE(Graphics::DX12_Format(m_DepthStencilFormat), 1.f, 0), &a, IID_PPV_ARGS(&dsb)));
			m_DSAlloc = UniqueD3D12MemoryAllocation{ a };
			ThrowIfFailed(dsb.As(&m_pDepthStencilBuffer));
		}
		// Create descriptor to mip level 0 of entire resource using the
		// format of the resource.
		m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_DepthStencilView());
		// Transition the resource from its initial state to be used as a depth buffer.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		// Set ViewPort and Scissors
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;
		m_ViewPort.Width = static_cast<float>(m_Width);
		m_ViewPort.Height = static_cast<float>(m_Height);
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.MaxDepth = 1.0f;
		m_pCommandList->RSSetViewports(1, &m_ViewPort);

		m_ScissorRect = { 0, 0, m_Width / 2, m_Height / 2 };
		m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

		// Commit initialization commands
		ThrowIfFailed(m_pCommandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
		m_pCommandQueue->ExecuteCommandLists(1, cmdsLists);
		m_FlushCommandQueue();

		// TODO
		D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> rootSigError;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &rootSigError));
		ThrowIfFailed(m_pDevice->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_grs)));
	}

	void GraphicsDeviceDX12::Begin()
	{
		// Reuse the memory associated with command recording.
		// We can only reset when the associated command lists have finished
		// execution on the GPU.
		ThrowIfFailed(m_pDirectCmdListAlloc->Reset());
		// A command list can be reset after it has been added to the 
		// command queue via ExecuteCommandList. Reusing the command list reuses memory.
		ThrowIfFailed(m_pCommandList->Reset(m_pDirectCmdListAlloc.Get(), nullptr));
		// Indicate a state transition on the resource usage.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		// Clear the back buffer and depth buffer.
		m_pCommandList->ClearRenderTargetView(m_CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
		m_pCommandList->ClearDepthStencilView(m_DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		// Specify the buffers we are going to render to.
		m_pCommandList->OMSetRenderTargets(1, &m_CurrentBackBufferView(), true, &m_DepthStencilView());

		// TODO
		m_pCommandList->SetGraphicsRootSignature(m_grs.Get());
	}

	void GraphicsDeviceDX12::End()
	{
		// Indicate a state transition on the resource usage.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		{
			std::lock_guard l{ m_CopyMutex };
			m_pCopyCommandList->Close();
			ID3D12CommandList* cmdsLists[] = { m_pCopyCommandList.Get() };
			m_pCopyQueue->ExecuteCommandLists(1, cmdsLists);

			// Signal and increment the fence value.
			UINT64 fenceToWaitFor = m_CopyFenceValue++;
			ThrowIfFailed(m_pCopyQueue->Signal(m_pCopyFence.Get(), fenceToWaitFor));

			// Wait until the GPU is done copying.
			if (m_pCopyFence->GetCompletedValue() < fenceToWaitFor)
			{
				ThrowIfFailed(m_pCopyFence->SetEventOnCompletion(fenceToWaitFor, m_CopyFenceEvent));
				WaitForSingleObject(m_CopyFenceEvent, INFINITE);
			}

			ThrowIfFailed(m_pCopyAllocator->Reset());
			ThrowIfFailed(m_pCopyCommandList->Reset(m_pCopyAllocator.Get(), nullptr));

			std::for_each(m_copyBuffers.begin(), m_copyBuffers.end(),
						  [](Graphics::buffer b)
						  {
							  reinterpret_cast<ID3D12Resource*>(b.resource)->Release();
							  reinterpret_cast<D3D12MA::Allocation*>(b.allocation)->Release();
						  });
			m_copyBuffers.clear();
		}

		// Done recording commands.
		ThrowIfFailed(m_pCommandList->Close());
		// Add the command list to the queue for execution.
		ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
		m_pCommandQueue->ExecuteCommandLists(1, cmdsLists);

		// swap the back and front buffers
		UINT SyncInterval{ m_VSync ? 1u : 0u };
		// https://docs.microsoft.com/ru-ru/windows/win32/direct3ddxgi/dxgi-present
		UINT Flags{ !m_VSync && m_TearingSupport && m_PresentMode != PresentMode::Fullscreen ? DXGI_PRESENT_ALLOW_TEARING : 0u }; // Note: VFR is not allowed in fullscreen mode
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-present
		ThrowIfFailed(m_pSwapChain->Present(SyncInterval, Flags));
		// Wait until frame commands are complete. This waiting is
		// inefficient and is done for simplicity. Later we will show how to
		// organize our rendering code so we do not have to wait per frame.
		m_FlushCommandQueue();
	}

	void GraphicsDeviceDX12::BindPipeline(Graphics::PipelineHandle pipeline)
	{
		m_pCommandList->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(pipeline.ph));
		m_pCommandList->IASetPrimitiveTopology(Graphics::DX12_PrimitiveTopology(pipeline.pt));
	}

	void GraphicsDeviceDX12::BindVertexBuffers(std::uint32_t start, const std::vector<Graphics::buffer>& buffers, const std::vector<std::uint64_t>& offsets, const std::vector<std::uint32_t>& strides)
	{
		assert(buffers.size() == offsets.size());
		assert(buffers.size() == strides.size());
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vb(buffers.size());

		for (int i{}; i < vb.size(); ++i)
		{
			vb[i] = { reinterpret_cast<ID3D12Resource*>(buffers[i].resource)->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(offsets[i]),
					static_cast<UINT>(reinterpret_cast<D3D12MA::Allocation*>(buffers[i].allocation)->GetSize() - offsets[i]), // ??
					strides[i] };
		}

		m_pCommandList->IASetVertexBuffers(start, static_cast<UINT>(vb.size()), vb.data());
	}

	void GraphicsDeviceDX12::BindViewports(const std::vector<Graphics::Viewport>& viewports)
	{
		std::vector<D3D12_VIEWPORT> vp(viewports.size());
		std::transform(viewports.begin(), viewports.end(), vp.begin(),
					   [](const Graphics::Viewport& v) -> D3D12_VIEWPORT
					   {
						   return { v.TopLeftX, v.TopLeftY, v.Width, v.Height, v.MinDepth, v.MaxDepth };
					   });
		m_pCommandList->RSSetViewports(static_cast<UINT>(vp.size()), vp.data());
	}

	void GraphicsDeviceDX12::BindScissorRects(const std::vector<Graphics::Rect>& scissors)
	{
		std::vector<D3D12_RECT> rc(scissors.size());
		std::transform(scissors.begin(), scissors.end(), rc.begin(),
					   [](const Graphics::Rect& r) -> D3D12_RECT
					   {
						   assert(r.right >= r.left);
						   assert(r.bottom >= r.top);
						   return { r.left, r.top, r.right, r.bottom };
					   });
		m_pCommandList->RSSetScissorRects(static_cast<UINT>(rc.size()), rc.data());
	}

	void GraphicsDeviceDX12::BindIndexBuffer(const Graphics::buffer& indexBuffer, std::uint64_t offsest, Graphics::INDEXBUFFER_FORMAT format)
	{
		D3D12_INDEX_BUFFER_VIEW ibv{};
		ibv.BufferLocation = reinterpret_cast<ID3D12Resource*>(indexBuffer.resource)->GetGPUVirtualAddress() + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(offsest);
		ibv.Format = format == Graphics::INDEXBUFFER_FORMAT::UINT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		ibv.SizeInBytes = static_cast<UINT>(reinterpret_cast<D3D12MA::Allocation*>(indexBuffer.allocation)->GetSize() - offsest); // ??
		m_pCommandList->IASetIndexBuffer(&ibv);
	}

	void GraphicsDeviceDX12::Draw(std::uint32_t count, std::uint32_t start)
	{
		m_pCommandList->DrawInstanced(count, 1, start, 0);
	}

	void GraphicsDeviceDX12::DrawIndexed(std::uint32_t count, std::uint32_t startVertex, std::uint32_t startIndex)
	{
		m_pCommandList->DrawIndexedInstanced(count, 1, startIndex, startVertex, 0);
	}

	Graphics::PipelineHandle GraphicsDeviceDX12::CreateGraphicsPipeline(Graphics::GraphicsPipelineDesc& desc)
	{
		// TODO
		constexpr auto CONSERVATIVE_RASTERIZATION = false;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psd{};

		if (desc.vs)
		{
			psd.VS.pShaderBytecode = desc.vs->data();
			psd.VS.BytecodeLength = desc.vs->size();
		}
		if (desc.hs)
		{
			psd.HS.pShaderBytecode = desc.hs->data();
			psd.HS.BytecodeLength = desc.hs->size();
		}
		if (desc.ds)
		{
			psd.DS.pShaderBytecode = desc.ds->data();
			psd.DS.BytecodeLength = desc.ds->size();
		}
		if (desc.gs)
		{
			psd.GS.pShaderBytecode = desc.gs->data();
			psd.GS.BytecodeLength = desc.gs->size();
		}
		if (desc.ps)
		{
			psd.PS.pShaderBytecode = desc.ps->data();
			psd.PS.BytecodeLength = desc.ps->size();
		}

		// D3DX ??
		const Graphics::RasterizerStateDesc& rsd = desc.RasterizerState ? *desc.RasterizerState : Graphics::RasterizerStateDesc{};
		psd.RasterizerState.FillMode				= Graphics::DX12_FillMode(rsd.FillMode);
		psd.RasterizerState.CullMode				= Graphics::DX12_CullMode(rsd.CullMode);
		psd.RasterizerState.FrontCounterClockwise	= rsd.FrontCounterClockwise;
		psd.RasterizerState.DepthBias				= rsd.DepthBias;
		psd.RasterizerState.DepthBiasClamp			= rsd.DepthBiasClamp;
		psd.RasterizerState.SlopeScaledDepthBias	= rsd.SlopeScaledDepthBias;
		psd.RasterizerState.DepthClipEnable			= rsd.DepthClipEnable;
		psd.RasterizerState.MultisampleEnable		= rsd.MultisampleEnable;
		psd.RasterizerState.AntialiasedLineEnable	= rsd.AntialiasedLineEnable;
		psd.RasterizerState.ConservativeRaster		= (CONSERVATIVE_RASTERIZATION && rsd.ConservativeRasterizationEnable)
													   ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		psd.RasterizerState.ForcedSampleCount		= rsd.ForcedSampleCount;

		const Graphics::DepthStencilStateDesc& dsd = desc.DepthStencilState ? *desc.DepthStencilState : Graphics::DepthStencilStateDesc{};
		psd.DepthStencilState.DepthEnable					= dsd.DepthEnable;
		psd.DepthStencilState.DepthWriteMask				= Graphics::DX12_DepthWriteMask(dsd.DepthWriteMask);
		psd.DepthStencilState.DepthFunc						= Graphics::DX12_ComparisonFunc(dsd.DepthFunc);
		psd.DepthStencilState.StencilEnable					= dsd.StencilEnable;
		psd.DepthStencilState.StencilReadMask				= dsd.StencilReadMask;
		psd.DepthStencilState.StencilWriteMask				= dsd.StencilWriteMask;
		psd.DepthStencilState.FrontFace.StencilDepthFailOp	= Graphics::DX12_StencilOp(dsd.FrontFace.StencilDepthFailOp);
		psd.DepthStencilState.FrontFace.StencilFailOp		= Graphics::DX12_StencilOp(dsd.FrontFace.StencilFailOp);
		psd.DepthStencilState.FrontFace.StencilFunc			= Graphics::DX12_ComparisonFunc(dsd.FrontFace.StencilFunc);
		psd.DepthStencilState.FrontFace.StencilPassOp		= Graphics::DX12_StencilOp(dsd.FrontFace.StencilPassOp);
		psd.DepthStencilState.BackFace.StencilDepthFailOp	= Graphics::DX12_StencilOp(dsd.BackFace.StencilDepthFailOp);
		psd.DepthStencilState.BackFace.StencilFailOp		= Graphics::DX12_StencilOp(dsd.BackFace.StencilFailOp);
		psd.DepthStencilState.BackFace.StencilFunc			= Graphics::DX12_ComparisonFunc(dsd.BackFace.StencilFunc);
		psd.DepthStencilState.BackFace.StencilPassOp		= Graphics::DX12_StencilOp(dsd.BackFace.StencilPassOp);

		const Graphics::BlendStateDesc& bsd = desc.BlendState ? *desc.BlendState : Graphics::BlendStateDesc{};
		psd.BlendState.AlphaToCoverageEnable = bsd.AlphaToCoverageEnable;
		psd.BlendState.IndependentBlendEnable = bsd.IndependentBlendEnable;

		static_assert(std::tuple_size_v<decltype(desc.BlendState->RenderTarget)> <= std::size(psd.BlendState.RenderTarget));
		for (size_t i{}; i < std::size(psd.BlendState.RenderTarget); ++i)
		{
			psd.BlendState.RenderTarget[i].BlendEnable = bsd.RenderTarget[i].BlendEnable;
			psd.BlendState.RenderTarget[i].SrcBlend = Graphics::DX12_Blend(bsd.RenderTarget[i].SrcBlend);
			psd.BlendState.RenderTarget[i].DestBlend = Graphics::DX12_Blend(bsd.RenderTarget[i].DestBlend);
			psd.BlendState.RenderTarget[i].BlendOp = Graphics::DX12_BlendOp(bsd.RenderTarget[i].BlendOp);
			psd.BlendState.RenderTarget[i].SrcBlendAlpha = Graphics::DX12_Blend(bsd.RenderTarget[i].SrcBlendAlpha);
			psd.BlendState.RenderTarget[i].DestBlendAlpha = Graphics::DX12_Blend(bsd.RenderTarget[i].DestBlendAlpha);
			psd.BlendState.RenderTarget[i].BlendOpAlpha = Graphics::DX12_BlendOp(bsd.RenderTarget[i].BlendOpAlpha);
			psd.BlendState.RenderTarget[i].RenderTargetWriteMask = Graphics::DX12_ColorWriteMask(bsd.RenderTarget[i].RenderTargetWriteMask);
		}

		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
		if (desc.InputLayout)
		{
			inputElements.resize(static_cast<unsigned int>(desc.InputLayout->size()));

			for (int i{}; i < inputElements.size(); ++i)
			{
				inputElements[i].SemanticName = (*desc.InputLayout)[i].SemanticName;
				inputElements[i].SemanticIndex = (*desc.InputLayout)[i].SemanticIndex;
				inputElements[i].Format = Graphics::DX12_Format((*desc.InputLayout)[i].Format);
				inputElements[i].InputSlot = (*desc.InputLayout)[i].InputSlot;
				inputElements[i].AlignedByteOffset = (*desc.InputLayout)[i].AlignedByteOffset == Graphics::VertexLayoutDesc::APPEND_ALIGNED_ELEMENT
													? D3D12_APPEND_ALIGNED_ELEMENT : (*desc.InputLayout)[i].AlignedByteOffset;
				inputElements[i].InputSlotClass = Graphics::DX12_InputClassification((*desc.InputLayout)[i].InputSlotClass);
				inputElements[i].InstanceDataStepRate = (*desc.InputLayout)[i].InstanceDataStepRate;
			}
		}
		psd.InputLayout.NumElements = static_cast<UINT>(inputElements.size());
		psd.InputLayout.pInputElementDescs = inputElements.data();

		psd.NumRenderTargets = desc.numRTs;
		assert(desc.numRTs <= std::size(psd.RTVFormats));
		static_assert(std::tuple_size_v<decltype(desc.RTFormats)> <= std::size(psd.RTVFormats));
		for (UINT i{}; i < psd.NumRenderTargets; ++i)
			psd.RTVFormats[i] = Graphics::DX12_Format(desc.RTFormats[i]);
		psd.DSVFormat = Graphics::DX12_Format(desc.DSFormat);

		psd.SampleDesc.Count = desc.sampleDesc.Count;
		psd.SampleDesc.Quality = desc.sampleDesc.Quality;
		psd.SampleMask = desc.sampleMask;

		psd.PrimitiveTopologyType = Graphics::DX12_PrimitiveTopologyType(desc.pt);

		// ??
		psd.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		// TODO
		psd.pRootSignature = m_grs.Get();

		ID3D12PipelineState* pso;
		ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psd, IID_PPV_ARGS(&pso)));

		return { reinterpret_cast<Graphics::handle>(pso), desc.pt };
	}

	void GraphicsDeviceDX12::DestroyGraphicsPipeline(Graphics::PipelineHandle pipeline)
	{
		// TODO: gpu sync
		reinterpret_cast<ID3D12PipelineState*>(pipeline.ph)->Release();
	}

	Graphics::buffer GraphicsDeviceDX12::CreateBuffer(Graphics::GPUBufferDesc& desc, std::optional<Graphics::SubresourceData> initData)
	{
		std::uint32_t alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		if (desc.BindFlags & Graphics::BIND_FLAG::CONSTANT_BUFFER)
			alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

		constexpr auto AlignHelper = [](std::uint32_t size, std::uint32_t alignment)
		{
			const auto a = alignment - 1;
			return (size + a) & ~a;
		};
		UINT64 alignedSize = AlignHelper(desc.ByteWidth, alignment);

		// TODO: constant buffers
		D3D12MA::ALLOCATION_DESC allocDesc{};
		allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
		D3D12MA::Allocation* a;
		ID3D12Resource* buffer;
		ThrowIfFailed(m_d3dma->CreateResource(&allocDesc,
											  &CD3DX12_RESOURCE_DESC::Buffer(alignedSize, desc.BindFlags & Graphics::BIND_FLAG::UNORDERED_ACCESS ?
																			 D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE),
											  D3D12_RESOURCE_STATE_COMMON, nullptr, &a, IID_PPV_ARGS(&buffer)));

		if (initData)
		{
			ID3D12Resource* uploadBuffer;
			D3D12MA::Allocation* uploadAllocation;

			D3D12MA::ALLOCATION_DESC uploadAllocDesc{};
			uploadAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
			ThrowIfFailed(m_d3dma->CreateResource(&uploadAllocDesc,
												  &CD3DX12_RESOURCE_DESC::Buffer(alignedSize),
												  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, &uploadAllocation, IID_PPV_ARGS(&uploadBuffer)));

			m_copyBuffers.push_back({ reinterpret_cast<Graphics::handle>(uploadBuffer), reinterpret_cast<Graphics::handle>(uploadAllocation) });

			void* data;
			uploadBuffer->Map(0, nullptr, &data);
			std::memcpy(data, initData->pSysMem, desc.ByteWidth);
			uploadBuffer->Unmap(0, nullptr);

			std::lock_guard l{ m_CopyMutex };
			m_pCopyCommandList->CopyResource(buffer, uploadBuffer);
			//m_pCopyCommandList->CopyBufferRegion(buffer, 0, uploadBuffer, 0, desc.ByteWidth);
		}

		return { reinterpret_cast<Graphics::handle>(buffer), reinterpret_cast<Graphics::handle>(a) };
	}

	void GraphicsDeviceDX12::DestroyBuffer(Graphics::buffer buffer)
	{
		// TODO gpu sync
		reinterpret_cast<ID3D12Resource*>(buffer.resource)->Release();
		reinterpret_cast<D3D12MA::Allocation*>(buffer.allocation)->Release();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDeviceDX12::m_CurrentBackBufferView() const
	{
		// CD3DX12 constructor to offset to the RTV of the current back buffer.
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(), // handle start
											 m_pSwapChain->GetCurrentBackBufferIndex(), // index to offset
											 m_RtvDescriptorSize); // byte size of descriptor
	}

	void GraphicsDeviceDX12::m_FlushCommandQueue()
	{
		// Advance the fence value to mark commands up to this fence point.
		m_CurrentFence++;
		// Add an instruction to the command queue to set a new fence point.
		// Because we are on the GPU timeline, the new fence point won’t be
		// set until the GPU finishes processing all the commands prior to
		// this Signal().
		ThrowIfFailed(m_pCommandQueue->Signal(m_pFence.Get(), m_CurrentFence));
		// Wait until the GPU has completed commands up to this fence point.
		if (m_pFence->GetCompletedValue() < m_CurrentFence)
		{
			if (HANDLE eventHandle = CreateEventExW(nullptr, false, false, EVENT_ALL_ACCESS); eventHandle)
			{
				// Fire event when GPU hits current fence. 
				ThrowIfFailed(m_pFence->SetEventOnCompletion(m_CurrentFence, eventHandle));
				// Wait until the GPU hits current fence event is fired.
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
			else
				throw std::runtime_error{ u8"Failed to create EventHandle for DX12 Fence" };
		}
	}

	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>> GraphicsDeviceDX12::m_EnumerateAdapters()
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> a;
		std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>> adapters;
		for (UINT i{}; m_pFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&a)) == S_OK; ++i)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter;
			ThrowIfFailed(a.As(&adapter));

			DXGI_ADAPTER_DESC3 desc;
			ThrowIfFailed(adapter->GetDesc3(&desc));

			std::wstring text = desc.Description;
			text += L" Memory: ";
			text += std::to_wstring(desc.DedicatedVideoMemory / 1024 / 1024);
			desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE ? text += L" SOFTWARE ADAPTER" : text += L" PHYSICAL ADAPTER";

			const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), nullptr, 0, nullptr, nullptr);
			std::string result(length, 0);
			WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), result.data(), length, nullptr, nullptr);
			spdlog::info(result);

			adapters.push_back(adapter);
		}

		return adapters;
	}

	std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>> GraphicsDeviceDX12::m_EnumerateAdapterOutputs(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
	{
		std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>> outputs;
		Microsoft::WRL::ComPtr<IDXGIOutput> o;
		for (UINT i{}; adapter->EnumOutputs(i, &o) == S_OK; ++i)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput6> output;
			ThrowIfFailed(o.As(&output));

			DXGI_OUTPUT_DESC1 desc;
			ThrowIfFailed(output->GetDesc1(&desc));

			std::wstring text = L"Output: ";
			text += desc.DeviceName;
			const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), nullptr, 0, nullptr, nullptr);
			std::string result(length, 0);
			WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), result.data(), length, nullptr, nullptr);
			spdlog::info(result);

			outputs.push_back(output);
		}

		return outputs;
	}

	std::vector<DXGI_MODE_DESC1> GraphicsDeviceDX12::m_EnumerateDisplayModes(Microsoft::WRL::ComPtr<IDXGIOutput6> output)
	{
		UINT flags{ DXGI_ENUM_MODES_INTERLACED };
		UINT count{};

		DXGI_FORMAT bbf = DX12_Format(m_BackBufferFormat);

		ThrowIfFailed(output->GetDisplayModeList1(bbf, flags, &count, nullptr));
		std::vector<DXGI_MODE_DESC1> modes(count);
		ThrowIfFailed(output->GetDisplayModeList1(bbf, flags, &count, modes.data()));

		for (const auto& mode : modes)
		{
			std::wstring text = L"Mode: Width = " +
				std::to_wstring(mode.Width) +
				L" " +
				L"Height = " +
				std::to_wstring(mode.Height) +
				L" " +
				L"Refresh = " +
				std::to_wstring(mode.RefreshRate.Numerator) +
				L"/" +
				std::to_wstring(mode.RefreshRate.Denominator);
			const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), nullptr, 0, nullptr, nullptr);
			std::string result(length, 0);
			WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.length()), result.data(), length, nullptr, nullptr);
			spdlog::info(result);
		}

		return modes;
	}

	Microsoft::WRL::ComPtr<IDXGIOutput6> GraphicsDeviceDX12::m_GetOutputFromWindow(HWND hWnd)
	{
		// assuming we want to get output of the window used by swap chain
		if (m_pSwapChain)
		{
			Microsoft::WRL::ComPtr<IDXGIOutput> o;
			ThrowIfFailed(m_pSwapChain->GetContainingOutput(&o));
			Microsoft::WRL::ComPtr<IDXGIOutput6> r;
			ThrowIfFailed(o.As(&r));
			return r;
		}

		HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
		auto adapters = m_EnumerateAdapters();
		for (const auto& adapter : adapters)
		{
			auto outputs = m_EnumerateAdapterOutputs(adapter);
			for (const auto& output : outputs)
			{
				DXGI_OUTPUT_DESC1 desc;
				ThrowIfFailed(output->GetDesc1(&desc));

				if (desc.Monitor == hMon)
					return output;
			}
		}
		
		return {};
	}

	void GraphicsDeviceDX12::m_CreateSwapChain()
	{
		HWND hWnd = std::get<HWND>(*std::static_pointer_cast<std::tuple<HWND, HINSTANCE>, void>(m_pWindow->GetNativeHandle()));
		int n{ 60 }, d{ 1 };

		// Get window dimentions and refresh rate
		{
			auto output = m_GetOutputFromWindow(hWnd);
			assert(output);

			auto [width, height] = m_pWindow->GetSize();

			DXGI_MODE_DESC in;
			in.Width = width;
			in.Height = height;
			in.RefreshRate.Numerator = 0;
			in.RefreshRate.Denominator = 0;
			in.Format = DX12_Format(m_BackBufferFormat);
			in.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			in.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			DXGI_MODE_DESC out;

			// https://docs.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgioutput-findclosestmatchingmode
			ThrowIfFailed(output->FindClosestMatchingMode(&in, &out, nullptr));

			// use new dimentions
			if (m_PresentMode != PresentMode::Windowed)
			{
				m_Width = out.Width;
				m_Height = out.Height;

				m_pWindow->ChangeResolution({ out.Width, out.Height }, true);
			}
			// preserve window dimentions
			else
			{
				m_Width = width;
				m_Height = height;
			}
			// set refresh rate from the closest output mode
			n = out.RefreshRate.Numerator;
			d = out.RefreshRate.Denominator;
		}

		// Release the previous swapchain we will be recreating.
		m_pSwapChain.Reset();

		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_desc1
		DXGI_SWAP_CHAIN_DESC1 sd;
		// 0 means "use window dimentions"
		sd.Width = m_Width;
		sd.Height = m_Height;
		// Note: the only supported formats with DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL are:
		// DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM
		sd.Format = DX12_Format(m_BackBufferFormat);
		sd.Stereo = FALSE;
		// Multisampling is unsupported with DXGI_SWAP_EFFECT_FLIP_*
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = m_BackBufferCount;
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_scaling
		sd.Scaling = DXGI_SCALING_STRETCH;
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_effect
		//sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // TODO: difference?
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_alpha_mode
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// Set swap chain flags
		{
			// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi/ne-dxgi-dxgi_swap_chain_flag
			UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			BOOL tearing{ FALSE };
			if (SUCCEEDED(m_pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing))) && tearing)
			{
				flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				m_TearingSupport = true;
			}
			sd.Flags = flags;
		}

		// https://docs.microsoft.com/ru-ru/windows/win32/api/dxgi1_2/ns-dxgi1_2-dxgi_swap_chain_fullscreen_desc
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsd;
		fsd.RefreshRate.Numerator = n;
		fsd.RefreshRate.Denominator = d;
		fsd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		// https://docs.microsoft.com/ru-ru/previous-versions/windows/desktop/legacy/bb173066(v=vs.85)
		fsd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fsd.Windowed = m_PresentMode != PresentMode::Fullscreen ? TRUE : FALSE;

		// https://docs.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforhwnd
		Microsoft::WRL::ComPtr<IDXGISwapChain1> sc;
		ThrowIfFailed(m_pFactory->CreateSwapChainForHwnd(m_pCommandQueue.Get(), hWnd, &sd, &fsd, nullptr /* TODO: restrict to output?? */, &sc));
		ThrowIfFailed(sc.As(&m_pSwapChain));
	}
}