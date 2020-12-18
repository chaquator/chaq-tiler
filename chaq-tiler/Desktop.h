#pragma once

#include "Vec.h"
#include "Enums.h"

#include "config.h"

// TODO: decide when to update this (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for orientation and position of it too)
struct Desktop {
	// Upper left point and screen-size
	Rect rect; 

	Enums::ViewType view = Config::InitialView;
	Vec::vec_t margin = Config::InitialMargin;

	float primary_stack_proportion = Config::InitialPrimaryStackProportion;
	Enums::Orientation primary_secondary_orientation = Config::InitialPrimarySecondaryOrientation;
	Enums::MajorDirection primary_major_direction = Config::InitialPrimaryMajorDirection;
	Enums::SubDirection primary_sub_direction = Config::InitialPrimarySubDirection;

	Enums::ViewType secondary_view = Config::InitialSecondaryView;
	Enums::Orientation secondary_orientation = Config::InitialSecondaryOrientation;
	bool secondary_reverse = Config::InitialSecondaryReverse;

	void updateRect(Rect rect);
};
