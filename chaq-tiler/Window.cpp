#include "Window.h"
#include "config.h"

void Window::SetPos(const Rect& rect, HWND after, bool hide) const {
    if (!hide) this->ApplyAction();

    SetWindowPos(this->handle, after, static_cast<int>(rect.upper_left.x), static_cast<int>(rect.upper_left.y),
                 static_cast<int>(rect.dimensions.x), static_cast<int>(rect.dimensions.y),
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

    if (hide) ShowWindow(this->handle, SW_MINIMIZE);
}

void Window::ApplyAction() const {
    switch (this->action) {
    case Rule::Action::None:
        break;
    case Rule::Action::Unmaximize: {
        ShowWindow(this->handle, SW_SHOWNORMAL);
    } break;
    case Rule::Action::Maximize: {
        ShowWindow(this->handle, SW_SHOWMAXIMIZED);
    } break;
    }
}

bool Window::DoesRuleApply(const Rule& rule, LONG style, LONG exStyle, std::wstring_view& title,
                           std::wstring_view& class_name) {
    // Class name filtering (substring search)
    if (!rule.ClassName.empty() && class_name.find(rule.ClassName) == std::string_view::npos) return false;

    // Title name filtering (substring search)
    if (!rule.TitleName.empty() && title.find(rule.TitleName) == std::string_view::npos) return false;

    // Style filtering (all flags in rule's style are present in the given style)
    if ((style & rule.Style) != rule.Style) return false;

    // Extended Style filtering (ditto)
    if ((exStyle & rule.ExStyle) != rule.ExStyle) return false;

    return true;
}

Window::Window(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name)
    : handle(window), title(title), class_name(class_name) {
    for (const auto& rule : Config::WindowRuleList) {
        // Skip rules that does not manage any windows
        if (!rule.Manage) continue;

        // Skip if rule does not apply
        if (!DoesRuleApply(rule, style, exStyle, title, class_name)) continue;

        // Otherwise, apply rules
        // Tag masks will be OR'd together
        this->floating = rule.Floating;
        this->current_tags |= rule.TagMask;
        this->action = rule.SpecificAction;
    }
}
