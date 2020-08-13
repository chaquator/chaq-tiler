#include <cstdint>

struct Vec {
	using vec_t = std::int32_t;
	vec_t x;
	vec_t y;
};
inline Vec operator +(Vec a, Vec b) {
	return { a.x + b.x, a.y + b.y };
}
inline Vec operator -(Vec a, Vec b) {
	return { a.x - b.x, a.y - b.y };
}
template <typename Scalar>
inline static Vec mul(Scalar scalar, Vec p) {
	return {
		static_cast<Vec::vec_t>(scalar * static_cast<Scalar>(p.x)),
		static_cast<Vec::vec_t>(scalar * static_cast<Scalar>(p.y))
	};
}
template <typename Scalar>
inline Vec operator *(Scalar a, Vec b) {
	return mul(a, b);
}
template <typename Scalar>
inline Vec operator *(Vec b, Scalar a) {
	return mul(a, b);
}