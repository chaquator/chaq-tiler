#include "Window.h"

void Window::SetPos(const Rect& rect, HWND after, bool hide) const {
    if (!hide) this->ApplyAction();

    SetWindowPos(this->handle, after, static_cast<int>(rect.upper_left.x), static_cast<int>(rect.upper_left.y),
                 static_cast<int>(rect.dimensions.x), static_cast<int>(rect.dimensions.y),
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

    if (hide) ShowWindow(this->handle, SW_MINIMIZE);
}

void Window::ApplyAction() const {
    switch (this->action) {
    case Rule::SingleAction::None:
        break;
    case Rule::SingleAction::Unmaximize: {
        ShowWindow(this->handle, SW_SHOWNORMAL);
    } break;
    case Rule::SingleAction::Maximize: {
        ShowWindow(this->handle, SW_SHOWMAXIMIZED);
    } break;
    }
}
