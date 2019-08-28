#pragma once
#include "Window.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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

		[[nodiscard]] PMResult ProcessMessages() noexcept override;
		void SetTitle(std::string_view title) override;
		[[nodiscard]] std::string GetTitle() const override;
		void UseSystemCursor(bool use) noexcept override;
		void ChangeResolution(std::pair<int, int> size, bool fullscreen) override;
	};
}
