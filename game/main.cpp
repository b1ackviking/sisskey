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

	sisskey::Graphics::GraphicsPipelineDesc gpd;
	gpd.vs = sisskey::GraphicsDevice::LoadShader(std::filesystem::current_path() / "vert.spv");
	gpd.ps = sisskey::GraphicsDevice::LoadShader(std::filesystem::current_path() / "frag.spv");
	gpd.numRTs = 1;
	gpd.RTFormats[0] = gd->GetBackBufferFormat();

	sisskey::Graphics::DepthStencilStateDesc dss{};
	dss.DepthEnable = true;
	dss.DepthFunc = sisskey::Graphics::COMPARISON_FUNC::LESS;
	dss.DepthWriteMask = sisskey::Graphics::DEPTH_WRITE_MASK::ALL;
	gpd.DSFormat = gd->GetDepthStencilFormat();
	gpd.DepthStencilState = std::move(dss);

	auto p = gd->CreateGraphicsPipeline(gpd);

	while (w->ProcessMessages() != sisskey::Window::PMResult::Quit) gd->Render(p);

	gd->DestroyGraphicsPipeline(p);

	return 0;
}