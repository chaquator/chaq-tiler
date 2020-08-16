#pragma once

#include "Vec.h"
#include "Enums.h"

#include "config.h"

// Desktop config
namespace Config {

}


// TODO: decide when to update this (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for orientation and position of it too)
struct Desktop {
	Rect rect; // Upper left point and screen-size
	Vec::vec_t primary_stack_size;

	Enums::Display view;
	Vec::vec_t margin;

	Enums::Orientation primary_secondary_orientation;
	Enums::MajorDirection primary_major_direction;
	Enums::SubDirection primary_sub_direction;

	Enums::Display secondary_view;
	Enums::Orientation secondary_orientation;
	bool secondary_reverse;

	Desktop() = default;
	Desktop(Rect desktopRect);
};