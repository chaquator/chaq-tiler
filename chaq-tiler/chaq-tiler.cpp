#include <windows.h>
#include <Shobjidl.h>

#include <cstddef>
#include <cmath>
#include <type_traits>

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <bitset>
#include <iterator>

#include <cassert>

#include <iostream>

// Usings and typedefs
using namespace std::literals;

// Structs and classes

struct Rule {
	enum SingleAction {
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

struct Point {
	int x;
	int y;
};
inline Point operator +(Point a, Point b) {
	return { a.x + b.x, a.y + b.y };
}
inline Point operator -(Point a, Point b) {
	return { a.x - b.x, a.y - b.y };
}
template <typename Scalar>
inline Point mul(Scalar scalar, Point p) {
	return {
		static_cast<int>(scalar * static_cast<Scalar>(p.x)),
		static_cast<int>(scalar * static_cast<Scalar>(p.y))
	};
}
template <typename Scalar>
inline Point operator *(Scalar a, Point b) {
	return mul(a, b);
}
template <typename Scalar>
inline Point operator *(Point b, Scalar a) {
	return mul(a, b);
}

// TODO: decide when to calculate this (probably in the beginning) and when to update (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for orientation and position of it too)
struct Desktop {
	int margin; // Space between each window (and from the monitor edge to the window)
	Point monitor_upper_left;
	Point monitor_lower_right;
};

struct Window {
	HWND handle;
	bool floating;
	// TODO: complete struct, things like rules applied

	Window(HWND hdnl, bool flt) : handle(hdnl), floating(flt) {}
};

// Globals

namespace Globals {
	std::vector<Window> Windows;
}

// Function definitions

// Debug utilities
static void debug(std::string&& text);

// Views
template <typename Container>
void cascade(const Container& windows, const Desktop& desktop);

static BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);
static bool shouldManageWindow(HWND);

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

constexpr static std::size_t buflen = 128;

// Including config
#include "config.h"

void debug(std::string&& text) {
	std::cout << "DEBUG: " << text << std::endl;
}

template <typename Container>
void cascade(const Container& windows, const Desktop& desktop) {
	// TODO: floating windows

	using SizeType = typename Container::size_type;
	using Iterator = typename Container::const_iterator;

	if (windows.empty()) return; // Short circuit break

	// Customize cascade here
	constexpr float window_factor = 0.35f; // Window will be 35% dimension of screen
	Point cascade_delta = {
		static_cast<int>(0.6f * static_cast<float>(desktop.margin)), // Slightly less x increase with every y increase
		desktop.margin
	};

	Point desktop_dimensions = desktop.monitor_lower_right - desktop.monitor_upper_left;
	Point window_dimensions = window_factor * desktop_dimensions;

	// Calculate, accounting for margin, how many cascades are needed for the screen
	int working_desktop_height = desktop_dimensions.y - 2 * cascade_delta.y - window_dimensions.y;
	int cascade_total_height = (static_cast<int>(windows.size() - 1) * cascade_delta.y); // cascade height only considering the margin
	std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height); // amount of cascades
	std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); // remaining height

	// # of windows to draw = ceil(desktop_height / window_height)
	SizeType windows_per_full_cascade = static_cast<SizeType>((working_desktop_height / cascade_delta.y)) + ((working_desktop_height % cascade_delta.y != 0) ? 1 : 0);

	auto single_cascade = [&](SizeType amount, Iterator start, std::size_t current_cascade) {
		for (std::size_t index = 0; index < static_cast<int>(amount); ++index, ++start) {
			/*
			Point ul = { desktop.margin, desktop.margin };
			Point cascade_offset = { current_cascade * (window_dimensions.x + cascade_delta.x), 0 };
			Point base = ul + cascade_offset;
			Point travel = index * cascade_delta;
			Point pos = base + travel;
			*/

			Point pos = (Point{ desktop.margin, desktop.margin } + Point{ static_cast<int>(current_cascade) * (window_dimensions.x + cascade_delta.x), 0 }) + (index * cascade_delta);
			SetWindowPos(start->handle, HWND_BOTTOM, pos.x, pos.y, window_dimensions.x, window_dimensions.y, SWP_NOACTIVATE);
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
		assert(std::distance(current_window, windows.cend()) < windows_per_full_cascade);
		single_cascade(std::distance(current_window, windows.cend()), current_window, current_cascade);
	}
}

// Definitions
bool shouldManageWindow(HWND window) {
	// Empty window name filtering
	if (GetWindowTextLength(window) == 0) return false;

	// Visibility Filtering
	if (!IsWindowVisible(window)) return false;

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

	TCHAR title_buf[buflen];
	int len = GetWindowText(window, title_buf, buflen);
	if (len == 0) {
		debug("Title len 0");
		// TODO: Error
		return false;
	}
	std::wstring_view title{ title_buf, static_cast<std::size_t>(len) };

	TCHAR class_buf[buflen];
	len = GetClassName(window, class_buf, buflen);
	if (len == 0) {
		debug("Class len 0");
		// TODO: Error
		return false;
	}
	std::wstring_view class_name{ class_buf, static_cast<std::size_t>(len) };

	for (auto& rule : window_rule_list) {
		// Class name filtering
		if (!rule.ClassName.empty() && class_name.find(rule.ClassName) == std::string_view::npos) continue;

		// Title name filtering
		if (!rule.TitleName.empty() && title.find(rule.TitleName) == std::string_view::npos) continue;

		// Style filtering
		if ((style & rule.Style) != rule.Style)	continue;

		// Extended Style filtering
		if ((exStyle & rule.ExStyle) != rule.ExStyle) continue;

		if (!rule.Manage) return false;
	}

	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND window, LPARAM) {
	// TODO: make WS_MAXIMIZE windows unmaximize, try ShowWindow(window, SW_RESTORE)

	if (shouldManageWindow(window)) {
		TCHAR buf[buflen];
		int len = GetWindowText(window, buf, buflen);
		if (len == 0) {
			debug("Title len 0");
			// Error
			return FALSE;
		}
		std::wstring_view title{ buf, static_cast<std::size_t>(len) };

		TCHAR class_buf[buflen];
		len = GetClassName(window, class_buf, buflen);
		if (len == 0) {
			debug("Class len 0");
			// TODO: Error
			return false;
		}
		std::wstring_view class_name{ class_buf, static_cast<std::size_t>(len) };

		// TODO: decide what to do with windows on different monitors
		// HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);

		std::wcout << title << std::endl;

		Globals::Windows.emplace_back(window, false);
	}

	return TRUE;
}

int WinMain(HINSTANCE Instance, HINSTANCE, LPSTR, int CmdShow) {
	FILE* fDummy;

	AllocConsole();
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	EnumWindows(EnumWindowsProc, NULL);

	Desktop desk = {
		.margin = 50,
		.monitor_upper_left = {0, 0},
		.monitor_lower_right = {2560, 1440}
	};

	cascade(Globals::Windows, desk);

	while (true) {
		if (GetAsyncKeyState(VK_NUMPAD0)) break;
	}

	fclose(fDummy);
	FreeConsole();

	return 0;
}