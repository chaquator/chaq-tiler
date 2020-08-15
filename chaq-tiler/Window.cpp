#include <Windows.h>

#include "Window.h"

void Window::SetPos(const Rect& rect) const {
	this->ApplyAction();

	::SetWindowPos(this->handle,
		HWND_BOTTOM,
		static_cast<int>(rect.upper_left.x), static_cast<int>(rect.upper_left.y),
		static_cast<int>(rect.dimensions.x), static_cast<int>(rect.dimensions.y),
		SWP_NOACTIVATE
	);
}

void Window::ApplyAction() const {
	switch (this->action) {
		case Rule::SingleAction::None: break;
		case Rule::SingleAction::Unmaximize:
		{
			ShowWindow(this->handle, SW_SHOWNORMAL);
		} break;
		case Rule::SingleAction::Maximize:
		{
			ShowWindow(this->handle, SW_SHOWMAXIMIZED);
		} break;
	}
}