#pragma once

#include <cstdint>
#include <type_traits>

struct Vec {
	using vec_t = std::int32_t;
	vec_t x;
	vec_t y;

	Vec& operator +=(const Vec& rhs) {
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}

	Vec& operator -=(const Vec& rhs) {
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}

	template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
	Vec& operator *=(T rhs) {
		this->x = static_cast<vec_t>(rhs * static_cast<T>(this->x));
		this->y = static_cast<vec_t>(rhs * static_cast<T>(this->y));
		return *this;
	}
};
inline Vec operator +(Vec lhs, const Vec& rhs) {
	return (lhs += rhs);
}
inline Vec operator -(Vec lhs, const Vec& rhs) {
	return (lhs -= rhs);
}
template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
inline Vec operator *(T a, Vec b) {
	return (b *= a);
}
template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>>
inline Vec operator *(Vec b, T a) {
	return (b *= a);
}

struct Rect {
	Vec upper_left;
	Vec dimensions;
};