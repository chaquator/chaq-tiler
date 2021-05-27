#include "Globals.h"

// Globals
namespace Globals {

// runtime globals
std::vector<Window> Windows;
decltype(Windows)::const_iterator WindowPartitionPoint;
decltype(Windows)::const_iterator WindowFocusPoint;

// setup globals
std::vector<Desktop> Desktops;

} // namespace Globals
