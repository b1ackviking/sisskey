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
	[[nodiscard]] std::shared_ptr<Window> Window::Create(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor)
	{
#ifdef _WIN64
		return std::make_shared<WindowWinAPI>(title, size, position, fullscreen, cursor);
#elif defined(__linux__)
		return std::make_shared<WindowXCB>(title, size, position, fullscreen, cursor);
#endif
	}

	[[nodiscard]] std::shared_ptr<void> Window::GetNativeHandle(Window* window)
	{
		assert(window);
#ifdef _WIN64
		WindowWinAPI* w = static_cast<WindowWinAPI*>(window);
		return std::make_shared<std::tuple<HWND, HINSTANCE>>(w->m_hWnd, w->m_hInstance);
#elif defined(__linux__)
		WindowXCB* w = static_cast<WindowXCB*>(window);
		return std::make_shared<std::tuple<xcb_connection_t*, xcb_window_t>>(w->m_pConnection, w->m_Window);
#endif
	}
}