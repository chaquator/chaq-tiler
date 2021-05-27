#pragma once

#include <Windows.h>
#include <bitset>
#include <string>
#include <string_view>

#include "Rule.h"
#include "Vec.h"

struct Window {
    HWND handle;
    bool floating = false;
    std::bitset<10> current_tags = 0;
    Rule::Action action = Rule::Action::None;

    // TODO: consider whether these really need to be here, we can just get the title whenever needed right
    std::wstring title;
    std::wstring class_name;

    Window(HWND window, LONG style, LONG exStyle, std::wstring_view& title, std::wstring_view& class_name);

    void SetPos(const Rect& rect, HWND after, bool hide = false) const;

    void ApplyAction() const;

    static bool DoesRuleApply(const Rule& rule, LONG style, LONG exStyle, std::wstring_view& title,
                              std::wstring_view& class_name);
};
