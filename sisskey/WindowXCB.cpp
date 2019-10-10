#include "WindowXCB.h"

#include <stdexcept>
#include <array>
#include <cstdint>
#include <cstring>
#include <xcb/xcb_image.h>

namespace sisskey
{
	Window::PMResult WindowXCB::ProcessMessages() noexcept
	{
		Window::PMResult res{ Window::PMResult::Nothing };
		xcb_generic_event_t* event;

		while (event = xcb_poll_for_event(m_pConnection))
		{
			// https://xcb.freedesktop.org/manual/xproto_8h_source.html
			switch (event->response_type & ~0x80)
			{
			case XCB_EXPOSE:
			{
				//xcb_expose_event_t* expose = reinterpret_cast<xcb_expose_event_t*>(event);
				res = Window::PMResult::Resume;
			} break;
			case XCB_MOTION_NOTIFY:
			{
				//xcb_motion_notify_event_t* motion = reinterpret_cast<xcb_motion_notify_event_t*>(event);

			} break;
			case XCB_BUTTON_PRESS:
			{
				//xcb_button_press_event_t* press = reinterpret_cast<xcb_button_press_event_t*>(event);

			} break;
			case XCB_BUTTON_RELEASE:
			{
				//xcb_button_release_event_t* release = reinterpret_cast<xcb_button_press_event_t*>(event);

			} break;
			case XCB_KEY_PRESS:
			{
				//xcb_key_press_event_t* press = reinterpret_cast<xcb_key_press_event_t*>(event);

			} break;
			case XCB_KEY_RELEASE:
			{
				//xcb_key_release_event_t* release = reinterpret_cast<xcb_key_release_event_t*>(event);

			} break;
			case XCB_FOCUS_IN:
			{
				//xcb_focus_in_event_t* focus_in = reinterpret_cast<xcb_focus_in_event_t*>(event);
				res = Window::PMResult::Resume;
			} break;
			case XCB_FOCUS_OUT:
			{
				//xcb_focus_out_event_t* focus_out = reinterpret_cast<xcb_focus_out_event_t*>(event);
				res = Window::PMResult::Pause;
			} break;
			case XCB_CLIENT_MESSAGE:
			{
				xcb_client_message_event_t* message = reinterpret_cast<xcb_client_message_event_t*>(event);
				if (message->data.data32[0] == m_CloseMessage)
					res = Window::PMResult::Quit;
			} break;
			}

			free(event);
		}

		return res;
	}

	WindowXCB::WindowXCB(std::string_view title, std::pair<int, int> size, std::pair<int, int> position, bool fullscreen, bool cursor)
	{
		// Unpack parameters
		auto [width, height] = size;
		auto [x, y] = position;

		// https://xcb.freedesktop.org/tutorial/
		// https://www.x.org/releases/X11R7.5/doc/libxcb/tutorial/
		// https://xcb.freedesktop.org/manual/group__XCB____API.html

		m_pConnection = xcb_connect(nullptr, nullptr);
		if (xcb_connection_has_error(m_pConnection))
			throw std::runtime_error{ u8"Failed to connect to X Server" };

		xcb_screen_t* pScreen = xcb_setup_roots_iterator(xcb_get_setup(m_pConnection)).data;

		// https://github.com/Medium/phantomjs-1/blob/master/src/qt/qtbase/src/plugins/platforms/xcb/qxcbcursor.cpp
		std::uint8_t cur_blank_bits[]{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

		xcb_pixmap_t cp = xcb_create_pixmap_from_bitmap_data(m_pConnection, pScreen->root,
															 cur_blank_bits, 16, 16,
															 1, 0, 0, nullptr);
		xcb_pixmap_t mp = xcb_create_pixmap_from_bitmap_data(m_pConnection, pScreen->root,
															 cur_blank_bits, 16, 16,
															 1, 0, 0, nullptr);

		m_NullCursor = xcb_generate_id(m_pConnection);
		xcb_create_cursor(m_pConnection, m_NullCursor, cp, mp, 0, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF, 8, 8);
		xcb_free_pixmap(m_pConnection, cp);
		xcb_free_pixmap(m_pConnection, mp);

		/* IMPORTANT: values must be in this order:
		typedef enum {
			XCB_CW_BACK_PIXMAP       = 1L<<0,
			XCB_CW_BACK_PIXEL        = 1L<<1,
			XCB_CW_BORDER_PIXMAP     = 1L<<2,
			XCB_CW_BORDER_PIXEL      = 1L<<3,
			XCB_CW_BIT_GRAVITY       = 1L<<4,
			XCB_CW_WIN_GRAVITY       = 1L<<5,
			XCB_CW_BACKING_STORE     = 1L<<6,
			XCB_CW_BACKING_PLANES    = 1L<<7,
			XCB_CW_BACKING_PIXEL     = 1L<<8,
			XCB_CW_OVERRIDE_REDIRECT = 1L<<9,
			XCB_CW_SAVE_UNDER        = 1L<<10,
			XCB_CW_EVENT_MASK        = 1L<<11,
			XCB_CW_DONT_PROPAGATE    = 1L<<12,
			XCB_CW_COLORMAP          = 1L<<13,
			XCB_CW_CURSOR            = 1L<<14
		} xcb_cw_t;
		*/
		std::uint32_t mask{ XCB_CW_BACK_PIXMAP | XCB_CW_BORDER_PIXEL | XCB_CW_BIT_GRAVITY | XCB_CW_BACKING_STORE | XCB_CW_EVENT_MASK | XCB_CW_CURSOR };
		std::uint32_t values[]{ XCB_NONE,
								0,
								XCB_GRAVITY_BIT_FORGET,
								XCB_BACKING_STORE_NOT_USEFUL,
								XCB_EVENT_MASK_EXPOSURE |
								XCB_EVENT_MASK_POINTER_MOTION |// XCB_EVENT_MASK_BUTTON_MOTION | ??
								XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |// ??
								XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | // mouse
								XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |// keyboard
								XCB_EVENT_MASK_FOCUS_CHANGE,
								cursor ? XCB_CURSOR_NONE : m_NullCursor
		};

		m_Window = xcb_generate_id(m_pConnection);

		xcb_create_window(m_pConnection,
						  XCB_COPY_FROM_PARENT, // depth
						  m_Window,
						  pScreen->root, // parent
						  // (-1,-1) seems to automatically place window at the center of the screen
						  static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y),
						  static_cast<std::uint16_t>(width), static_cast<std::uint16_t>(height),
						  0, // border
						  XCB_WINDOW_CLASS_INPUT_OUTPUT,
						  pScreen->root_visual,
						  mask, values);

		ChangeResolution({ width,height }, fullscreen);


		// https://www.x.org/releases/current/doc/man/man3/xcb_change_property.3.xhtml
		xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window,
							XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, static_cast<std::uint32_t>(title.size()), title.data());

		/* set the title of the window icon */

		// char *iconTitle = "Hello World ! (iconified)";
		// xcb_change_property (connection,
		//                      XCB_PROP_MODE_REPLACE,
		//                      window,
		//                      XCB_ATOM_WM_ICON_NAME,
		//                      XCB_ATOM_STRING,
		//                      8,
		//                      strlen(iconTitle),
		//                      iconTitle);

		// https://stackoverflow.com/questions/8776300/how-to-exit-program-with-close-button-in-xcb
		// https://marc.info/?l=freedesktop-xcb&m=129381953404497
		// https://tronche.com/gui/x/icccm/sec-4.html#s-4.2.8.1
		xcb_intern_atom_cookie_t cookie1 = xcb_intern_atom(m_pConnection, 1, static_cast<uint16_t>(strlen("WM_PROTOCOLS")), "WM_PROTOCOLS");
		xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_pConnection, 0, static_cast<uint16_t>(strlen("WM_DELETE_WINDOW")), "WM_DELETE_WINDOW");
		xcb_intern_atom_reply_t* reply1 = xcb_intern_atom_reply(m_pConnection, cookie1, nullptr);
		xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(m_pConnection, cookie2, nullptr);
		xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window, reply1->atom, 4, 32, 1, &reply2->atom);
		m_CloseMessage = reply2->atom;

		free(reply1);
		free(reply2);

		xcb_map_window(m_pConnection, m_Window);

		xcb_flush(m_pConnection);
	}

	WindowXCB::~WindowXCB()
	{
		UseSystemCursor(true);
		if (m_NullCursor)
			xcb_free_cursor(m_pConnection, m_NullCursor);
		xcb_destroy_window(m_pConnection, m_Window);
		xcb_disconnect(m_pConnection);
	}

	void WindowXCB::SetTitle(std::string_view title)
	{
		xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window,
							XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, static_cast<std::uint32_t>(title.size()), title.data());
	}

	std::string WindowXCB::GetTitle() const
	{
		std::string result{};
		xcb_get_property_cookie_t cookie = xcb_get_property(m_pConnection, 0, m_Window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, 100);
		xcb_get_property_reply_t* reply = xcb_get_property_reply(m_pConnection, cookie, nullptr);
		if (reply)
		{
			result = (char*)xcb_get_property_value(reply);
			free(reply);
		}

		return result;
	}

	void WindowXCB::UseSystemCursor(bool use) noexcept
	{
		if (use)
		{
			std::uint32_t value{ XCB_CURSOR_NONE };
			xcb_change_window_attributes(m_pConnection, m_Window, XCB_CW_CURSOR, &value);
		}
		else xcb_change_window_attributes(m_pConnection, m_Window, XCB_CW_CURSOR, &m_NullCursor);
	}

	void WindowXCB::ChangeResolution(std::pair<int, int> size, bool fullscreen)
	{
		auto [width, height] = size;

		// to remove decorations
		// https://stackoverflow.com/questions/28366896/how-to-remove-window-decorations-with-xcb

		// to disable actions
		// https://stackoverflow.com/questions/14442081/disable-actions-move-resize-minimize-etc-using-python-xlib/38175137#38175137
		// https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html#id2523223

		// TODO: 1) when fullscreen: use xcb/randr to determine/set output mode
		// xrandr source: https://gitlab.freedesktop.org/xorg/app/xrandr
		// randr docs: https://xcb.freedesktop.org/manual/group__XCB__RandR__API.html

		// TODO: 2) fullscreen -> window transition
		// _NET_WM_STATE magic: https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html#id2523223

		enum
		{
			XCB_SIZE_US_POSITION_HINT = 1 << 0,
			XCB_SIZE_US_SIZE_HINT = 1 << 1,
			XCB_SIZE_P_POSITION_HINT = 1 << 2,
			XCB_SIZE_P_SIZE_HINT = 1 << 3,
			XCB_SIZE_P_MIN_SIZE_HINT = 1 << 4,
			XCB_SIZE_P_MAX_SIZE_HINT = 1 << 5,
			XCB_SIZE_P_RESIZE_INC_HINT = 1 << 6,
			XCB_SIZE_P_ASPECT_HINT = 1 << 7,
			XCB_SIZE_BASE_SIZE_HINT = 1 << 8,
			XCB_SIZE_P_WIN_GRAVITY_HINT = 1 << 9
		};

		struct xcb_size_hints_t
		{
			uint32_t flags;
			int32_t  x, y, width, height;
			int32_t  min_width, min_height;
			int32_t  max_width, max_height;
			int32_t  width_inc, height_inc;
			int32_t  min_aspect_num, min_aspect_den;
			int32_t  max_aspect_num, max_aspect_den;
			int32_t  base_width, base_height;
			uint32_t win_gravity;
		};

		xcb_size_hints_t hints{};
		hints.flags = XCB_SIZE_US_SIZE_HINT | XCB_SIZE_P_SIZE_HINT | XCB_SIZE_P_MIN_SIZE_HINT | XCB_SIZE_P_MAX_SIZE_HINT;
		hints.min_width = width;
		hints.max_width = width;
		hints.min_height = height;
		hints.max_height = height;
		hints.win_gravity = XCB_GRAVITY_CENTER;

		// https://www.x.org/releases/current/doc/man/man3/xcb_change_property.3.xhtml
		xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window,
							XCB_ATOM_WM_NORMAL_HINTS,
							XCB_ATOM_WM_SIZE_HINTS,
							32, sizeof(hints) / 4, &hints);

		if (fullscreen)
		{
			xcb_intern_atom_cookie_t cookie1;
			xcb_intern_atom_cookie_t cookie2;
			xcb_intern_atom_reply_t* reply;
			xcb_atom_t prop;
			xcb_atom_t state;
			static constexpr auto netWmState = "_NET_WM_STATE";
			static constexpr auto netWmStateFullscreen = "_NET_WM_STATE_FULLSCREEN";

			cookie1 = xcb_intern_atom_unchecked(m_pConnection, 0, static_cast<std::uint16_t>(strlen(netWmState)), netWmState);
			cookie2 = xcb_intern_atom_unchecked(m_pConnection, 0, static_cast<std::uint16_t>(strlen(netWmStateFullscreen)), netWmStateFullscreen);;
			reply = xcb_intern_atom_reply(m_pConnection, cookie1, nullptr);
			prop = reply->atom;
			free(reply);
			reply = xcb_intern_atom_reply(m_pConnection, cookie2, nullptr);
			state = reply->atom;
			free(reply);
			xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window, prop, XCB_ATOM_ATOM, 32, 1, (const void*)& state);
		}

		xcb_flush(m_pConnection);
	}

	std::pair<int, int> WindowXCB::GetSize() const noexcept
	{
		xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry(m_pConnection, m_Window);
		xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(m_pConnection, geomCookie, nullptr);

		auto ret = std::make_pair(geom->width, geom->height);

		free(geom);

		return ret;
	}
}