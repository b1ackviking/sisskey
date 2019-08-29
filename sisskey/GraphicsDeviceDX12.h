#pragma once
#include "GraphicsDevice.h"

#ifdef NDEBUG
#define D3D12_IGNORE_SDK_LAYERS
#endif
#include <d3d12.h>
#include <dxgi1_6.h>

namespace sisskey
{
	class GraphicsDeviceDX12 final : public GraphicsDevice
	{
	private:

	public:
		GraphicsDeviceDX12();
		~GraphicsDeviceDX12();
	};
}