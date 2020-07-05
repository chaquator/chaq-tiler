// Window rule list
// Class substring, Window substring, Window style, Extended window style, Should Manage, Tag Mask, Action
constexpr std::array<Rule, 5> window_rule_list = {
	Rule{L"", L"", WS_DISABLED, WS_EX_TOOLWINDOW, false, 0, Rule::SingleAction::None}, // Default filtering
	Rule{L"", L"Settings", 0, 0, false, 0, Rule::SingleAction::None}, // Hidden settings window
	Rule{L"", L"Microsoft Store", 0, 0, false, 0, Rule::SingleAction::None}, // Hidden Microsoft Store window
	Rule{L"Progman", L"Program Manager", 0, 0, false, 0, Rule::SingleAction::None}, // Progman

	Rule{L"CEF-OSC-WIDGET", L"NVIDIA GeForce Overlay", 0, 0, false, 0, Rule::SingleAction::None} // NVIDIA Geforce
};