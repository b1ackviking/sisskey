#pragma once

#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN64
#include <Windows.h>
#elif defined(__linux__)
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#endif

namespace sisskey
{
	class Window
	{
	private:
#ifdef _WIN64
		std::wstring m_WndClassName;
		HWND m_hWnd{ nullptr };
		HINSTANCE m_hInstance{ nullptr };

	public:
		[[nodiscard]] HWND GetHWND() const noexcept { return m_hWnd; }
		[[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept { return m_hInstance; }
	private:

		static LRESULT CALLBACK m_StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

#elif defined(__linux__)
		xcb_atom_t m_CloseMessage{ XCB_NONE };
		xcb_cursor_t m_NullCursor{ XCB_NONE };

		xcb_connection_t* m_pConnection{ nullptr };
		xcb_window_t m_Window{};

	public:
		[[nodiscard]] xcb_connection_t* GetXCBConnection() const noexcept { return m_pConnection; }
		[[nodiscard]] xcb_window_t GetXCBWindow() const noexcept { return m_Window; }
	private:
#endif

	public:
		Window(std::string_view title = u8"sisskey", std::pair<int, int> size = { 1280, 720 }, std::pair<int, int> position = { -1,-1 }, bool fullscreen = false, bool cursor = true);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		// TODO: constness??

		// TODO: should this function return enum value
		// for Nothing, Quit, Pause, Resume??
		[[nodiscard]] bool ProcessMessages() noexcept;

		void SetTitle(std::string_view title);
		[[nodiscard]] std::string GetTitle() const;

		void UseSystemCursor(bool use) noexcept;

		void ChangeResolution(std::pair<int, int> size, bool fullscreen);
	};
}