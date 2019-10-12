#pragma once
#include "Window.h"

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/randr.h>

namespace sisskey
{
	class WindowXCB final : public Window
	{
		friend Window;
	private:
		xcb_atom_t m_CloseMessage{ XCB_NONE };
		xcb_cursor_t m_NullCursor{ XCB_NONE };

		xcb_connection_t* m_pConnection{ nullptr };
		xcb_window_t m_Window{};

		bool m_Fullscreen{ false };
		xcb_randr_mode_t m_CRTCMode{};

		// returns CRTC of a primary output
		xcb_randr_crtc_t m_GetCRTC();
		// returns previous CRTC mode
		xcb_randr_mode_t m_SetCRTCMode(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode);

	public:
		WindowXCB(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor);
		~WindowXCB();

		[[nodiscard]] PMResult ProcessMessages() noexcept final;
		void SetTitle(std::string_view title) final;
		[[nodiscard]] std::string GetTitle() const final;
		void UseSystemCursor(bool use) noexcept final;
		void ChangeResolution(std::pair<int, int> size, bool fullscreen) final;
		[[nodiscard]] std::pair<int, int> GetSize() const noexcept final;
		[[nodiscard]] std::shared_ptr<void> GetNativeHandle() const final
		{
			return std::make_shared<std::tuple<xcb_connection_t*, xcb_window_t>>(m_pConnection, m_Window);
		}

		[[nodiscard]] std::vector<DisplayMode> EnumDisplayModes() const final;
	};
}
