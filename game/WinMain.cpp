#include "../sisskey/Engine.h"
#include "../sisskey/Window.h"

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
	while (w->ProcessMessages() != sisskey::Window::PMResult::Quit);
	
	return 0;
}