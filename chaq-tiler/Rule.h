#pragma once

#include <windows.h>

#include <bitset>
#include <string>
#include <string_view>

struct Rule {
    enum class Action {
        None,
        Unmaximize,
        Maximize,
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
    // See https://docs.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
    LONG ExStyle;

    // Decisions

    // Manage -- should the prorgam manage windows in this rule?
    // NOTE: if false, attributes below will obviously not apply. If you want them to apply while
    // also keeping the window "unmanaged", set Manage to true and Floating to false
    bool Manage;

    // Attributes

    // Floating -- true -> window should NOT be moved when arranged
    bool Floating;
    // TagMask -- 10-bit bitstring representing which tags to place the window in when first managed
    // All 0s -> window is associated to the current tag
    // NOTE: 0th bit is intended to represent key 1 on the number row, and so on
    std::bitset<10> TagMask;
    // Action -- any specific action applied to the window when first managed
    Action SpecificAction;
};
