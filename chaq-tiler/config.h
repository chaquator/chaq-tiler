#include "Rule.h"

#include <array>
#include <string_view>

using namespace std::literals::string_view_literals;

namespace Config {
	// Window rule list
	// Rules will be applied in order listed, any conflicting rules will be decided by whichever rule comes last, so order them from most general -> most specific.
	// For all windows, if there exists a rule which applies to the window and the rule specifies not to manage the window, then the window is not managed.
	//	That is, it takes *only one* matching rule specifying not to manage for the window to be excluded from managing, write accordingly.
	// Class substring, Window substring, Window style, Extended window style, Should Manage, Floating, Tag Mask, Action
	constexpr const auto WindowRuleList = std::to_array({
		// Exclude windows that match these
		Rule{L""sv, L""sv, WS_DISABLED, 0, false, false, 0, Rule::SingleAction::None}, // Disabled window filtering
		Rule{L""sv, L""sv, 0, WS_EX_TOOLWINDOW, false, false, 0, Rule::SingleAction::None}, // Toolwindow filtering
		Rule{L""sv, L"Settings"sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // Hidden settings window
		Rule{L""sv, L"Microsoft Store"sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // Hidden Microsoft Store window
		Rule{L"Progman"sv, L"Program Manager"sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // Progman
		Rule{L"ApplicationFrameWindow"sv, L""sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // ApplicationFrameWindow
		Rule{L"CEF-OSC-WIDGET"sv, L"NVIDIA GeForce Overlay"sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // NVIDIA Geforce
		Rule{L"Windows.UI.Core.CoreWindow"sv, L""sv, 0, 0, false, false, 0, Rule::SingleAction::None}, // Windows UI

		// Default management
		Rule{L""sv, L""sv, 0, 0, true, false, 0, Rule::SingleAction::Unmaximize}, // Default rule

		// Specific rules for any matched windows
		Rule{L"mpv"sv, L""sv, 0, 0, true, true, 0, Rule::SingleAction::None} // mpv
		});

	constexpr const Vec::vec_t DefaultMargin = 16;

	constexpr const Vec DefaultPrimaryStackDimensions = { 1, 1 };

	constexpr const bool PrimaryMonitorWindowsOnly = true;
}