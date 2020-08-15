#include <Windows.h>

#include "Window.h"

void Window::SetPos(Rect rect) const {
	::SetWindowPos(this->handle,
		HWND_BOTTOM,
		static_cast<int>(rect.upper_left.x), static_cast<int>(rect.upper_left.y),
		static_cast<int>(rect.dimensions.x), static_cast<int>(rect.dimensions.y),
		SWP_NOACTIVATE
	);
}