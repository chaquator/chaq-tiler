#include <windows.h>

#include <string_view>
#include <cstdio>
#include <cwchar>

//#define debug(s) std::printf(s "\n\n");
#define debug(s) ((void)0)

constexpr std::size_t buflen = 128;

BOOL CALLBACK ListWindowTraits(HWND window, LPARAM) {
	// Attributes
	LONG style = GetWindowLong(window, GWL_STYLE);
	if (style == 0) {
		debug("Style 0, skipping...");
		return TRUE;
	}

	LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
	if (exStyle == 0) {
		debug("ExStyle 0, skipping...");
		return TRUE;
	}

	TCHAR title_buf[buflen];
	int len = GetWindowText(window, title_buf, buflen);
	if (len == 0) {
		debug("Title len 0, skipping...");
		return TRUE;
	}
	// std::wstring_view title{ title_buf, static_cast<std::size_t>(len) };

	TCHAR class_buf[buflen];
	len = GetClassName(window, class_buf, buflen);
	if (len == 0) {
		debug("Class len 0, skipping...");
		return TRUE;
	}
	// std::wstring_view class_name{ class_buf, static_cast<std::size_t>(len) };

	std::wprintf(L"Title: %s\n"
		L"Class: %s\n"
		L"Style: 0x%08X, ExStyle: 0x%08X\n\n",
		title_buf, class_buf, style, exStyle);

	return TRUE;
}

int main() {
	EnumWindows(ListWindowTraits, NULL);
}