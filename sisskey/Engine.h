#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace sisskey
{
	class Engine
	{
	private:

	public:
		Engine() = default;
		~Engine() = default;
		Engine(Engine&&) = default;
		Engine& operator=(Engine&&) = default;
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void ParseCmdLine(std::vector<std::string>& args);
		void LoadSettings(std::filesystem::path settings);

		void Initialize();
	};
}