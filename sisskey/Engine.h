#pragma once
#include <vector>
#include <string>

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

		static void ParseCmdLine(std::vector<std::string>& args);
	};

}