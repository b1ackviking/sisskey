#pragma once

// this file is an edited verison of
// https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiGraphicsDescriptors.h

#include <limits>
#include <cstdint>
#include <type_traits>
#include <vector>

// TODO: types for enums
// TODO: stop using unsigned int
namespace sisskey::Graphics
{
	// http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
	template<typename Enum>
	struct EnableBitMaskOperators
	{
		static constexpr bool enable = false;
	};
	#define ENABLE_BITMASK_OPERATORS(x) template<> struct EnableBitMaskOperators<x> { static const bool enable = true; };

	template<typename Enum>
	constexpr auto operator |(Enum lhs, Enum rhs) -> typename std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
	{
		using underlying = typename std::underlying_type_t<Enum>;
		return static_cast<Enum> (static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
	}

	template<typename Enum>
	constexpr auto operator &(Enum lhs, Enum rhs) -> typename std::enable_if_t<EnableBitMaskOperators<Enum>::enable, Enum>
	{
		using underlying = typename std::underlying_type_t<Enum>;
		return static_cast<Enum> (static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
	}

	using Shader = std::vector<std::byte>;

	struct BlendState;
	struct RasterizerState;
	struct DepthStencilState;

	struct VertexLayout;
	struct Texture;

	enum class SHADERSTAGE
	{
		VS,
		HS,
		DS,
		GS,
		PS,
		CS,
		COUNT
	};
	enum class PRIMITIVE_TOPOLOGY
	{
		UNDEFINED,
		TRIANGLELIST,
		TRIANGLESTRIP,
		POINTLIST,
		LINELIST,
		PATCHLIST,
	};
	enum class COMPARISON_FUNC
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS,
	};
	enum class DEPTH_WRITE_MASK
	{
		ZERO,
		ALL,
	};
	enum class STENCIL_OP
	{
		KEEP,
		ZERO,
		REPLACE,
		INCR_SAT,
		DECR_SAT,
		INVERT,
		INCR,
		DECR,
	};
	enum class BLEND
	{
		ZERO,
		ONE,
		SRC_COLOR,
		INV_SRC_COLOR,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DEST_ALPHA,
		INV_DEST_ALPHA,
		DEST_COLOR,
		INV_DEST_COLOR,
		SRC_ALPHA_SAT,
		BLEND_FACTOR,
		INV_BLEND_FACTOR,
		SRC1_COLOR,
		INV_SRC1_COLOR,
		SRC1_ALPHA,
		INV_SRC1_ALPHA,
	};
	enum class COLOR_WRITE_ENABLE
	{
		DISABLE = 0,
		RED = 1,
		GREEN = 2,
		BLUE = 4,
		ALPHA = 8,
		ALL = (((RED | GREEN) | BLUE) | ALPHA)
	};
	ENABLE_BITMASK_OPERATORS(COLOR_WRITE_ENABLE);
	enum class BLEND_OP
	{
		ADD,
		SUBTRACT,
		REV_SUBTRACT,
		MIN,
		MAX,
	};
	enum class FILL_MODE
	{
		WIREFRAME,
		SOLID,
	};
	enum class CULL_MODE
	{
		NONE,
		FRONT,
		BACK,
	};
	enum class INPUT_CLASSIFICATION
	{
		INPUT_PER_VERTEX_DATA,
		INPUT_PER_INSTANCE_DATA,
	};
	enum class USAGE
	{
		DEFAULT,
		IMMUTABLE,
		DYNAMIC,
		STAGING,
	};
	enum class TEXTURE_ADDRESS_MODE
	{
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE,
	};
	enum class FILTER
	{
		MIN_MAG_MIP_POINT,
		MIN_MAG_POINT_MIP_LINEAR,
		MIN_POINT_MAG_LINEAR_MIP_POINT,
		MIN_POINT_MAG_MIP_LINEAR,
		MIN_LINEAR_MAG_MIP_POINT,
		MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MIN_MAG_LINEAR_MIP_POINT,
		MIN_MAG_MIP_LINEAR,
		ANISOTROPIC,
		COMPARISON_MIN_MAG_MIP_POINT,
		COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_MAG_MIP_LINEAR,
		COMPARISON_ANISOTROPIC,
		MINIMUM_MIN_MAG_MIP_POINT,
		MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_MAG_MIP_LINEAR,
		MINIMUM_ANISOTROPIC,
		MAXIMUM_MIN_MAG_MIP_POINT,
		MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_MAG_MIP_LINEAR,
		MAXIMUM_ANISOTROPIC,
	};
	enum class FORMAT
	{
		UNKNOWN,

		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,

		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,

		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,

		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G8X24_TYPELESS,		// depth + stencil (alias)
		D32_FLOAT_S8X24_UINT,	// depth + stencil

		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,

		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		B8G8R8A8_UNORM,
		B8G8R8A8_UNORM_SRGB,

		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,

		R32_TYPELESS,			// depth (alias)
		D32_FLOAT,				// depth
		R32_FLOAT,
		R32_UINT,
		R32_SINT,
		R24G8_TYPELESS,			// depth + stencil (alias)
		D24_UNORM_S8_UINT,		// depth + stencil

		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,
		R16_TYPELESS,			// depth (alias)
		R16_FLOAT,
		D16_UNORM,				// depth
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,

		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,

		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UF16,
		BC6H_SF16,
		BC7_UNORM,
		BC7_UNORM_SRGB
	};
	enum class GPU_QUERY_TYPE
	{
		EVENT,				// has the GPU reached this point?
		OCCLUSION,			// how many samples passed depthstencil test?
		OCCLUSION_PREDICATE, // are there any samples that passed depthstencil test
		TIMESTAMP,			// retrieve time point of gpu execution
		TIMESTAMP_DISJOINT,	// timestamp frequency information
	};
	enum class INDEXBUFFER_FORMAT
	{
		INDEXFORMAT_16BIT,
		INDEXFORMAT_32BIT,
	};

	// Flags
	enum class CLEAR_FLAG
	{
		CLEAR_DEPTH = 0x1L,
		CLEAR_STENCIL = 0x2L,
	};
	enum class BIND_FLAG
	{
		VERTEX_BUFFER = 0x1L,
		INDEX_BUFFER = 0x2L,
		CONSTANT_BUFFER = 0x4L,
		SHADER_RESOURCE = 0x8L,
		STREAM_OUTPUT = 0x10L,
		RENDER_TARGET = 0x20L,
		DEPTH_STENCIL = 0x40L,
		UNORDERED_ACCESS = 0x80L,
	};
	enum class CPU_ACCESS
	{
		CPU_ACCESS_WRITE = 0x10000L,
		CPU_ACCESS_READ = 0x20000L,
	};
	enum class RESOURCE_MISC_FLAG
	{
		RESOURCE_MISC_SHARED = 0x2L,
		RESOURCE_MISC_TEXTURECUBE = 0x4L,
		RESOURCE_MISC_DRAWINDIRECT_ARGS = 0x10L,
		RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS = 0x20L,
		RESOURCE_MISC_BUFFER_STRUCTURED = 0x40L,
		RESOURCE_MISC_TILED = 0x40000L,
	};
	enum class RESOURCE_STATES
	{
		COMMON = 0,
		VERTEX_AND_CONSTANT_BUFFER = 0x1,
		INDEX_BUFFER = 0x2,
		RENDER_TARGET = 0x4,
		UNORDERED_ACCESS = 0x8,
		DEPTH_WRITE = 0x10,
		DEPTH_READ = 0x20,
		NON_PIXEL_SHADER_RESOURCE = 0x40,
		PIXEL_SHADER_RESOURCE = 0x80,
		STREAM_OUT = 0x100,
		INDIRECT_ARGUMENT = 0x200,
		COPY_DEST = 0x400,
		COPY_SOURCE = 0x800,
		RESOLVE_DEST = 0x1000,
		RESOLVE_SOURCE = 0x2000,
		RAYTRACING_ACCELERATION_STRUCTURE = 0x400000,
		SHADING_RATE_SOURCE = 0x1000000,
		GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		PRESENT = 0,
		PREDICATION = 0x200,
		VIDEO_DECODE_READ = 0x10000,
		VIDEO_DECODE_WRITE = 0x20000,
		VIDEO_PROCESS_READ = 0x40000,
		VIDEO_PROCESS_WRITE = 0x80000,
		VIDEO_ENCODE_READ = 0x200000,
		VIDEO_ENCODE_WIRITE = 0x800000
	};

	// Structs
	struct Viewport
	{
		float TopLeftX{};
		float TopLeftY{};
		float Width{};
		float Height{};
		float MinDepth{};
		float MaxDepth{ 1.f };
	};
	struct VertexLayoutDesc
	{
		static constexpr unsigned int APPEND_ALIGNED_ELEMENT{ 0xffffffff }; // automatically figure out AlignedByteOffset depending on Format

		char* SemanticName{ nullptr };
		unsigned int SemanticIndex{};
		FORMAT Format{ FORMAT::UNKNOWN };
		unsigned int InputSlot{};
		unsigned int AlignedByteOffset{ APPEND_ALIGNED_ELEMENT };
		INPUT_CLASSIFICATION InputSlotClass{ INPUT_CLASSIFICATION::INPUT_PER_VERTEX_DATA };
		unsigned int InstanceDataStepRate{};
	};
	struct SampleDesc
	{
		unsigned int Count{ 1 };
		unsigned int Quality{};
	};
	struct TextureDesc
	{
		unsigned int Width{};
		unsigned int Height{};
		unsigned int Depth{};
		unsigned int ArraySize{ 1 };
		unsigned int MipLevels{ 1 };
		FORMAT Format{ FORMAT::UNKNOWN };
		SampleDesc Sample;
		USAGE Usage{ USAGE::DEFAULT };
		unsigned int BindFlags{};
		unsigned int CPUAccessFlags{};
		unsigned int MiscFlags{};
	};
	struct SamplerDesc
	{
		FILTER Filter{ FILTER::MIN_MAG_MIP_POINT };
		TEXTURE_ADDRESS_MODE AddressU{ TEXTURE_ADDRESS_MODE::CLAMP };
		TEXTURE_ADDRESS_MODE AddressV{ TEXTURE_ADDRESS_MODE::CLAMP };
		TEXTURE_ADDRESS_MODE AddressW{ TEXTURE_ADDRESS_MODE::CLAMP };
		float MipLODBias{};
		unsigned int MaxAnisotropy{};
		COMPARISON_FUNC ComparisonFunc{ COMPARISON_FUNC::NEVER };
		float BorderColor[4]{};
		float MinLOD{};
		float MaxLOD{ std::numeric_limits<float>::max() };
	};
	struct RasterizerStateDesc
	{
		FILL_MODE FillMode{ FILL_MODE::SOLID };
		CULL_MODE CullMode{ CULL_MODE::NONE };
		bool FrontCounterClockwise{};
		int DepthBias{};
		float DepthBiasClamp{};
		float SlopeScaledDepthBias{};
		bool DepthClipEnable{};
		bool MultisampleEnable{};
		bool AntialiasedLineEnable{};
		bool ConservativeRasterizationEnable{};
		unsigned int ForcedSampleCount{};
	};
	struct DepthStencilOpDesc
	{
		STENCIL_OP StencilFailOp{ STENCIL_OP::KEEP };
		STENCIL_OP StencilDepthFailOp{ STENCIL_OP::KEEP };
		STENCIL_OP StencilPassOp{ STENCIL_OP::KEEP };
		COMPARISON_FUNC StencilFunc{ COMPARISON_FUNC::NEVER };
	};
	struct DepthStencilStateDesc
	{
		bool DepthEnable{};
		DEPTH_WRITE_MASK DepthWriteMask{ DEPTH_WRITE_MASK::ZERO };
		COMPARISON_FUNC DepthFunc{ COMPARISON_FUNC::NEVER };
		bool StencilEnable{};
		std::uint8_t StencilReadMask{ 0xff };
		std::uint8_t StencilWriteMask{ 0xff };
		DepthStencilOpDesc FrontFace;
		DepthStencilOpDesc BackFace;
	};
	struct RenderTargetBlendStateDesc
	{
		bool BlendEnable{};
		BLEND SrcBlend{ BLEND::SRC_ALPHA };
		BLEND DestBlend{ BLEND::INV_SRC_ALPHA };
		BLEND_OP BlendOp{ BLEND_OP::ADD };
		BLEND SrcBlendAlpha{ BLEND::ONE };
		BLEND DestBlendAlpha{ BLEND::ONE };
		BLEND_OP BlendOpAlpha{ BLEND_OP::ADD };
		COLOR_WRITE_ENABLE RenderTargetWriteMask{ COLOR_WRITE_ENABLE::ALL };
	};
	struct BlendStateDesc
	{
		bool AlphaToCoverageEnable{};
		bool IndependentBlendEnable{};
		RenderTargetBlendStateDesc RenderTarget[8];
	};
	struct GPUBufferDesc
	{
		unsigned int ByteWidth{};
		USAGE Usage{ USAGE::DEFAULT };
		unsigned int BindFlags{};
		unsigned int CPUAccessFlags{};
		unsigned int MiscFlags{};
		unsigned int StructureByteStride{}; // needed for typed and structured buffer types!
		FORMAT Format{ FORMAT::UNKNOWN }; // only needed for typed buffer!
	};
	struct GPUQueryDesc
	{
		GPU_QUERY_TYPE Type{ GPU_QUERY_TYPE::EVENT };
	};
	struct GPUQueryResult
	{
		int result_passed{};
		std::uint64_t result_passed_sample_count{};
		std::uint64_t result_timestamp{};
		std::uint64_t result_timestamp_frequency{};
		int result_disjoint{};
	};
	struct GraphicsPipelineDesc
	{
		const Shader* vs{ nullptr };
		const Shader* hs{ nullptr };
		const Shader* ds{ nullptr };
		const Shader* gs{ nullptr };
		const Shader* ps{ nullptr };
		const BlendState* bs{ nullptr };
		const RasterizerState* rs{ nullptr };
		const DepthStencilState* dss{ nullptr };
		const VertexLayout* il{ nullptr };
		PRIMITIVE_TOPOLOGY pt{ PRIMITIVE_TOPOLOGY::TRIANGLELIST };
		unsigned int numRTs{};
		FORMAT RTFormats[8]{};
		FORMAT DSFormat{ FORMAT::UNKNOWN };
		SampleDesc sampleDesc;
		unsigned int sampleMask{ 0xFFFFFFFF };
	};
	struct IndirectDrawArgsInstanced
	{
		unsigned int VertexCountPerInstance{};
		unsigned int InstanceCount{};
		unsigned int StartVertexLocation{};
		unsigned int StartInstanceLocation{};
	};
	struct IndirectDrawArgsIndexedInstanced
	{
		unsigned int IndexCountPerInstance{};
		unsigned int InstanceCount{};
		unsigned int StartIndexLocation{};
		int BaseVertexLocation{};
		unsigned int StartInstanceLocation{};
	};
	struct IndirectDispatchArgs
	{
		unsigned int ThreadGroupCountX{};
		unsigned int ThreadGroupCountY{};
		unsigned int ThreadGroupCountZ{};
	};
	struct SubresourceData
	{
		const void* pSysMem{ nullptr };
		unsigned int SysMemPitch{};
		unsigned int SysMemSlicePitch{};
	};
	struct Rect
	{
		long left{};
		long top{};
		long right{};
		long bottom{};
	};
}