#pragma once

#include "Vec.h"

// TODO: decide when to update this (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for orientation and position of it too)
struct Desktop {
	int margin; // Space between each window (and from the monitor edge to the window)
	Rect rect;
};