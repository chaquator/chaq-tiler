#pragma once

#include "Enums.h"
#include "Vec.h"

#include "config.h"

// TODO: decide when to update this (looks like WM_DISPLAYCHANGE message in WindowProc).
// whenever calculating, determine size based on whether taskbar is also present (will also need to account for
// orientation and position of it too)
struct Desktop {
    // Upper left point and screen-size
    Rect rect;
    Vec::vec_t margin = Config::InitialMargin;
    float primary_stack_proportion = Config::InitialPrimaryStackProportion;

    Enums::ViewType view = Config::InitialView;
    Enums::ViewType secondary_view = Config::InitialSecondaryView;

    Config::StackGridOrientation grid_orientation = Config::InitialStackGridOrientation;

    void updateRect(Rect rect);
};
