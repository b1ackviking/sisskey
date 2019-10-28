#include "../sisskey/Engine.h"
#include "../sisskey/Window.h"
#include "../sisskey/GraphicsDevice.h"

#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// set linker flags for visual studio here because
// it's impossible to do this with CMakeLists.txt
// conditionally on a build type
// TODO: clang-cl ???
#ifdef _MSC_VER
#ifndef NDEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#endif
#endif

#ifndef NDEBUG
#include <crtdbg.h>

int main(int argc, char** argv)
{
	// Concat all arguments into a string to pass further
	std::string cmdLine{};
	for (int i{}; i < argc; ++i)
		cmdLine += argv[i], cmdLine += ' ';
	cmdLine[cmdLine.length() - 1] = '\0';

	// Helps to detect memory leaks
	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0);

	// Change codepage to UTF-8 (65001)
	SetConsoleOutputCP(CP_UTF8);

	// Enable full buffering with 1k buffer size
	// to correcty output UTF-8 in Windows console
	// Note: requiers explicit flush to output smaller portions of output
	setvbuf(stdout, nullptr, _IOFBF, 1024);

	return WinMain(GetModuleHandleW(nullptr), nullptr, cmdLine.data(), 0);
}
#endif // !NDEBUG

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int)
{
	// Extract arguments from a string
	std::vector<std::string> cmdLine;
	{
		std::istringstream ss{ lpCmdLine };
		do
		{
			std::string s;
			ss >> s;
			cmdLine.push_back(std::move(s));
		} while (ss);
	}

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
		sisskey::Graphics::Viewport vp{ 0, 0, width, height, .0f, 1.f };
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