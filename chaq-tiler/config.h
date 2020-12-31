#pragma once

#include "Enums.h"
#include "Rule.h"
#include "Vec.h"

#include <array>
#include <bitset>
#include <cstddef>
#include <string_view>

namespace Config {

using namespace std::literals::string_view_literals;

// Window rule list
// Rules will be applied in order listed, any conflicting rules will be decided by whichever rule comes last,
// so order them from most general -> most specific.
// For all windows, if there exists a rule which applies to the window and the rule specifies not to manage the
// window, then the window is not managed.
// 	That is, it takes *only one* matching rule specifying not to manage for the window to be excluded from managing,
//  write accordingly.
// Class substring, Window substring, Window style, Extended window style, Should Manage, Floating, Tag Mask, Action
constexpr const std::array<Rule, 11> WindowRuleList = {
    // Exclude windows that match these

    // Disabled window filtering
    Rule{L""sv, L""sv, WS_DISABLED, 0, false, false, 0, Rule::Action::None},
    // Toolwindow filtering
    Rule{L""sv, L""sv, 0, WS_EX_TOOLWINDOW, false, false, 0, Rule::Action::None},
    // Hidden settings window
    Rule{L""sv, L"Settings"sv, 0, 0, false, false, 0, Rule::Action::None},
    // Hidden Microsoft Store window
    Rule{L""sv, L"Microsoft Store"sv, 0, 0, false, false, 0, Rule::Action::None},
    // Progman
    Rule{L"Progman"sv, L"Program Manager"sv, 0, 0, false, false, 0, Rule::Action::None},
    // ApplicationFrameWindow
    Rule{L"ApplicationFrameWindow"sv, L""sv, 0, 0, false, false, 0, Rule::Action::None},
    // NVIDIA Geforce
    Rule{L"CEF-OSC-WIDGET"sv, L"NVIDIA GeForce Overlay"sv, 0, 0, false, false, 0, Rule::Action::None},
    // Windows UI
    Rule{L"Windows.UI.Core.CoreWindow"sv, L""sv, 0, 0, false, false, 0, Rule::Action::None},
    // Skype's little window
    Rule{L"Skype"sv, L"Chrome_WidgetWin_1"sv, 0, WS_EX_TOPMOST, false, false, 0, Rule::Action::None},

    // Default management

    // Default rule
    Rule{L""sv, L""sv, 0, 0, true, false, 0, Rule::Action::Unmaximize},

    // Specific rules for any matched windows

    // mpv
    Rule{L"mpv"sv, L""sv, 0, 0, true, true, 0, Rule::Action::None},
};

// Window filtering
const bool PrimaryMonitorWindowsOnly = true;

// Screen defaults
const Enums::ViewType InitialView = Enums::ViewType::TileStack;
const Enums::ViewType InitialSecondaryView = Enums::ViewType::TileStack;
const Vec::vec_t InitialMargin = 16;
const Vec::vec_t MarginIncrement = 16;
const Vec::vec_t ProportionIncrement = 32;

// Primary-secondary stack defaults

// Proportion of screen that primary stack takes up
const float InitialPrimaryStackProportion = 0.666666f;
// Dimensions of primary grid
const Vec InitialPrimaryGridDimensions = Vec{1, 1};
// Maximum number of windows to display in secondary stack
const std::size_t InitialSecondaryMaxWindows = 2;

/*
    Stack Grid Orientaiton
    2 Axes: X and Y. 2 Directions: Positive and Negative
     For X, positive is right, for Y, downwards
    Enumerated from least significant (rightmost) bit to most significant (leftmost) bit
    0th bit -- Grid primary axis
                 Axis that first strip goes along
                 0: X, 1: Y
    1st bit -- Grid primary direction
                 Direction that first strip goes along primary axis
                 0: Positive, 1: Negative
    2nd bit -- Grid secondary direction
                 Direction that strips are tiled along the secondary axis
                 0: Positive, 1: Negative
    3rd bit -- Primary-secondary axis
                 Axis that primary grid and secondary stack are laid out along
                 0: X, 1: Y
    4th bit -- Primary-secondary direction
                 Direction that the primary grid and then secondary stack are laid out along primary-secondary axis
                 0: Positive, 1: Negative
    5th bit -- Secondary stack direction
                 Direction secondary stack goes along remaining axis
                 0: Positive, 1: Negative
    Diagram:
        Primary grid                                Secondary stack
       (if primary-secondary-direction is positive)
    +---------------------------------------------+-----------------------+
    |                                             |                       |
    | (-)<----Primary-grid-axis-(set to X)--->(+) |                       |
    |                                             |                       |
    | (-)                                         |     (-)               |
    |  ^                                          |      ^                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  | (Secondary                               |      | (Secondary     |
    |  |    grid                                  |      | stack          |
    |  |    axis)                                 |      | direction)     |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  |                                          |      |                |
    |  v                                          |      v                |
    | (+)                                         |     (+)               |
    |                                             |                       |
    |                                             |                       |
    |                                             |                       |
    +---------------------------------------------+-----------------------+

     (-)<-----------Primary-secondary-axis-(set to X)---------------->(+)

*/
using StackGridOrientation = std::bitset<6>;
const StackGridOrientation InitialStackGridOrientation = 0b010000;
enum Orientation : std::size_t {
    GridAxis,
    GridPrimaryDirection,
    GridSecondaryDirection,
    PrimarySecondaryAxis,
    PrimarySecondaryDirection,
    SecondaryDirection,
};

} // namespace Config
