#include "../sisskey/Engine.h"
#include "../sisskey/Window.h"
#include "../sisskey/GraphicsDevice.h"

#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <memory>

int main(int argc, char** argv)
{
	std::vector<std::string> cmdLine(argc);
	for (int i{}; i < argc; ++i)
		cmdLine[i] = argv[i];

	// Some ideas on interface
	sisskey::Engine engine;
	engine.ParseCmdLine(cmdLine);
	engine.LoadSettings(std::filesystem::current_path() / u8"settings.json");
	engine.Initialize();

	auto w = sisskey::Window::Create();
	auto gd = sisskey::GraphicsDevice::Create(w);

	struct Vertex
	{
		float x, y;
		float r, g, b;
	};
	static_assert(sizeof(Vertex) == 5 * sizeof(float));

	const std::vector<Vertex> vertices{
		{  .0f,  .5f, 1.f, 0.f, 0.f },
		{  .5f, -.5f, .0f, 1.f, 0.f },
		{ -.5f, -.5f, .0f, .0f, 1.f },
	};

	sisskey::Graphics::GPUBufferDesc bd;
	bd.BindFlags = sisskey::Graphics::BIND_FLAG::VERTEX_BUFFER;
	bd.ByteWidth = static_cast<unsigned int>(vertices.size() * sizeof(Vertex));
	sisskey::Graphics::SubresourceData data;
	data.pSysMem = vertices.data();
	sisskey::Graphics::buffer vb = gd->CreateBuffer(bd, data);

	sisskey::Graphics::VertexLayout il;
	sisskey::Graphics::VertexLayoutDesc pos;
	pos.SemanticName = "POSITION";
	pos.Format = sisskey::Graphics::FORMAT::R32G32_FLOAT;
	il.push_back(pos);

	sisskey::Graphics::VertexLayoutDesc col;
	col.SemanticName = "COLOR";
	col.Format = sisskey::Graphics::FORMAT::R32G32B32_FLOAT;
	il.push_back(col);

	sisskey::Graphics::GraphicsPipelineDesc gpd;
	gpd.vs = sisskey::GraphicsDevice::LoadShader(std::filesystem::current_path() / "vert.spv");
	gpd.ps = sisskey::GraphicsDevice::LoadShader(std::filesystem::current_path() / "frag.spv");
	gpd.numRTs = 1;
	gpd.RTFormats[0] = gd->GetBackBufferFormat();
	gpd.InputLayout = std::move(il);

	sisskey::Graphics::DepthStencilStateDesc dss{};
	dss.DepthEnable = true;
	dss.DepthFunc = sisskey::Graphics::COMPARISON_FUNC::LESS;
	dss.DepthWriteMask = sisskey::Graphics::DEPTH_WRITE_MASK::ALL;
	gpd.DSFormat = gd->GetDepthStencilFormat();
	gpd.DepthStencilState = std::move(dss);

	auto p = gd->CreateGraphicsPipeline(gpd);

	while (w->ProcessMessages() != sisskey::Window::PMResult::Quit)
	{
		auto[width, height] = w->GetSize();
		sisskey::Graphics::Viewport vp{ .0f, .0f, static_cast<float>(width), static_cast<float>(height), .0f, 1.f };
		sisskey::Graphics::Rect sr{ 0, 0, width, height };
		gd->Begin();
		gd->BindViewports({ vp });
		gd->BindScissorRects({ sr });
		gd->BindPipeline(p);
		gd->BindVertexBuffers(0, { vb }, { 0 });
		gd->Draw(3, 0);
		gd->End();
	}

	gd->DestroyGraphicsPipeline(p);

	gd->DestroyBuffer(vb);

	return 0;
}