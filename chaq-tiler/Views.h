#pragma once

#include <Windows.h>
#include <algorithm>
#include <iterator>
#include <type_traits>

#include "Desktop.h"
#include "Enums.h"
#include "Globals.h"
#include "Vec.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace Views {

// integer ceil div (positive)
template <typename T1, typename T2>
static std::common_type_t<T1, T2> ceil(T1 a, T2 b) {
    return (a / b) + (((a % b) != 0) ? 1 : 0);
}

// min
template <typename T1, typename T2>
static std::common_type_t<T1, T2> min(T1 a, T2 b) {
    return (a > b) ? b : a;
}

// max
template <typename T1, typename T2>
static std::common_type_t<T1, T2> max(T1 a, T2 b) {
    return (a > b) ? a : b;
}

// Cascading view, breaks cascade into multiple to make windows fit vertically if necessary
// Mainly used for testing
template <typename Iterator>
void Cascade(const Iterator start, const Iterator end, const Desktop& desktop) {
    using DiffType = typename std::iterator_traits<Iterator>::difference_type;

    DiffType size = std::distance(start, end);
    if (size == 0) return;

    // Customize cascade here
    constexpr float window_factor = 0.5f; // Window will be 50% dimension of screen
    Vec cascade_delta = {
        // static_cast<Vec::vec_t>(0.2f * static_cast<float>(desktop.margin)), // Less x increase with every y increase
        max(10, desktop.margin), max(10, desktop.margin)};

    Rect window_rect{
        desktop.rect.upper_left + Vec{desktop.margin, desktop.margin}, // Upper-left
        window_factor * desktop.rect.dimensions,                       // Dimensions
    };

    // Calculate, accounting for margin, how many cascades are needed for the screen
    Vec::vec_t working_desktop_height = desktop.rect.dimensions.y - 2 * cascade_delta.y - window_rect.dimensions.y;
    // cascade height only considering the margin
    Vec::vec_t cascade_total_height = (static_cast<Vec::vec_t>(size - 1) * cascade_delta.y);
    // amount of cascades
    std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height);
    // std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); //
    // remaining height

    // # of windows to draw = ceil(desktop_height / window_height)
    DiffType windows_per_full_cascade = ceil(working_desktop_height, cascade_delta.y);

    auto single_cascade = [&desktop, &cascade_delta](DiffType amount, Iterator start, std::size_t current_cascade,
                                                     Rect base_rect) {
        base_rect.upper_left +=
            Vec{static_cast<Vec::vec_t>(current_cascade) * (base_rect.dimensions.x + cascade_delta.x), 0};

        std::for_each_n(start, amount, [&base_rect, &cascade_delta](const auto& window) {
            window.SetPos(base_rect);
            base_rect.upper_left += cascade_delta;
        });
    };

    // Full cascades
    std::size_t current_cascade = 0;
    Iterator current_window = start;
    while (current_cascade < cascades) {
        single_cascade(windows_per_full_cascade, current_window, current_cascade, window_rect);

        current_window += windows_per_full_cascade;
        ++current_cascade;
    }

    // Leftovers
    DiffType remaining = std::distance(current_window, end);
    assert(remaining <= windows_per_full_cascade);
    single_cascade(remaining, current_window, current_cascade, window_rect);
}

// Traditional dwm-like stack
template <typename Iterator>
void TileStack(const Iterator start, const Iterator end, const Desktop& desktop) {
    auto size = std::distance(start, end);
    auto primary_max = desktop.stack_dimensions.x * desktop.stack_dimensions.y;
    bool do_secondary = size > primary_max;

    auto& orientation = desktop.grid_orientation;
    // draw grid (margin only between tile strips, not around borders)
    /*
    auto draw_primary = [&primary_max, &orientation, &dim = desktop.stack_dimensions, &margin = desktop.margin](
                            const Iterator start, const Iterator end, const Rect& area) -> Iterator {
        // axis: bit 0, primary direction: bit 1, secondary direction: bit 2
        auto axis = orientation.test(0);
        auto primary_reverse = orientation.test(1);
        auto secondary_reverse = orientation.test(2);

        auto dist = min(std::distance(start, end), primary_max);
        auto strip_length = component_of_interest(axis, dim);
        auto num_strips = ceil(dist, strip_length);

        Rect cur_rect = area;
        auto& ul_component = component_of_interest(!axis, cur_rect.upper_left);
        auto& size_component = component_of_interest(!axis, cur_rect.dimensions);
        size_component += margin;
        size_component /= static_cast<Vec::vec_t>(num_strips);
        size_component -= margin;

        auto ul_offset = size_component + margin;
        if (secondary_reverse) {
            ul_component += component_of_interest(!axis, area.dimensions) - size_component;
            ul_offset = -ul_offset;
        }

        auto cur = start;
        auto nend = start + dist;

        while (cur != nend) {
            // draw strip
            auto inc = min(std::distance(cur, nend), strip_length);
            cur = tile_strip(cur, cur + inc, cur_rect, margin, axis, primary_reverse);
            ul_component += ul_offset;
        }
    };
    */
    // draw_grid(start, end, area, desktop.margin, orientation, desktop.stack_dimensions);

    // get size for primary area of the important dimension
    auto& primary_secondary_dimension = component_of_interest(orientation.test(3), desktop.rect.dimensions);
}

// select comopnent as reference given axis
template <typename Vector>
static decltype(auto) component_of_interest(bool axis, Vector& v) {
    return axis ? (v.x) : (v.y);
}

template <typename Iterator>
static Iterator draw_grid(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin,
                          const Config::StackGridOrientation& orientation, const Vec& stack_dimensions) {
    // axis: bit 0, primary direction: bit 1, secondary direction: bit 2
    auto axis = orientation.test(0);
    auto primary_reverse = orientation.test(1);
    auto secondary_reverse = orientation.test(2);

    auto primary_max = stack_dimensions.x * stack_dimensions.y;

    auto dist = min(std::distance(start, end), primary_max);
    auto strip_length = component_of_interest(axis, stack_dimensions);
    auto num_strips = ceil(dist, strip_length);

    Rect cur_rect = area;
    auto& ul_component = component_of_interest(!axis, cur_rect.upper_left);
    auto& size_component = component_of_interest(!axis, cur_rect.dimensions);
    size_component += margin;
    size_component /= static_cast<Vec::vec_t>(num_strips);
    size_component -= margin;

    auto ul_offset = size_component + margin;
    if (secondary_reverse) {
        ul_component += component_of_interest(!axis, area.dimensions) - size_component;
        ul_offset = -ul_offset;
    }

    auto cur = start;
    auto nend = start + dist;

    while (cur != nend) {
        // draw strip
        auto inc = min(std::distance(cur, nend), strip_length);
        cur = tile_strip(cur, cur + inc, cur_rect, margin, axis, primary_reverse);
        ul_component += ul_offset;
    }
}

// Helper function to draw single tiled strip with parameterized orientation and direction
// No margins on the sides, only margin between windows
// Axis: false -> X, true -> Y
// Reverse: false -> negative, true -> positive
template <typename Iterator>
static Iterator tile_strip(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin, bool axis,
                           bool reverse) {
    Vec working_size = area.dimensions;

    Rect window_rect{
        area.upper_left, // Upper-left
        working_size,    // Dimensions
    };

    // Set correct window size so that edges have no margin, only margin between windows in strip
    // L+M = N*(S+M) --> S = (L+M)/N - M
    // the reason to have L+M is because one area of margin to the right (or downwards) will not be used
    auto& size_component = component_of_interest(axis, window_rect.dimensions);
    size_component += margin;
    size_component /= static_cast<Vec::vec_t>(std::distance(start, end));
    size_component -= margin;

    // Upper left component of interest
    auto& current_ul_component = component_of_interest(axis, window_rect.upper_left);
    auto ul_offset = size_component + margin;

    // reverse starts from other side and moves backwards
    if (reverse) {
        ul_offset = -ul_offset;
        current_ul_component += component_of_interest(axis, working_size) - size_component;
    }

    // Tile windows
    std::for_each(start, end, [&window_rect, &current_ul_component, &ul_offset](const auto& window) {
        window.SetPos(window_rect);
        current_ul_component += ul_offset;
    });

    // Function will be used like Iterator current_window = tile_strip(...);
    return end;
}

template <typename Iterator>
static Iterator monocle_area(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin,
                             bool reverse) {
    auto size = std::distance(start, end);
    if (size == 0) return end;

    // different behavior based on whether range includes current focused window
    // relying on global data to change the behavior of a function, hate to see it
    auto draw_window = start;
    auto& f = Globals::WindowFocusPoint;
    auto l = std::distance(start, f);
    auto r = std::distance(f, end);
    if (l >= 0 && r >= 0) {
        // focus point is within range
        draw_window = f;
    }

    Rect window_rect = area;
    window_rect.dimensions -= 2 * Vec{margin, margin};
    window_rect.upper_left += Vec{margin, margin};

    Iterator current = start;
    while (current != end) {
        auto& window = *current;
        /*
        HWND after = ((current + 1) == end) ? HWND_TOP : (current + 1)->handle;
        HWND before = (current == start) ? HWND_BOTTOM : (current - 1)->handle;
        */

        window.SetPos(window_rect, HWND_TOP, current != draw_window);

        ++current;
    }

    return end;
}

} // namespace Views
