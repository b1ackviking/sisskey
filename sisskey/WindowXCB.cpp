#include "WindowXCB.h"
#include <xcb/xcb_image.h>

#include <stdexcept>
#include <array>
#include <cstdint>
#include <cstring>
#include <cassert>

namespace sisskey
{
	Window::PMResult WindowXCB::ProcessMessages() noexcept
	{
		Window::PMResult res{ Window::PMResult::Nothing };
		xcb_generic_event_t* event;

		while ( (event = xcb_poll_for_event(m_pConnection)) )
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

		ChangeResolution({ width, height }, fullscreen);

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
		constexpr auto WmProtocols = "WM_PROTOCOLS";
		constexpr auto WmDeleteWindow = "WM_DELETE_WINDOW";
		xcb_intern_atom_cookie_t cookie1 = xcb_intern_atom(m_pConnection, 1, static_cast<uint16_t>(strlen(WmProtocols)), WmProtocols);
		xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_pConnection, 0, static_cast<uint16_t>(strlen(WmDeleteWindow)), WmDeleteWindow);
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
		// restore CRTCMode
		if(m_CRTCMode)
			m_SetCRTCMode(m_GetCRTC(), m_CRTCMode);

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

	xcb_randr_crtc_t WindowXCB::m_GetCRTC()
	{
		// assume the window is located on the primary output
		xcb_randr_get_output_primary_cookie_t opc = xcb_randr_get_output_primary_unchecked(m_pConnection, m_Window);
		xcb_randr_get_output_primary_reply_t* opr = xcb_randr_get_output_primary_reply(m_pConnection, opc, nullptr);
		xcb_randr_output_t output = opr->output;
		free(opr);

		xcb_randr_get_output_info_cookie_t omc = xcb_randr_get_output_info(m_pConnection, output, XCB_CURRENT_TIME);
		xcb_randr_get_output_info_reply_t* omr = xcb_randr_get_output_info_reply(m_pConnection, omc, nullptr);
		xcb_randr_crtc_t ret = omr->crtc;
		free(omr);

		return ret;
	}

	xcb_randr_mode_t WindowXCB::m_SetCRTCMode(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode)
	{
		xcb_randr_get_crtc_info_cookie_t cic = xcb_randr_get_crtc_info(m_pConnection, crtc, XCB_CURRENT_TIME);
		xcb_randr_get_crtc_info_reply_t* cir = xcb_randr_get_crtc_info_reply(m_pConnection, cic, nullptr);

		xcb_randr_mode_t ret = cir->mode;

		xcb_randr_output_t* couts = xcb_randr_get_crtc_info_outputs(cir);

		xcb_randr_set_crtc_config_cookie_t sccc = xcb_randr_set_crtc_config(m_pConnection, crtc, XCB_CURRENT_TIME, cir->timestamp, cir->x, cir->y, mode, cir->rotation, cir->num_outputs, couts);
		free(cir);
		xcb_randr_set_crtc_config_reply_t* sccr = xcb_randr_set_crtc_config_reply(m_pConnection, sccc, nullptr);
		free(sccr);

		xcb_flush(m_pConnection);

		return ret;
	}

	void WindowXCB::ChangeResolution(std::pair<int, int> size, bool fullscreen)
	{
		auto [width, height] = size;

		// Documentation:
		// https://standards.freedesktop.org/wm-spec/wm-spec-latest.html

		auto applySizeHints = [this](std::int32_t width, std::int32_t height, bool fullscreen)
		{
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

			// window must be resizeable to switch between normal and fullsceen mode
			const std::int32_t offset = fullscreen ? 1 : 0;
			xcb_size_hints_t hints{};
			hints.flags = XCB_SIZE_US_SIZE_HINT | XCB_SIZE_P_SIZE_HINT | XCB_SIZE_P_MIN_SIZE_HINT | XCB_SIZE_P_MAX_SIZE_HINT | XCB_SIZE_P_WIN_GRAVITY_HINT;
			hints.width = width;
			hints.min_width = width - offset;
			hints.max_width = width + offset;
			hints.height = height;
			hints.min_height = height - offset;
			hints.max_height = height + offset;
			hints.win_gravity = XCB_GRAVITY_CENTER;

			// https://www.x.org/releases/current/doc/man/man3/xcb_change_property.3.xhtml
			xcb_change_property(m_pConnection, XCB_PROP_MODE_REPLACE, m_Window,
								XCB_ATOM_WM_NORMAL_HINTS,
								XCB_ATOM_WM_SIZE_HINTS,
								32, sizeof(hints) / 4, &hints);
		};

		enum class Transition { ToFullscreen, FromFullscreen };
		auto transition = [this](Transition dir)
		{
			constexpr auto netWmState{ "_NET_WM_STATE" };
			constexpr auto netWmStateFullscreen{ "_NET_WM_STATE_FULLSCREEN" };
			constexpr auto netWmStateRemove{ 0u };
			constexpr auto netWmStateAdd{ 1u };
			constexpr auto netWmStateToggle{ 2u };
			xcb_intern_atom_reply_t* reply;
			xcb_atom_t prop;
			xcb_atom_t state;

			xcb_intern_atom_cookie_t wm_state_ck = xcb_intern_atom_unchecked(m_pConnection, 0, static_cast<std::uint16_t>(strlen(netWmState)), netWmState);
			xcb_intern_atom_cookie_t wm_state_fs_ck = xcb_intern_atom_unchecked(m_pConnection, 0, static_cast<std::uint16_t>(strlen(netWmStateFullscreen)), netWmStateFullscreen);
			reply = xcb_intern_atom_reply(m_pConnection, wm_state_ck, nullptr);
			prop = reply->atom;
			free(reply);
			reply = xcb_intern_atom_reply(m_pConnection, wm_state_fs_ck, nullptr);
			state = reply->atom;
			free(reply);

			xcb_client_message_event_t ev;
			std::memset (&ev, 0, sizeof(ev));
			ev.response_type = XCB_CLIENT_MESSAGE;
			ev.type = prop;
			ev.format = 32;
			ev.window = m_Window;
			ev.data.data32[0] = dir == Transition::ToFullscreen ? netWmStateAdd : netWmStateRemove;
			ev.data.data32[1] = state;
			ev.data.data32[2] = XCB_ATOM_NONE;
			ev.data.data32[3] = 0;
			ev.data.data32[4] = 0;

			// xcb_send_event(m_pConnection, 1, m_Window, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)(&ev));
			xcb_send_event(m_pConnection, 1, xcb_setup_roots_iterator(xcb_get_setup(m_pConnection)).data->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (const char*)(&ev));
		};

		if(fullscreen)
		{
			xcb_randr_crtc_t crtc = m_GetCRTC();

			xcb_randr_get_screen_resources_current_cookie_t srcc = xcb_randr_get_screen_resources_current(m_pConnection, m_Window);
			xcb_randr_get_screen_resources_current_reply_t* srcr = xcb_randr_get_screen_resources_current_reply(m_pConnection, srcc, nullptr);
			xcb_randr_mode_info_iterator_t mit = xcb_randr_get_screen_resources_current_modes_iterator(srcr);
			int ml = xcb_randr_get_screen_resources_current_modes_length(srcr);
			free(srcr);

			// store the first mode
			xcb_randr_mode_t m = mit.data->id;
			std::uint16_t w{ mit.data->width }, h{ mit.data->height };

			xcb_randr_mode_t id{};

			for(int i{}; i < ml; xcb_randr_mode_info_next(&mit), ++i)
			{
				if(width == mit.data->width && height == mit.data->height)
				{
					id = mit.data->id;
					break;
				}
			}

			if(!id)
			{
				id = m;
				width = w;
				height = h;
			}

			auto prevmode = m_SetCRTCMode(crtc, id);
			m_CRTCMode = m_Fullscreen ? m_CRTCMode : prevmode;

			applySizeHints(width, height, true);

			if(m_Fullscreen)
				transition(Transition::FromFullscreen);

			transition(Transition::ToFullscreen);
			m_Fullscreen = true;
		}
		else
		{
			if(m_CRTCMode)
				m_SetCRTCMode(m_GetCRTC(), m_CRTCMode);

			if(m_Fullscreen)
				transition(Transition::FromFullscreen);

			applySizeHints(width, height, false);
			m_Fullscreen = false;
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

	std::vector<Window::DisplayMode> WindowXCB::EnumDisplayModes() const
	{
		std::vector<Window::DisplayMode> ret;

		// code adopted from https://gitlab.freedesktop.org/xorg/app/xrandr/tree/master
		// file: xrandr.c commit: 829ed54d89bb37c9e2f8050fe72bd4ecf7b5395a
		// static double mode_refresh(const XRRModeInfo *mode_info) at lines 574-598
		// refresh frequency in Hz
		constexpr auto mode_refresh = [](const xcb_randr_mode_info_t* mode_info) -> double
		{
			double rate;
			double vTotal = mode_info->vtotal;

			// doublescan doubles the number of lines
			if (mode_info->mode_flags & XCB_RANDR_MODE_FLAG_DOUBLE_SCAN)
				vTotal *= 2;

			// interlace splits the frame into two fields
			// the field rate is what is typically reported by monitors
			if (mode_info->mode_flags & XCB_RANDR_MODE_FLAG_INTERLACE)
				vTotal /= 2;

			if (mode_info->htotal && vTotal)
				rate = (static_cast<double>(mode_info->dot_clock) / (static_cast<double>(mode_info->htotal) * static_cast<double>(vTotal)));
			else
				rate = .0;

			return rate;
		};

		xcb_randr_get_screen_resources_current_cookie_t cookie = xcb_randr_get_screen_resources_current_unchecked(m_pConnection, m_Window);
		xcb_randr_get_screen_resources_current_reply_t* reply = xcb_randr_get_screen_resources_current_reply(m_pConnection, cookie, nullptr);

		xcb_randr_mode_info_iterator_t it = xcb_randr_get_screen_resources_current_modes_iterator(reply);
		for(int i{ it.rem }; i > 0; --i)
		{
			ret.push_back({ { it.data->width, it.data->height }, static_cast<int>(mode_refresh(it.data)) });
			xcb_randr_mode_info_next(&it);
		}
		assert(it.rem == 0);

		free(reply);

		// some info on CRTC
		// https://stackoverflow.com/questions/22108822/how-do-i-get-the-resolution-of-randr-outputs-through-the-xcb-randr-extension

		return ret;
	}
}