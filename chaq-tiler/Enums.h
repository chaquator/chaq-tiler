#pragma once

namespace Enums {
	enum class Orientation {
		Vertical,
		Horizontal
	};

	enum class Display {
		Monocle,
		TileStack
	};

	// Initial direction of the primary stack
	enum class MajorDirection {
		PositiveX, // Starts on the left side
		NegativeX, // Starts on the right side
		PositiveY, // Starts on the upper side
		NegativeY // Starts on the lower side
	};

	// Secondary direction of the primary stack (will be direction of the unchosen component)
	enum class SubDirection {
		Positive, // If primary is some X, positive means down. If primary is some Y, positive means right
		Negative // If primary is some X, negative means up. If primary is some Y, negative means left
	};
}