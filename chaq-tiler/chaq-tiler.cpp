#include <windows.h>

#include <cstddef>

#include <string>
#include <string_view>
#include <vector>

#include <cassert>

#include <algorithm>

#include "Desktop.h"
#include "Rule.h"
#include "Vec.h"
#include "Views.h"
#include "Window.h"
#include "config.h"

#define NOMINMAX

// Usings and typedefs
using namespace std::literals;

// Structs and classes

// Globals

namespace Globals {
	// runtime globals
	std::vector<Window> Windows;
	decltype(Windows)::const_iterator WindowPartitionPoint;
	Vec PrimaryStackDimensions = Config::DefaultPrimaryStackDimensions;

	// setup globals
	HMONITOR PrimaryMonitor;
	Desktop PrimaryDesktop;
}

// Function definitions

static bool DoesRuleApply(const Rule& rule, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);
static Window GenerateWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);
static bool ShouldManageWindow(HWND);
static BOOL CALLBACK CreateWindows(HWND, LPARAM);
static HMONITOR GetPrimaryMonitorHandle();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);

constexpr std::size_t buflen = 256; // TODO: this absolutely has to go somewhere else s2g

#ifdef NDEBUG
#define debug(s) ((void)0)
#else
#define debug(s) OutputDebugStringA(s "\n")
#endif

// Given the attributes, does the given rule apply
bool DoesRuleApply(const Rule& rule, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name) {
	// Class name filtering (substring search)
	if (!rule.ClassName.empty() && class_name.find(rule.ClassName) == std::string_view::npos) return false;

	// Title name filtering (substring search)
	if (!rule.TitleName.empty() && title.find(rule.TitleName) == std::string_view::npos) return false;

	// Style filtering (all flags in rule's style are present in the given style)
	if ((style & rule.Style) != rule.Style)	return false;

	// Extended Style filtering (ditto)
	if ((exStyle & rule.ExStyle) != rule.ExStyle) return false;

	return true;
}

// Create new window and apply all rules to it given its attributes
Window GenerateWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name) {
	Window new_window { window, title, class_name };
	for (const auto& rule : Config::WindowRuleList) {
		// Skip rules that does not manage any windows
		if (!rule.Manage) continue;

		// Skip if rule does not apply
		if (!DoesRuleApply(rule, style, exStyle, title, class_name)) continue;

		// Otherwise, apply rules
		// Tag masks will be OR'd together
		new_window.floating = rule.Floating;
		new_window.current_tags |= rule.TagMask;
		new_window.action = rule.Action;
	}

	return new_window;
}

// Given the attributes, should this window be managed
bool ShouldManageWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name) {
	// TODO: virtual desktop filtering

	// Empty window name filtering
	if (GetWindowTextLengthW(window) == 0) return false;

	// Visibility Filtering
	if (!IsWindowVisible(window)) return false;

	// Primary monitor only filtering
	if (Config::PrimaryMonitorWindowsOnly) {
		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
		if (monitor != Globals::PrimaryMonitor) return false;
	}

	bool should_skip = std::any_of(Config::WindowRuleList.cbegin(), Config::WindowRuleList.cend(), [&style, &exStyle, &title, &class_name] (const auto& rule) -> bool {
		return DoesRuleApply(rule, style, exStyle, title, class_name) && !rule.Manage;
	});

	return !should_skip;
}

BOOL CALLBACK CreateWindows(HWND window, LPARAM) {
	// TODO: make WS_MAXIMIZE windows unmaximize, try ShowWindow(window, SW_RESTORE)
	// Attributes
	LONG style = GetWindowLongW(window, GWL_STYLE);
	if (style == 0) {
		debug("Style 0");
		// TODO: Error
		return TRUE;
	}

	LONG exStyle = GetWindowLongW(window, GWL_EXSTYLE);
	if (exStyle == 0) {
		debug("ExStyle 0");
		// TODO: Error
		return TRUE;
	}

	TCHAR title_buf[buflen];
	int len = GetWindowTextW(window, title_buf, buflen);
	if (len == 0) {
		debug("Title len 0");
		// TODO: Error
		return TRUE;
	}
	std::wstring_view title { title_buf, static_cast<std::size_t>(len) };

	TCHAR class_buf[buflen];
	len = GetClassNameW(window, class_buf, buflen);
	if (len == 0) {
		debug("Class len 0");
		// TODO: Error
		return TRUE;
	}
	std::wstring_view class_name { class_buf, static_cast<std::size_t>(len) };

	if (ShouldManageWindow(window, style, exStyle, title, class_name)) {
		Window new_window = GenerateWindow(window, style, exStyle, title, class_name);
		Globals::Windows.push_back(std::move(new_window));
	}

	return TRUE;
}

HMONITOR GetPrimaryMonitorHandle() {
	return MonitorFromPoint(POINT { 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// Setup globals
	// Primary monitor
	Globals::PrimaryMonitor = GetPrimaryMonitorHandle();
	MONITORINFO monitor_info = {
		.cbSize = sizeof(MONITORINFO)
	};
	if (!GetMonitorInfoW(Globals::PrimaryMonitor, &monitor_info)) {
		debug("Failed to get primary monitor");
		return EXIT_FAILURE;
	}
	// Primary desktop rectangle
	Globals::PrimaryDesktop = Desktop {
		Config::DefaultMargin,
		Rect {
			Vec { monitor_info.rcWork.left, monitor_info.rcWork.top },
			Vec { monitor_info.rcWork.right - monitor_info.rcWork.left, monitor_info.rcWork.bottom - monitor_info.rcWork.top }
		}
	};

	// Set up windows
	EnumWindows(CreateWindows, NULL);
	// Partition windows between non-floating and floating
	Globals::WindowPartitionPoint = std::stable_partition(Globals::Windows.begin(), Globals::Windows.end(), [] (const auto& window) -> bool { return !window.floating; });

	// Single view call for now
	//Views::cascade(Globals::Windows.cbegin(), Globals::WindowPartitionPoint, Globals::PrimaryDesktop);
	Views::primary_secondary_stack(Globals::Windows.cbegin(), Globals::WindowPartitionPoint, Globals::PrimaryDesktop);

	return 0;
}

/*
	implement runtime data structure storing all managed windows.
	[X] Will be an std::vector of windows in stack order, partitioned as:
		[ not floating windows | floating windows ]
	[X] Come up with a rule that manages a window while also letting it be floating (prolly mpv)
	[X] At the end of all windows being created partition by floating, store iterator to
		first floating window globally
	[X] Modify current cascading view to accept start and end iterator instead of whole vector
	[X] test

	dwm like primary-seocndary stack
	[X] Create tile-strip function, drawns range of windows tiled next to each other (accounts for margins and all)
		within an area, parameterized for both horizontal and verticla orientation
			Consider parameterizing reverse of drawing too (for all levels)
	[X] Test tile-strip
	[ ] Create monocle function, piles windows on top of each other (bottom up)
	[ ] Use tile-strip & monocle to draw primary stack, secondary stack
	[ ] test

	move window-related functions out into seperate file
	[X] set window position function
	[ ] the rest

	Future notes:
	This only matters for the cascading and monocle view, consider setting the user's
	foucs to the correct window if just adjusting the windows is not focusing correctly

	When hotkeys are in, be sure cursor doesn't enter into floating windows partition, instead loops back.
		There definitely should be a way to change focus too. Maybe even change cursor position.
			SetFocus? SetForegroundWindow? Investigate dwm-win32 and bug.n

	When focus tracking is in, be sure cursor does enter floating partition based on focus

	In the future, there will be an arising issue of whether the tiler will be capable
	of tracking windows as their attributes change and applying new rules accordingly.

*/