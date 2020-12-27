#pragma once

#include <Windows.h>
#include <bitset>
#include <string>
#include <string_view>

#include "Rule.h"
#include "Vec.h"

struct Window {
    HWND handle;
    bool floating;
    std::bitset<10> current_tags;
    Rule::SingleAction action;

    // TODO: consider whether these really need to be here, we can just get the title whenever needed right
    std::wstring title;
    std::wstring class_name;

    Window(HWND handle, std::wstring_view& title, std::wstring_view& class_name)
        : handle(handle), floating(false), current_tags(0), action(Rule::SingleAction::None), title(title),
          class_name(class_name) {}

    void SetPos(const Rect& rect, HWND after = HWND_BOTTOM, bool hide = false) const;

    void ApplyAction() const;
};
