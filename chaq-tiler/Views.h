#pragma once

#include <iterator>
#include <algorithm>
#include <Windows.h>

#include "Vec.h"
#include "Desktop.h"

class Views {
	enum class Orientation {
		Vertical,
		Horizontal
	};

	template<Views::Orientation orientation, typename Iterator>
	static Iterator tile_strip(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin, bool reverse);

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

	Rect window_rect {
		desktop.rect.upper_left + Vec{ desktop.margin, desktop.margin }, // Upper-left
		window_factor * desktop.rect.dimensions // Dimensions
	};

	// Calculate, accounting for margin, how many cascades are needed for the screen
	Vec::vec_t working_desktop_height = desktop.rect.dimensions.y - 2 * cascade_delta.y - window_rect.dimensions.y;
	Vec::vec_t cascade_total_height = (static_cast<Vec::vec_t>(size - 1) * cascade_delta.y); // cascade height only considering the margin
	std::size_t cascades = static_cast<std::size_t>(cascade_total_height / working_desktop_height); // amount of cascades
	// std::size_t cascade_leftover = static_cast<std::size_t>(cascade_total_height % working_desktop_height); // remaining height

	// # of windows to draw = ceil(desktop_height / window_height)
	DiffType windows_per_full_cascade = static_cast<DiffType>((working_desktop_height / cascade_delta.y)) + ((working_desktop_height % cascade_delta.y != 0) ? 1 : 0);

	auto single_cascade = [&desktop, &cascade_delta] (DiffType amount, Iterator start, std::size_t current_cascade, Rect base_rect) {
		base_rect.upper_left += Vec { static_cast<Vec::vec_t>(current_cascade) * (base_rect.dimensions.x + cascade_delta.x), 0 };

		for (DiffType index = 0; index < amount; ++index, ++start) {
			start->SetPos(base_rect);

			base_rect.upper_left += cascade_delta;
		}
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
void Views::primary_secondary_stack(const Iterator start, const Iterator end, const Desktop& desktop) {
	using DiffType = typename std::iterator_traits<Iterator>::difference_type;

	tile_strip<Orientation::Vertical>(start, end, desktop.rect, desktop.margin, false);
}

// Helper function to draw single tiled strip with parameterized orientation and direction
// reverse = true --> drawn right-to-left or down-to-up depending on orientation (vertical or horizontal)
template<Views::Orientation orientation, typename Iterator>
Iterator Views::tile_strip(const Iterator start, const Iterator end, const Rect& area, Vec::vec_t margin, bool reverse) {
	// Returns reference to component of interest for the given orientation (which component to divide in length and tile along)
	auto component_of_interest = [](auto& vec) constexpr -> auto& {	return orientation == Views::Orientation::Horizontal ? vec.x : vec.y; };

	Vec working_size = area.dimensions - (2 * Vec { margin, margin });

	Rect window_rect {
		area.upper_left + Vec{ margin, margin }, // Upper-left
		working_size // Dimensions
	};

	// Set correct window size
	auto& size_component = component_of_interest(window_rect.dimensions);
	size_component /= static_cast<Vec::vec_t>(std::distance(start, end));
	size_component -= margin;

	// Upper left component of interest
	auto& current_ul_component = component_of_interest(window_rect.upper_left);
	// Reverse will start from other side
	if (reverse) {
		current_ul_component += component_of_interest(working_size) - size_component;
	}
	auto ul_offset = (reverse) ? -(size_component + margin) : (size_component + margin);

	// Tile windows
	std::for_each(start, end, [&window_rect, &current_ul_component, &ul_offset] (const auto& window) {
		window.SetPos(window_rect);

		// Reverse travels backwards
		current_ul_component += ul_offset;
	});

	// Function will be used like Iterator current_window = tile_strip(...);
	return end;
}