#pragma once

#include <utility>
#include <string>
#include <string_view>
#include <memory>
#include <vector>

namespace sisskey
{
	class Window
	{
	protected:
		Window() = default;
	public:
		enum class PMResult
		{
			Nothing,
			Quit,
			Pause,
			Resume
		};

		virtual ~Window() = default;
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		[[nodiscard]] virtual PMResult ProcessMessages() noexcept = 0;
		virtual void SetTitle(std::string_view title) = 0;
		[[nodiscard]] virtual std::string GetTitle() const = 0;
		virtual void UseSystemCursor(bool use) noexcept = 0;
		virtual void ChangeResolution(std::pair<int, int> size, bool fullscreen) = 0;
		[[nodiscard]] virtual std::pair<int, int> GetSize() const noexcept = 0;
		[[nodiscard]] virtual std::shared_ptr<void> GetNativeHandle() const = 0;

		struct DisplayMode
		{
			std::pair<int, int> dimentions;
			int refresh; // TODO: needed ??

			friend auto operator==(const DisplayMode& l, const DisplayMode& r) noexcept { return l.dimentions == r.dimentions && l.refresh == r.refresh; }
		};
		[[nodiscard]] virtual std::vector<DisplayMode> EnumDisplayModes() const = 0;

		[[nodiscard]] static std::shared_ptr<Window> Create(std::string_view title = u8"sisskey",
															std::pair<int, int> size = { 1280, 720 },
															std::pair<int, int> position = { -1,-1 },
															bool fullscreen = false,
															bool cursor = true);
	};
}