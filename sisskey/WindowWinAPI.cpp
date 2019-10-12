#include "WindowWinAPI.h"

#include <stdexcept>
#include <array>

namespace sisskey
{
	LRESULT WindowWinAPI::m_StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (message == WM_CREATE)
		{
			// if the message is WM_CREATE, the lParam contains a pointer to a CREATESTRUCT
			// the CREATESTRUCT contains the "this" pointer from the CreateWindow method
			// the "this" pointer of our app is stored in the createstruct pcs->lpCreateParams
			CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCS->lpCreateParams));
		}
		else
		{
			//retrieve the stored "this" pointer
			WindowWinAPI* window = reinterpret_cast<WindowWinAPI*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
			if (window != nullptr)
				return window->WndProc(hWnd, message, wParam, lParam);
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	LRESULT WindowWinAPI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (message)
		{
		case WM_CLOSE: PostQuitMessage(0); return 0;

		case WM_ACTIVATEAPP: (LOWORD(wParam) == WA_INACTIVE) ? m_PMR = Window::PMResult::Pause : m_PMR = Window::PMResult::Resume; return 0;

		default: return DefWindowProcW(hWnd, message, wParam, lParam);
		}
	}

	Window::PMResult WindowWinAPI::ProcessMessages() noexcept
	{
		m_PMR = Window::PMResult::Nothing;
		MSG msg{};
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return Window::PMResult::Quit;

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		return m_PMR;
	}

	WindowWinAPI::WindowWinAPI(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor)
	{
		// Unpack parameters
		auto [width, height] = size;
		auto [x, y] = position;

		m_WndClassName = title.empty() ? L"sisskey" : std::wstring(title.begin(), title.end());
		m_hInstance = GetModuleHandleW(nullptr);

		WNDCLASSEXW wc;
		wc.cbSize = sizeof(wc);
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)6;
		wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
		wc.hIcon = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION));
		wc.hIconSm = wc.hIcon;
		wc.hInstance = m_hInstance;
		wc.lpfnWndProc = m_StaticWndProc;
		wc.lpszClassName = m_WndClassName.c_str();
		wc.lpszMenuName = nullptr;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		if (!RegisterClassExW(&wc))
			throw std::runtime_error{ u8"Failed to register window class" };

		if (fullscreen)
		{
			DEVMODEW dmScreenSettings;
			std::memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = static_cast<DWORD>(width);
			dmScreenSettings.dmPelsHeight = static_cast<DWORD>(height);
			dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettingsW(&dmScreenSettings, CDS_FULLSCREEN);

			m_hWnd = CreateWindowExW(0, m_WndClassName.c_str(), m_WndClassName.c_str(), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_POPUP, 0, 0, width, height, nullptr, nullptr, m_hInstance, this);
		}
		else
		{
			RECT WindowRect{ 0, 0, width, height };
			assert(AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE));
			int WindowWidth = WindowRect.right - WindowRect.left;
			int WindowHeight = WindowRect.bottom - WindowRect.top;

			// Note: GetSystemMetrics returns width and height of the PRIMARY monitor
			int WindowX = x == -1 ? (GetSystemMetrics(SM_CXSCREEN) - WindowWidth) / 2 : x;
			int WindowY = y == -1 ? (GetSystemMetrics(SM_CYSCREEN) - WindowHeight) / 2 : y;

			// NOTE: if we try to create window that client area covers full screen CreateWindowEx silently truncates it to some maximum size
			m_hWnd = CreateWindowExW(0, m_WndClassName.c_str(), m_WndClassName.c_str(), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_OVERLAPPED | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION, WindowX, WindowY, WindowWidth, WindowHeight, nullptr, nullptr, m_hInstance, this);
		}

		if (!m_hWnd)
			throw std::runtime_error{ u8"Failed to create window" };

		ShowWindow(m_hWnd, SW_SHOW);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);
		UseSystemCursor(cursor);
		UpdateWindow(m_hWnd);
	}

	WindowWinAPI::~WindowWinAPI()
	{
		UseSystemCursor(true);
		ChangeDisplaySettingsW(nullptr, 0); // Restore display mode if changed
		DestroyWindow(m_hWnd);
		UnregisterClassW(m_WndClassName.c_str(), m_hInstance);
	}

	void WindowWinAPI::SetTitle(std::string_view title)
	{
		const int length = MultiByteToWideChar(CP_UTF8, 0, title.data(), static_cast<int>(title.length()), nullptr, 0);
		std::wstring t(length, 0);
		MultiByteToWideChar(CP_UTF8, 0, title.data(), static_cast<int>(title.length()), t.data(), length);
		SetWindowTextW(m_hWnd, t.c_str());
	}

	std::string WindowWinAPI::GetTitle() const
	{
		std::array<wchar_t, 128> buffer;
		GetWindowTextW(m_hWnd, buffer.data(), static_cast<int>(buffer.size()));
		std::wstring t{ buffer.data() };

		const int length = WideCharToMultiByte(CP_UTF8, 0, t.data(), static_cast<int>(t.length()), nullptr, 0, nullptr, nullptr);
		std::string result(length, 0);
		WideCharToMultiByte(CP_UTF8, 0, t.data(), static_cast<int>(t.length()), result.data(), length, nullptr, nullptr);
		return result;
	}

	void WindowWinAPI::UseSystemCursor(bool use) noexcept
	{
		if (use) while (ShowCursor(TRUE) < 0);
		else while (ShowCursor(FALSE) >= 0);
	}

	// How do I switch a window between normal and fullscreen?
	// Raymond Chen, 2010
	// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
	void WindowWinAPI::ChangeResolution(std::pair<int, int> size, bool fullscreen)
	{
		auto [width, height] = size;
		DWORD dwStyle = GetWindowLongW(m_hWnd, GWL_STYLE);

		if (fullscreen)
		{
			SetWindowLongW(m_hWnd, GWL_STYLE, dwStyle & ~(WS_OVERLAPPED | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION));

			DEVMODEW dmScreenSettings;
			std::memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = static_cast<DWORD>(width);
			dmScreenSettings.dmPelsHeight = static_cast<DWORD>(height);
			dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			ChangeDisplaySettingsW(&dmScreenSettings, CDS_FULLSCREEN);

			// Set window size to cover full screen
			MONITORINFO mi{ sizeof(mi) };
			GetMonitorInfoW(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			SetWindowPos(m_hWnd, HWND_TOP,
						 mi.rcMonitor.left, mi.rcMonitor.top,
						 mi.rcMonitor.right - mi.rcMonitor.left,
						 mi.rcMonitor.bottom - mi.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
		else
		{
			// Restore display mode
			ChangeDisplaySettingsW(nullptr, 0);
			SetWindowLongW(m_hWnd, GWL_STYLE, (dwStyle | WS_OVERLAPPED | WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION) & ~WS_POPUP);

			// Calculate window rect to be centered on the screen
			MONITORINFO mi{ sizeof(mi) };
			GetMonitorInfoW(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
			int l = mi.rcMonitor.left + (mi.rcMonitor.right - width) / 2;
			int t = mi.rcMonitor.top + (mi.rcMonitor.bottom - height) / 2;
			RECT WindowRect{ l, t, l + width, t + height };
			AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

			WINDOWPLACEMENT WP{ sizeof(WP) };
			WP.ptMinPosition = { -1, -1 };
			WP.ptMaxPosition = { -1, -1 };
			WP.rcNormalPosition = WindowRect;
			WP.showCmd = SW_SHOWNORMAL;
			SetWindowPlacement(m_hWnd, &WP);

			// Probably don't need this call
			// SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

			// Restore icon in titlebar
			SendMessageW(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)IDI_APPLICATION);
		}
	}

	std::vector<Window::DisplayMode> WindowWinAPI::EnumDisplayModes() const
	{
		std::vector<Window::DisplayMode> ret;

		// https://docs.microsoft.com/ru-ru/windows/win32/api/wingdi/ns-wingdi-devmodew
		DEVMODEW devmode;
		std::memset(&devmode, 0, sizeof(devmode));
		devmode.dmSize = sizeof(devmode);

		// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumdisplaysettingsw
		for (DWORD i{}; EnumDisplaySettingsW(nullptr, i, &devmode); ++i)
		{
			assert(devmode.dmDisplayFrequency && devmode.dmDisplayFrequency != 1);
			ret.push_back({ { devmode.dmPelsWidth, devmode.dmPelsHeight }, static_cast<int>(devmode.dmDisplayFrequency) });
			std::memset(&devmode, 0, sizeof(devmode));
			devmode.dmSize = sizeof(devmode);
		}

		ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
		std::reverse(ret.begin(), ret.end());

		return ret;
	}
}