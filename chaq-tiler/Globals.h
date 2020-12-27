#pragma once
#include <vector>
#include <windows.h>

#include "Desktop.h"
#include "Window.h"

// Globals
namespace Globals {

// runtime globals
std::vector<Window> Windows;
decltype(Windows)::const_iterator WindowPartitionPoint;
decltype(Windows)::const_iterator WindowFocusPoint;

// setup globals
HMONITOR PrimaryMonitor;
Desktop PrimaryDesktop;

} // namespace Globals
