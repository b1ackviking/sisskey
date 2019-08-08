#include "../sisskey/Engine.h"

#include <string>
#include <sstream>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#ifndef NDEBUG
#include <crtdbg.h>

int main(int argc, char** argv)
{
	std::string cmdLine{};

	for (int i{}; i < argc; ++i)
		cmdLine += argv[i], cmdLine += ' ';
	cmdLine[cmdLine.length() - 1] = '\0';

	HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(0);

	SetConsoleOutputCP(CP_UTF8); // Change codepage to UTF-8 (65001)
	setvbuf(stdout, nullptr, _IOFBF, 1000); // MAGIC - DO NOT TOUCH

	return WinMain(GetModuleHandleW(nullptr), nullptr, cmdLine.data(), 0);
}
#endif // !NDEBUG

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ int)
{
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

	sisskey::Engine::ParseCmdLine(cmdLine);
	
	return 0;
}