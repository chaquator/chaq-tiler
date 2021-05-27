#pragma once
#include <vector>
#include <windows.h>

#include "Desktop.h"
#include "Window.h"

// Globals
namespace Globals {

// runtime globals
extern std::vector<Window> Windows;
extern decltype(Windows)::const_iterator WindowPartitionPoint;
extern decltype(Windows)::const_iterator WindowFocusPoint;

// setup globals
extern std::vector<Desktop> Desktops;

} // namespace Globals
