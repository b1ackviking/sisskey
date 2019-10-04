#include "GraphicsDeviceDX12.h"
#include "Window.h"

#include "d3dx12.h"
#include <DirectXColors.h>

#include <stdexcept>
#include <array>
#include <string>
#include <cassert>

namespace sisskey
{
	namespace Graphics
	{
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

		// TODO: log
		// Fallback to WARP adapter
		if (!ret)
		{
			Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
			ThrowIfFailed(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
			ThrowIfFailed(pWarpAdapter.As(&ret));
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
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)));
		ThrowIfFailed(m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pDirectCmdListAlloc)));
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cl;
			ThrowIfFailed(m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pDirectCmdListAlloc.Get(), // Associated command allocator
													   nullptr, // Initial PipelineStateObject
													   IID_PPV_ARGS(&cl)));
			ThrowIfFailed(cl.As(&m_pCommandList));
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
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = m_DepthStencilFormat;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		
		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_DepthStencilFormat;
		// TODO: Reverse Z projection
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		{
			Microsoft::WRL::ComPtr<ID3D12Resource> dsb;
			ThrowIfFailed(m_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_COMMON, &optClear, IID_PPV_ARGS(&dsb)));
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
	}

	void GraphicsDeviceDX12::Render()
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
		// Set the viewport and scissor rect. This needs to be reset
		// whenever the command list is reset.
		m_pCommandList->RSSetViewports(1, &m_ViewPort);
		m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);
		// Clear the back buffer and depth buffer.
		m_pCommandList->ClearRenderTargetView(m_CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
		m_pCommandList->ClearDepthStencilView(m_DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		// Specify the buffers we are going to render to.
		m_pCommandList->OMSetRenderTargets(1, &m_CurrentBackBufferView(), true, &m_DepthStencilView());
		// Indicate a state transition on the resource usage.
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		
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

			// TODO: log
			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L" Memory: ";
			text += std::to_wstring(desc.DedicatedVideoMemory / 1024 / 1024);
			desc.Flags& DXGI_ADAPTER_FLAG3_SOFTWARE ? text += L" SOFTWARE ADAPTER!" : text += L" PHYSICAL ADAPTER";
			text += L"\n";
			OutputDebugStringW(text.c_str());

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

			// TODO: log
			std::wstring text = L"***Output: ";
			text += desc.DeviceName;
			text += L"\n";
			OutputDebugStringW(text.c_str());

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

		// TODO: log
		for (const auto& mode : modes)
		{
			std::wstring text = L"Width = " +
				std::to_wstring(mode.Width) +
				L" " +
				L"Height = " +
				std::to_wstring(mode.Height) +
				L" " +
				L"Refresh = " +
				std::to_wstring(mode.RefreshRate.Numerator) +
				L"/" +
				std::to_wstring(mode.RefreshRate.Denominator) +
				L"\n";
			OutputDebugStringW(text.c_str());
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
		HWND hWnd = std::get<0>(*std::static_pointer_cast<std::tuple<HWND, HINSTANCE>, void>(Window::GetNativeHandle(m_pWindow.get())));
		int n{ 60 }, d{ 1 };

		// Get window dimentions and refresh rate
		{
			auto output = m_GetOutputFromWindow(hWnd);
			assert(output);

			// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
			RECT r;
			GetClientRect(hWnd, &r);

			DXGI_MODE_DESC in;
			in.Width = r.right;
			in.Height = r.bottom;
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
				m_Width = r.right;
				m_Height = r.bottom;
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