// Window rule list
// Rules will be applied in order listed, any conflicting rules will be decided by whichever rule comes last, so list them from most general -> most specific
// Class substring, Window substring, Window style, Extended window style, Should Manage, Floating, Tag Mask, Action
namespace Config {
	constexpr const std::array<Rule, 7> WindowRuleList = {
		Rule{L"", L"", NULL, NULL, true, false, 0, Rule::SingleAction::Unmaximize}, // Default rule

		Rule{L"", L"", WS_DISABLED, WS_EX_TOOLWINDOW, false, false, 0, Rule::SingleAction::None}, // Disabled/Toolwindow filtering
		Rule{L"", L"Settings", 0, 0, false, false, 0, Rule::SingleAction::None}, // Hidden settings window
		Rule{L"", L"Microsoft Store", 0, 0, false, false, 0, Rule::SingleAction::None}, // Hidden Microsoft Store window
		Rule{L"Progman", L"Program Manager", 0, 0, false, false, 0, Rule::SingleAction::None}, // Progman

		Rule{L"CEF-OSC-WIDGET", L"NVIDIA GeForce Overlay", 0, 0, false, false, 0, Rule::SingleAction::None}, // NVIDIA Geforce
		Rule{L"Windows.UI.Core.CoreWindow", L"", 0, 0, false, false, 0, Rule::SingleAction::None} // Windows UI
	};

	constexpr const int DefaultMargin = 50;

	constexpr const bool PrimaryMonitorWindowsOnly = true;
}