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
    using c_t = std::common_type_t<T1, T2>;
    auto ca = static_cast<c_t>(a);
    auto cb = static_cast<c_t>(b);
    return (ca > cb) ? cb : ca;
}

// max
template <typename T1, typename T2>
static std::common_type_t<T1, T2> max(T1 a, T2 b) {
    using c_t = std::common_type_t<T1, T2>;
    auto ca = static_cast<c_t>(a);
    auto cb = static_cast<c_t>(b);
    return (ca > cb) ? ca : cb;
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
    // std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height);
    // remaining height

    // # of windows to draw = ceil(desktop_height / window_height)
    DiffType windows_per_full_cascade = ceil(working_desktop_height, cascade_delta.y);

    auto single_cascade = [&desktop, &cascade_delta](DiffType amount, Iterator start, std::size_t current_cascade,
                                                     Rect base_rect) {
        base_rect.upper_left +=
            Vec{static_cast<Vec::vec_t>(current_cascade) * (base_rect.dimensions.x + cascade_delta.x), 0};

        std::for_each_n(start, amount, [&base_rect, &cascade_delta](const auto& window) {
            window.SetPos(base_rect, HWND_BOTTOM);
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
    auto primary_max = desktop.grid_dimensions.x * desktop.grid_dimensions.y;
    bool do_secondary = size > primary_max;

    auto apply_margin = [&margin = desktop.margin](auto& v) {
        v.upper_left += Vec{margin, margin};
        v.dimensions -= 2 * Vec{margin, margin};
    };

    Rect primary_rect = desktop.rect;
    if (do_secondary) {
        auto primary_secondary_axis = desktop.grid_orientation.test(Config::Orientation::PrimarySecondaryAxis);
        auto primary_secondary_direction =
            desktop.grid_orientation.test(Config::Orientation::PrimarySecondaryDirection);
        auto secondary_direction = desktop.grid_orientation.test(Config::Orientation::SecondaryDirection);

        // Scale primary and secondary areas according to proportion
        auto& primary_rect_dim = component_of_interest(primary_secondary_axis, primary_rect.dimensions);
        primary_rect_dim =
            static_cast<Vec::vec_t>(static_cast<float>(primary_rect_dim) * desktop.primary_stack_proportion);

        Rect secondary_rect = desktop.rect;
        auto& secondary_rect_dim = component_of_interest(primary_secondary_axis, secondary_rect.dimensions);
        secondary_rect_dim = component_of_interest(primary_secondary_axis, desktop.rect.dimensions) - primary_rect_dim;

        // Offset primary or secondary area according to direction
        if (primary_secondary_direction) {
            auto& primary_ul = component_of_interest(primary_secondary_axis, primary_rect.upper_left);
            primary_ul = secondary_rect_dim;
        } else {
            auto& secondary_ul = component_of_interest(primary_secondary_axis, secondary_rect.upper_left);
            secondary_ul = primary_rect_dim;
        }

        // Then apply margin offsets
        apply_margin(primary_rect);
        apply_margin(secondary_rect);

        // Draw primary and secondary views
        Iterator cur = start;
        cur = draw_grid(start, start + primary_max, primary_rect, desktop.margin, desktop.grid_orientation,
                        desktop.grid_dimensions);

        switch (desktop.secondary_view) {
        case Enums::ViewType::TileStack: {
            auto remaining = std::distance(cur, end);
            auto s_dist = min(remaining, desktop.secondary_max_windows);
            auto s_end = cur + s_dist;
            tile_strip(cur, s_end, secondary_rect, desktop.margin, !primary_secondary_axis, secondary_direction);

            // Hide remaining windows
            std::for_each(s_end, end,
                          [&secondary_rect](const auto& w) { w.SetPos(secondary_rect, HWND_BOTTOM, true); });
        } break;
        case Enums::ViewType::Monocle:
            monocle_area(cur, end, secondary_rect, desktop.margin, secondary_direction);
            break;
        }

    } else {
        apply_margin(primary_rect);
        draw_grid(start, end, primary_rect, desktop.margin, desktop.grid_orientation, desktop.grid_dimensions);
    }
}

// select comopnent as an lvalue reference given the axis
template <typename Vector>
static decltype(auto) component_of_interest(bool axis, Vector& v) {
    return axis ? (v.y) : (v.x);
}

template <typename Iterator>
static Iterator draw_grid(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin,
                          const Config::StackGridOrientation& orientation, const Vec& grid_dimensions) {
    auto axis = orientation.test(Config::Orientation::GridAxis);
    auto primary_reverse = orientation.test(Config::Orientation::GridPrimaryDirection);
    auto secondary_reverse = orientation.test(Config::Orientation::GridSecondaryDirection);

    auto primary_max = grid_dimensions.x * grid_dimensions.y;

    auto dist = min(std::distance(start, end), primary_max);
    auto strip_length = component_of_interest(axis, grid_dimensions);
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

    return nend;
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
        window.SetPos(window_rect, HWND_TOP);
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
    auto draw_window = reverse ? end : start;
    auto& f = Globals::WindowFocusPoint;
    auto l = std::distance(start, f);
    auto r = std::distance(f, end);
    if (l >= 0 && r >= 0) {
        // focus point is within range
        draw_window = f;
    }

    Iterator current = start;
    while (current != end) {
        auto& window = *current;

        window.SetPos(area, HWND_TOP, current != draw_window);

        ++current;
    }

    return end;
}

} // namespace Views
