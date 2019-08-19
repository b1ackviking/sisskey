#include "../sisskey/Engine.h"
#include "../sisskey/Window.h"

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

	auto w = std::make_unique<sisskey::Window>();
	while (w->ProcessMessages() != sisskey::Window::PMResult::Quit);

	return 0;
}