#include "Desktop.h"

// it ees wat it ees
Desktop::Desktop(Rect desktopRect) :
	rect(desktopRect), view(Config::InitialView), margin(Config::InitialMargin),
	primary_secondary_orientation(Config::InitialPrimarySecondaryOrientation),
	primary_major_direction(Config::InitialPrimaryMajorDirection),
	primary_sub_direction(Config::InitialPrimarySubDirection),
	secondary_view(Config::InitialSecondaryView),
	secondary_orientation(Config::InitialSecondaryOrientation),
	secondary_reverse(Config::InitialSecondaryReverse) {
	// primary_stack_size
	auto component_of_interest = [] (Enums::Orientation orientation, auto& vec) constexpr -> decltype(auto) {
		return orientation == Enums::Orientation::Horizontal ? (vec.x) : (vec.y);
	};
	this->primary_stack_size = static_cast<decltype(primary_stack_size)>(
		static_cast<float>(component_of_interest(Config::InitialPrimarySecondaryOrientation, desktopRect.dimensions)) * Config::InitialPrimarySecondaryProportion);
}
