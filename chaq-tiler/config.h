#pragma once

#include "Enums.h"
#include "Rule.h"
#include "Vec.h"

#include <array>
#include <bitset>
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
constexpr const std::array<Rule, 10> WindowRuleList = {
    // Exclude windows that match these

    // Disabled window filtering
    Rule{L""sv, L""sv, WS_DISABLED, 0, false, false, 0, Rule::SingleAction::None},
    // Toolwindow filtering
    Rule{L""sv, L""sv, 0, WS_EX_TOOLWINDOW, false, false, 0, Rule::SingleAction::None},
    // Hidden settings window
    Rule{L""sv, L"Settings"sv, 0, 0, false, false, 0, Rule::SingleAction::None},
    // Hidden Microsoft Store window
    Rule{L""sv, L"Microsoft Store"sv, 0, 0, false, false, 0, Rule::SingleAction::None},
    // Progman
    Rule{L"Progman"sv, L"Program Manager"sv, 0, 0, false, false, 0, Rule::SingleAction::None},
    // ApplicationFrameWindow
    Rule{L"ApplicationFrameWindow"sv, L""sv, 0, 0, false, false, 0, Rule::SingleAction::None},
    // NVIDIA Geforce
    Rule{L"CEF-OSC-WIDGET"sv, L"NVIDIA GeForce Overlay"sv, 0, 0, false, false, 0, Rule::SingleAction::None},
    // Windows UI
    Rule{L"Windows.UI.Core.CoreWindow"sv, L""sv, 0, 0, false, false, 0, Rule::SingleAction::None},

    // Default management

    // Default rule
    Rule{L""sv, L""sv, 0, 0, true, false, 0, Rule::SingleAction::Unmaximize},

    // Specific rules for any matched windows

    // mpv
    Rule{L"mpv"sv, L""sv, 0, 0, true, true, 0, Rule::SingleAction::None},
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
const float InitialPrimaryStackProportion = 0.75f;
// false -> [primary | secondary] (if orientation is vertical, primary stack is on top)
const bool InitialPrimarySecondaryReverse = false;
const Vec InitialPrimaryStackDimensions = Vec{1, 1};

// Stack Grid Orientaiton
// 2 Axes: X and Y. 2 Directions: Positive and Negative
//  For X, positive is right, for Y, downwards
// Enumerated from least significant (rightmost) bit to most significant (leftmost) bit
// 0th bit -- Primary Axis
//              Axis that first strip goes along
//              0: X, 1: Y
// 1st bit -- Primary grid direction
//              Direction that first strip goes along primary axis
//              0: Positive, 1: Negative
// 2nd bit -- Secondary grid direction
//              Direction that strips are tiled along the secondary axis
//              0: Positive, 1: Negative
// 3rd bit -- Primary-Secondary axis
//              Axis that primary grid and secondary stack are laid out along
//              0: X, 1: Y
// 4th bit -- Primary-Secondary direction
//              Direction that the primary grid and then secondary stack are laid out along primary-secondary axis
//              0: Positive, 1: Negative
// 5th bit -- Secondary stack direction
//              Direction secondary stack goes along remaining axis
//              0: Positive, 1: Negative
using StackGridOrientation = std::bitset<6>;
const StackGridOrientation InitialStackGridOrientation = 0b010000;

} // namespace Config
