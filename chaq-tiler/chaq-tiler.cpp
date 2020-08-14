#include <windows.h>

#include <cstddef>
#include <type_traits>

#include <string>
#include <string_view>
#include <vector>
#include <bitset>
#include <iterator>

#include <cassert>

#include <algorithm>

#include "Rule.h"
#include "Vec.h"
#include "config.h"

#define NOMINMAX

// Usings and typedefs
using namespace std::literals;

// Structs and classes

// TODO: decide when to update this (looks like WM_DISPLAYCHANGE message in WindowProc).
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

	// TODO: consider whether these really need to be here, we can just get the title whenever needed right
	std::wstring title;
	std::wstring class_name;

	Window(HWND handle, std::wstring_view& title, std::wstring_view& class_name) :
		handle(handle), floating(false), current_tags(0), action(Rule::SingleAction::None),
		title(title), class_name(class_name) {
	}
};

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

// Views
class Views {
	// TODO: helper view functions here, like tile-strip and area-monocle

public:
	template <typename Iterator>
	static void cascade(const Iterator start, const Iterator end, const Desktop& desktop);

	template <typename Iterator>
	static void primary_secondary_stack(const Iterator start, const Iterator end, const Desktop& desktop);
};

static bool DoesRuleApply(const Rule& rule, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);
static Window GenerateWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);
static bool ShouldManageWindow(HWND);
static BOOL CALLBACK CreateWindows(HWND, LPARAM);
static HMONITOR GetPrimaryMonitorHandle();
void ApplyAction(const Window&);

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);

constexpr std::size_t buflen = 256; // TODO: this absolutely has to go somewhere else s2g

#ifdef NDEBUG
#define debug(s) ((void)0)
#else
#define debug(s) OutputDebugStringA(s "\n")
#endif

// Cascading view, breaks cascade into multiple to make windows fit vertically if necessary
// Mainly used for testing
template <typename Iterator>
void Views::cascade(const Iterator start, const Iterator end, const Desktop& desktop) {
	using DiffType = typename std::iterator_traits<Iterator>::difference_type;

	DiffType size = std::distance(start, end);
	if (size == 0) return; // Short circuit break

	// Customize cascade here
	constexpr float window_factor = 0.5f; // Window will be 50% dimension of screen
	Vec cascade_delta = {
		// static_cast<Vec::vec_t>(0.2f * static_cast<float>(desktop.margin)), // Less x increase with every y increase
		max(10, desktop.margin),
		max(10, desktop.margin)
	};

	Vec window_dimensions = window_factor * desktop.monitor_dimensions;

	// Calculate, accounting for margin, how many cascades are needed for the screen
	Vec::vec_t working_desktop_height = desktop.monitor_dimensions.y - 2 * cascade_delta.y - window_dimensions.y;
	Vec::vec_t cascade_total_height = (static_cast<Vec::vec_t>(size - 1) * cascade_delta.y); // cascade height only considering the margin
	std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height); // amount of cascades
	// std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); // remaining height

	// # of windows to draw = ceil(desktop_height / window_height)
	DiffType windows_per_full_cascade = static_cast<DiffType>((working_desktop_height / cascade_delta.y)) + ((working_desktop_height % cascade_delta.y != 0) ? 1 : 0);

	auto single_cascade = [&desktop, &window_dimensions, &cascade_delta] (DiffType amount, Iterator start, std::size_t current_cascade) {
		for (DiffType index = 0; index < amount; ++index, ++start) {
			/*
			Point upper_left = { desktop.margin, desktop.margin };
			Point cascade_offset = { static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x), 0 };
			Point base = upper_left + cascade_offset;
			Point travel = static_cast<Vec::vec_t>(index) * cascade_delta;
			Point pos = base + travel;
			*/
			Vec pos = {
				desktop.monitor_upper_left.x + static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x) + static_cast<Vec::vec_t>(index) * cascade_delta.x,
				desktop.monitor_upper_left.y + static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(index) * cascade_delta.y
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
	Iterator current_window = start;
	while (current_cascade < cascades) {
		single_cascade(windows_per_full_cascade, current_window, current_cascade);

		current_window += windows_per_full_cascade;
		++current_cascade;
	}

	// Leftovers
	DiffType remaining = std::distance(current_window, end);
	assert(remaining <= windows_per_full_cascade);
	single_cascade(remaining, current_window, current_cascade);
}

// Traditional dwm-like stack
template <typename Iterator>
void Views::primary_secondary_stack(const Iterator start, const Iterator end, const Desktop& desktop) {
	using DiffType = typename std::iterator_traits<Iterator>::difference_type;
}

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
		ApplyAction(new_window);
		Globals::Windows.push_back(std::move(new_window));
	}
	
	return TRUE;
}

HMONITOR GetPrimaryMonitorHandle() {
	return MonitorFromPoint(POINT { 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
}

void ApplyAction(const Window& window) {
	switch (window.action) {
		case Rule::SingleAction::None: break;
		case Rule::SingleAction::Unmaximize:
		{
			ShowWindow(window.handle, SW_SHOWNORMAL);
		} break;
		case Rule::SingleAction::Maximize:
		{
			ShowWindow(window.handle, SW_SHOWMAXIMIZED);
		} break;
	}
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
		Vec { monitor_info.rcWork.left, monitor_info.rcWork.top },
		Vec { monitor_info.rcWork.right - monitor_info.rcWork.left, monitor_info.rcWork.bottom - monitor_info.rcWork.top }
	};

	// Set up windows
	EnumWindows(CreateWindows, NULL);
	// Partition windows between non-floating and floating
	Globals::WindowPartitionPoint = std::stable_partition(Globals::Windows.begin(), Globals::Windows.end(), [] (auto& window) -> bool { return !window.floating; });

	// Single view call for now
	Views::cascade(Globals::Windows.cbegin(), Globals::WindowPartitionPoint, Globals::PrimaryDesktop);

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
	[ ] Create tile-strip function, drawns range of windows tiled next to each other (accounts for margins and all)
		within an area, parameterized for both horizontal and verticla orientation
			Consider parameterizing reverse of drawing too (for all levels)
	[ ] Create monocle function, piles windows on top of each other (bottom up)
	[ ] Use tile-strip & monocle to draw primary stack, secondary stack
	[ ] test

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