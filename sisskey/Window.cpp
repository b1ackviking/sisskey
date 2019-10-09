#include "Window.h"

#ifdef _WIN64
#include "WindowWinAPI.h"
#elif defined(__linux__)
#include "WindowXCB.h"
#endif

#include <tuple>
#include <cassert>

namespace sisskey
{
	std::shared_ptr<Window> Window::Create(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor)
	{
#ifdef _WIN64
		return std::make_shared<WindowWinAPI>(title, size, position, fullscreen, cursor);
#elif defined(__linux__)
		return std::make_shared<WindowXCB>(title, size, position, fullscreen, cursor);
#endif
	}
}