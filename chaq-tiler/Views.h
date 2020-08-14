#pragma once

#include <iterator>
#include <Windows.h>

#include "Vec.h"
#include "Desktop.h"

class Views {
	enum class Orientation {
		Vertical,
		Horizontal
	};

	template<typename Iterator, Orientation orientation, bool reverse>
	static Iterator tile_strip(const Iterator start, const Iterator end, const Rect& area, int margin);

public:
	template <typename Iterator>
	static void cascade(const Iterator start, const Iterator end, const Desktop& desktop);

	template <typename Iterator>
	static void primary_secondary_stack(const Iterator start, const Iterator end, const Desktop& desktop);
};

// Cascading view, breaks cascade into multiple to make windows fit vertically if necessary
// Mainly used for testing
template <typename Iterator>
void Views::cascade(const Iterator start, const Iterator end, const Desktop& desktop) {
	using DiffType = typename std::iterator_traits<Iterator>::difference_type;

	DiffType size = std::distance(start, end);
	if (size == 0) return; // Short circuit break

	// Customize cascade here
	constexpr float window_factor = 0.5f; // Window will be 50% dimension of screen
	Vec cascade_delta = {
		// static_cast<Vec::vec_t>(0.2f * static_cast<float>(desktop.margin)), // Less x increase with every y increase
		max(10, desktop.margin),
		max(10, desktop.margin)
	};

	Vec window_dimensions = window_factor * desktop.rect.dimensions;

	// Calculate, accounting for margin, how many cascades are needed for the screen
	Vec::vec_t working_desktop_height = desktop.rect.dimensions.y - 2 * cascade_delta.y - window_dimensions.y;
	Vec::vec_t cascade_total_height = (static_cast<Vec::vec_t>(size - 1) * cascade_delta.y); // cascade height only considering the margin
	std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height); // amount of cascades
	// std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); // remaining height

	// # of windows to draw = ceil(desktop_height / window_height)
	DiffType windows_per_full_cascade = static_cast<DiffType>((working_desktop_height / cascade_delta.y)) + ((working_desktop_height % cascade_delta.y != 0) ? 1 : 0);

	auto single_cascade = [&desktop, &window_dimensions, &cascade_delta] (DiffType amount, Iterator start, std::size_t current_cascade) {
		for (DiffType index = 0; index < amount; ++index, ++start) {
			/*
			Point upper_left = desktop.rect.upper_left + { desktop.margin, desktop.margin };
			Point cascade_offset = { static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x), 0 };
			Point base = upper_left + cascade_offset;
			Point travel = static_cast<Vec::vec_t>(index) * cascade_delta;
			Point pos = base + travel;
			*/
			Vec pos = {
				desktop.rect.upper_left.x + static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(current_cascade) * (window_dimensions.x + cascade_delta.x) + static_cast<Vec::vec_t>(index) * cascade_delta.x,
				desktop.rect.upper_left.y + static_cast<Vec::vec_t>(desktop.margin) + static_cast<Vec::vec_t>(index) * cascade_delta.y
			};

			::SetWindowPos(start->handle,
				HWND_BOTTOM,
				static_cast<int>(pos.x), static_cast<int>(pos.y),
				static_cast<int>(window_dimensions.x), static_cast<int>(window_dimensions.y),
				SWP_NOACTIVATE
			);
		}
	};

	// Full cascades
	std::size_t current_cascade = 0;
	Iterator current_window = start;
	while (current_cascade < cascades) {
		single_cascade(windows_per_full_cascade, current_window, current_cascade);

		current_window += windows_per_full_cascade;
		++current_cascade;
	}

	// Leftovers
	DiffType remaining = std::distance(current_window, end);
	assert(remaining <= windows_per_full_cascade);
	single_cascade(remaining, current_window, current_cascade);
}

// Traditional dwm-like stack
template <typename Iterator>
void Views::primary_secondary_stack(const Iterator start, const Iterator end, const Desktop& desktop) {
	using DiffType = typename std::iterator_traits<Iterator>::difference_type;
}

// Helper function to draw single tiled strip with parameterized orientation and direction
// reverse = true --> drawn right-to-left or down-to-up depending on orientation (vertical or horizontal)
template<typename Iterator, Views::Orientation orientation, bool reverse>
Iterator Views::tile_strip(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin) {

	// Returns reference to component of interest for the given orientation (which component to divide in length and tile along)
	auto component_of_interest = [&orientation] (const Vec& vec) constexpr -> Vec::vec_t& {
		return orientation == Views::Orientation::Horizontal ? vec.x : vec.y;
	};

	Vec working_size = area.dimensions - (2 * Vec { margin, margin });

	// Window size (doesn't subtract margin)
	Vec window_dimensions = working_size;
	// TODO: find out if type needs to be auto& or auto (i think it's the latter maybe)
	auto& divided_component = component_of_interest(window_dimensions);
	divided_component /= static_cast<Vec::vec_t>(std::distance(start, end));
	divided_component -= margin;

	Vec current_ul = area.upper_left + Vec { margin, margin };
	auto& current_ul_component = component_of_interest(current_ul);

	Iterator current = start;
	while (current != end) {
		::SetWindowPos(start->handle,
			HWND_BOTTOM,
			static_cast<int>(current_ul.x), static_cast<int>(current_ul.y),
			static_cast<int>(window_dimensions.x), static_cast<int>(window_dimensions.y),
			SWP_NOACTIVATE
		);

		current_ul_component += divided_component + margin;
		++current;
	}

	// Function will be used like Iterator current_window = tile_strip(...);
	return end;
}