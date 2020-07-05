#include <windows.h>
#include <Shobjidl.h>

#include <cstddef>

#include <string>
#include <string_view>
#include <array>
#include <bitset>

#include <iostream>

// Usings and typedefs
using namespace std::literals;

// Structs and classes
struct Rule {
	enum SingleAction {
		None,
		Minimize,
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

	// Actions

	// Should windows under this rule be managed? If not, Tag Mask and Action doesn't matter.
	bool Manage;
	// 10-bit bitstring representing which tags to place the window in when first managed
	// NOTE: 0th bit is intended to represent key 1 on the number row, and so on
	std::bitset<10> TagMask;
	// Apply any specific actions to the window
	SingleAction Action;
};

// Function definitions
static void debug(std::string&& text);
static BOOL CALLBACK EnumWindowsProc(HWND, LPARAM);
static bool shouldManageWindow(HWND);
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

constexpr static std::size_t buflen = 128;

// Including config
#include "config.h"

void debug(std::string&& text) {
	std::cout << "DEBUG: " << text << std::endl;
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

		HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);

		std::wcout << title << std::endl;
	}

	return TRUE;
}

int WinMain(HINSTANCE Instance, HINSTANCE, LPSTR, int CmdShow) {
	FILE* fDummy;

	AllocConsole();
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	EnumWindows(EnumWindowsProc, NULL);

	while (true) {
		if (GetAsyncKeyState(VK_NUMPAD0)) break;
	}

	fclose(fDummy);
	FreeConsole();

	return 0;
}