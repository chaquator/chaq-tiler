#include <windows.h>
#include <Shobjidl.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <bitset>
#include <iterator>

#include <cassert>

#include <algorithm>
#include <iostream>

// TODO: pull code out into different places

// Usings and typedefs
using namespace std::literals;

// Structs and classes

struct Rule {
	enum class SingleAction {
		None,
		Unmaximize,
		Maximize
	};

	// Filtering

	// Class name -- empty string -> catch all
	std::wstring_view ClassName;
	// Title name -- empty string -> catch all
	std::wstring_view TitleName;
	// Style flags -- Or them together.
	// See https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles
	LONG Style;
	// Extended style flags -- Or them together.
	//See https://docs.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
	LONG ExStyle;

	// Decisions

	// Manage -- should the prorgam manage windows in this rule?
	// NOTE: if false, attributes below will obviously not apply. If you want them to apply while
	// also keeping the window "unmanaged", set Manage to true and Floating to false
	bool Manage;

	// Attributes

	// Floating -- true -> window should NOT be moved when arranged
	bool Floating;
	// TagMask -- 10-bit bitstring representing which tags to place the window in when first managed
	// All 0s -> no particular tags applied when window is first managed
	// NOTE: 0th bit is intended to represent key 1 on the number row, and so on
	std::bitset<10> TagMask;
	// Action -- any specific action applied to the window when first managed
	SingleAction Action;
};

struct Vec {
	using vec_t = std::int32_t;
	vec_t x;
	vec_t y;
};
inline Vec operator +(Vec a, Vec b) {
	return { a.x + b.x, a.y + b.y };
}
inline Vec operator -(Vec a, Vec b) {
	return { a.x - b.x, a.y - b.y };
}
template <typename Scalar>
inline static Vec mul(Scalar scalar, Vec p) {
	return {
		static_cast<Vec::vec_t>(scalar * static_cast<Scalar>(p.x)),
		static_cast<Vec::vec_t>(scalar * static_cast<Scalar>(p.y))
	};
}
template <typename Scalar>
inline Vec operator *(Scalar a, Vec b) {
	return mul(a, b);
}
template <typename Scalar>
inline Vec operator *(Vec b, Scalar a) {
	return mul(a, b);
}

// TODO: decide when to calculate this (probably in the beginning) and when to update (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for orientation and position of it too)
struct Desktop {
	int margin; // Space between each window (and from the monitor edge to the window)
	Vec monitor_upper_left;
	Vec monitor_dimensions;
};

struct Window {
	HWND handle;
	bool floating;
	std::bitset<10> current_tags;
	Rule::SingleAction action;

	std::wstring title;
	std::wstring class_name;

	Window(HWND handle, std::wstring_view& title, std::wstring_view& class_name):
		handle(handle), floating(false), current_tags(0), action(Rule::SingleAction::None),
		title(title), class_name(class_name) {
	}
};

// Globals

namespace Globals {
	std::vector<Window> Windows;
	HMONITOR PrimaryMonitor;
	Desktop PrimaryDesktop;
}

// Function definitions

// Debug utilities
void PrintDebugString(std::string_view&& text);

// Views
template <typename Container>
void cascade(const Container& windows, const Desktop& desktop);

static Window ApplyRules(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);
static bool ShouldManageWindow(HWND);
static BOOL CALLBACK ScanWindows(HWND, LPARAM);
static HMONITOR GetPrimaryMonitorHandle();

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

constexpr static std::size_t buflen = 128;

// Including config
#include "config.h"

void PrintDebugString(std::string_view&& text) {
	std::cout << "DEBUG: " << text << std::endl;
}
#ifdef NDEBUG
#define debug(s) ((void)0);
#else
#define debug(s) PrintDebugString(s);
#endif

template <typename Container>
void cascade(const Container& windows, const Desktop& desktop) {
	// TODO: floating windows

	using SizeType = typename Container::size_type;
	using DiffType = typename Container::difference_type;
	using Iterator = typename Container::const_iterator;

	if (windows.empty()) return; // Short circuit break

	// Customize cascade here
	constexpr float window_factor = 0.35f; // Window will be 35% dimension of screen
	Vec cascade_delta = {
		static_cast<Vec::vec_t>(0.2f * static_cast<float>(desktop.margin)), // Less x increase with every y increase
		desktop.margin
	};

	Vec window_dimensions = window_factor * desktop.monitor_dimensions;

	// Calculate, accounting for margin, how many cascades are needed for the screen
	Vec::vec_t working_desktop_height = desktop.monitor_dimensions.y - 2 * cascade_delta.y - window_dimensions.y;
	Vec::vec_t cascade_total_height = (static_cast<Vec::vec_t>(windows.size() - 1) * cascade_delta.y); // cascade height only considering the margin
	std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height); // amount of cascades
	std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); // remaining height

	// # of windows to draw = ceil(desktop_height / window_height)
	SizeType windows_per_full_cascade = static_cast<SizeType>((working_desktop_height / cascade_delta.y)) + ((working_desktop_height % cascade_delta.y != 0) ? 1 : 0);

	auto single_cascade = [&] (SizeType amount, Iterator start, std::size_t current_cascade) {
		for (SizeType index = 0; index < amount; ++index, ++start) {
			/*
			Point upper_left = { desktop.margin, desktop.margin };
			Point cascade_offset = { static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x), 0 };
			Point base = upper_left + cascade_offset;
			Point travel = static_cast<Vec::vec_t>(index) * cascade_delta;
			Point pos = base + travel;
			*/
			Vec pos = {
				static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x) + static_cast<Vec::vec_t>(index) * cascade_delta.x,
				static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(index) * cascade_delta.y
			};

			::SetWindowPos(start->handle,
				HWND_BOTTOM,
				static_cast<int>(pos.x), static_cast<int>(pos.y),
				static_cast<int>(window_dimensions.x), static_cast<int>(window_dimensions.y),
				SWP_NOACTIVATE
			);
		}
	};

	// Full cascades
	std::size_t current_cascade = 0;
	Iterator current_window = windows.cbegin();
	while (current_cascade < cascades) {
		single_cascade(windows_per_full_cascade, current_window, current_cascade);

		current_window += windows_per_full_cascade;
		++current_cascade;
	}

	// Leftovers
	if (std::distance(current_window, windows.cend()) > 0) {
		assert(std::distance(current_window, windows.cend()) < static_cast<DiffType>(windows_per_full_cascade));
		single_cascade(std::distance(current_window, windows.cend()), current_window, current_cascade);
	}
}

Window ApplyRules(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name) {
	Window new_window { window, title, class_name };
	for (auto& rule : Config::WindowRuleList) {
		if (!rule.Manage) continue; // Skip rules that exclude windows

		// Class name filtering
		if (!rule.ClassName.empty() && class_name.find(rule.ClassName) == std::string_view::npos) continue;

		// Title name filtering
		if (!rule.TitleName.empty() && title.find(rule.TitleName) == std::string_view::npos) continue;

		// Style filtering
		if ((style & rule.Style) != rule.Style)	continue;

		// Extended Style filtering
		if ((exStyle & rule.ExStyle) != rule.ExStyle) continue;

		new_window.floating = rule.Floating;
		new_window.current_tags |= rule.TagMask;
		new_window.action = rule.Action;
	}
	return new_window;
}

// Definitions
bool ShouldManageWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name) {
	// TODO: virtual desktop filtering

	// Empty window name filtering
	if (GetWindowTextLength(window) == 0) return false;

	// Visibility Filtering
	if (!IsWindowVisible(window)) return false;

	// Primary monitor only filtering
	if (Config::PrimaryMonitorWindowsOnly) {
		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (monitor != Globals::PrimaryMonitor) return false;
	}

	bool should_manage = std::any_of(Config::WindowRuleList.cbegin(), Config::WindowRuleList.cend(), [&] (auto& rule) -> bool {
		// Class name filtering
		if (!rule.ClassName.empty() && class_name.find(rule.ClassName) == std::string_view::npos) return false;

		// Title name filtering
		if (!rule.TitleName.empty() && title.find(rule.TitleName) == std::string_view::npos) return false;

		// Style filtering
		if ((style & rule.Style) != rule.Style)	return false;

		// Extended Style filtering
		if ((exStyle & rule.ExStyle) != rule.ExStyle) return false;

		// Manage window (return true) if rule says window should be managed
		return rule.Manage;
	});

	return should_manage;
}

BOOL CALLBACK ScanWindows(HWND window, LPARAM) {
	// TODO: make WS_MAXIMIZE windows unmaximize, try ShowWindow(window, SW_RESTORE)
	// Attributes
	LONG style = GetWindowLong(window, GWL_STYLE);
	if (style == 0) {
		debug("Style 0");
		// TODO: Error
		return false;
	}

	LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
	if (exStyle == 0) {
		debug("ExStyle 0");
		// TODO: Error
		return false;
	}

	TCHAR title_buf [buflen];
	int len = GetWindowText(window, title_buf, buflen);
	if (len == 0) {
		debug("Title len 0");
		// TODO: Error
		return false;
	}
	std::wstring_view title { title_buf, static_cast<std::size_t>(len) };

	TCHAR class_buf [buflen];
	len = GetClassName(window, class_buf, buflen);
	if (len == 0) {
		debug("Class len 0");
		// TODO: Error
		return false;
	}
	std::wstring_view class_name { class_buf, static_cast<std::size_t>(len) };

	if (ShouldManageWindow(window, style, exStyle, title, class_name)) {
		std::wcout << title << std::endl;

		Window new_window = ApplyRules(window, style, exStyle, title, class_name);
		Globals::Windows.push_back(std::move(new_window));
	}

	return TRUE;
}

HMONITOR GetPrimaryMonitorHandle() {
	return MonitorFromPoint(POINT { 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
}

int WinMain(HINSTANCE Instance, HINSTANCE, LPSTR, int CmdShow) {
	FILE* fDummy;
	AllocConsole();
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	// Setup desktop
	Globals::PrimaryMonitor = GetPrimaryMonitorHandle();
	MONITORINFO monitor_info = {
		.cbSize = sizeof(MONITORINFO)
	};
	if (!GetMonitorInfo(Globals::PrimaryMonitor, &monitor_info)) {
		// TODO: Error
		debug("Failed to get primary monitor")
			return EXIT_FAILURE;
	}
	Globals::PrimaryDesktop = Desktop {
		.margin = Config::DefaultMargin,
		.monitor_upper_left = Vec { monitor_info.rcWork.left, monitor_info.rcWork.top },
		.monitor_dimensions = Vec { monitor_info.rcWork.right - monitor_info.rcWork.left, monitor_info.rcWork.bottom - monitor_info.rcWork.top }
	};

	EnumWindows(ScanWindows, NULL);

	cascade(Globals::Windows, Globals::PrimaryDesktop);

	while (true) {
		if (GetAsyncKeyState(VK_NUMPAD0)) break;
	}

	fclose(fDummy);
	FreeConsole();

	return 0;
}