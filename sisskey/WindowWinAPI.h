#pragma once
#include "Window.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cassert>

namespace sisskey
{
	class WindowWinAPI final : public Window
	{
		friend Window;
	private:
		std::wstring m_WndClassName;
		HWND m_hWnd{ nullptr };
		HINSTANCE m_hInstance{ nullptr };
		PMResult m_PMR{ PMResult::Nothing };

		static LRESULT CALLBACK m_StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

	public:
		WindowWinAPI(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor);
		~WindowWinAPI();

		// TODO: constness??

		[[nodiscard]] PMResult ProcessMessages() noexcept final;
		void SetTitle(std::string_view title) final;
		[[nodiscard]] std::string GetTitle() const final;
		void UseSystemCursor(bool use) noexcept final;
		void ChangeResolution(std::pair<int, int> size, bool fullscreen) final;
		[[nodiscard]] std::pair<int, int> GetSize() const noexcept final
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getclientrect
			RECT rc{};
			assert(GetClientRect(m_hWnd, &rc));
			return { rc.right - rc.left, rc.bottom - rc.top };
		}
		[[nodiscard]] std::shared_ptr<void> GetNativeHandle() const final
		{
			return std::make_shared<std::tuple<HWND, HINSTANCE>>(m_hWnd, m_hInstance);
		}
	};
}
