#include <windows.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "Desktop.h"
#include "Globals.h"
#include "Rule.h"
#include "Vec.h"
#include "Views.h"
#include "Window.h"
#include "config.h"
#include "debug.h"

static bool ShouldManageWindow(HWND, LONG, LONG, std::wstring_view&, std::wstring_view&);
static BOOL CALLBACK CreateWindows(HWND, LPARAM);
static HMONITOR PrimaryMonitorHandle();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);

constexpr std::size_t buflen = 256;

static HMONITOR primary_monitor;

// Given the attributes, should this window be managed
static bool ShouldManageWindow(HWND window, LONG style, LONG exStyle, std::wstring_view& title,
                               std::wstring_view& class_name) {
    // TODO: virtual desktop filtering

    // Empty window name filtering
    if (GetWindowTextLengthW(window) == 0) return false;

    // Visibility Filtering
    if (!IsWindowVisible(window)) return false;

    // Primary monitor only filtering
    if (Config::PrimaryMonitorWindowsOnly) {
        HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
        if (monitor != primary_monitor) return false;
    }

    // skip if any applying rule specifies that the window should not be managed
    auto skip_test = [&style, &exStyle, &title, &class_name](const Rule& rule) -> bool {
        // dont worry abuot rules not relevant to window
        if (!Window::DoesRuleApply(rule, style, exStyle, title, class_name)) return false;

        // if the relevant rule says not to manage the window, we skip adding the window
        return !rule.Manage;
    };
    bool should_skip = std::any_of(Config::WindowRuleList.cbegin(), Config::WindowRuleList.cend(), skip_test);

    // true if window should *not* be skipped
    return !should_skip;
}

static BOOL CALLBACK CreateWindows(HWND window, LPARAM) {
    // TODO: make WS_MAXIMIZE windows unmaximize, try ShowWindow(window,
    // SW_RESTORE) Attributes
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
    int len = GetWindowTextW(window, (LPWSTR)title_buf, buflen);
    if (len == 0) {
        debug("Title len 0");
        // TODO: Error
        return TRUE;
    }
    std::wstring_view title{(LPWSTR)title_buf, static_cast<std::size_t>(len)};

    TCHAR class_buf[buflen];
    len = GetClassNameW(window, (LPWSTR)class_buf, buflen);
    if (len == 0) {
        debug("Class len 0");
        // TODO: Error
        return TRUE;
    }
    std::wstring_view class_name{(LPWSTR)class_buf, static_cast<std::size_t>(len)};

    if (ShouldManageWindow(window, style, exStyle, title, class_name)) {
        Globals::Windows.emplace_back(window, style, exStyle, title, class_name);
    }

    return TRUE;
}

static HMONITOR PrimaryMonitorHandle() { return MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY); }

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    // Setup globals
    // Primary monitor
    primary_monitor = PrimaryMonitorHandle();
    Globals::Desktops.emplace_back(primary_monitor);

    // Set up windows
    EnumWindows(CreateWindows, (LPARAM)0);
    // Partition windows between non-floating and floating
    Globals::WindowPartitionPoint = std::stable_partition(Globals::Windows.begin(), Globals::Windows.end(),
                                                          [](const auto& window) -> bool { return !window.floating; });
    Globals::WindowFocusPoint = Globals::Windows.cbegin();

    // Single view call for now
    Views::TileStack(Globals::Windows.cbegin(), Globals::WindowPartitionPoint, Globals::Desktops[0]);

    return 0;
}

/*
        implement runtime data structure storing all managed windows.
        [X] Will be an std::vector of windows in stack order, partitioned as:
                [ not floating windows | floating windows ]
        [X] Come up with a rule that manages a window while also letting it be
   floating (prolly mpv) [X] At the end of all windows being created partition
   by floating, store iterator to first floating window globally [X] Modify
   current cascading view to accept start and end iterator instead of whole
   vector [X] test

        dwm like primary-seocndary stack
        [X] Create tile-strip function, drawns range of windows tiled next to
   each other (accounts for margins and all) within an area, parameterized for
   both horizontal and vertical orientation Consider parameterizing reverse of
   drawing too (for all levels) [X] Create monocle area function, piles windows
   on top of each other (bottom up) [X] Write draw-grid function [X] Test
   tile-strip [X] Test monocle area function [X] Test draw-grid function [X]
   Write and test entire tile-stack fuction

        move window-related functions out into seperate file
        [X] set window position function
        [ ] the rest

        track new windows
        [ ] Investigate the work of bug.n and dwm-win32 to see what the process
   is for tracking new windows

        When hotkeys are in, be sure cursor doesn't enter into floating windows
   partition, instead loops back. There definitely should be a way to change
   focus too. Maybe even change cursor position. SetFocus? SetForegroundWindow?
   Investigate dwm-win32 and bug.n When focus tracking is in, be sure cursor
   does enter floating partition based on focus

        In the future, there will be an arising issue of whether the tiler will
   be capable of tracking windows as their attributes change and applying new
   rules accordingly.

*/
