#pragma once

namespace {
const float C_PI = 3.14159265359f;
const float C_2PI = 6.28318530718f;

template <typename type_t>
void swapv(type_t & a, type_t & b) {
    type_t c = a; a = b; b = c;
}

template <typename type_t>
constexpr float lerp(type_t x, type_t y, type_t i)
{
    return x + (y - x) * i;
}

constexpr float fpart(const float& in)
{
    return in - float(int(in));
}

template <typename type_t>
constexpr type_t min2(const type_t a,
            const type_t b) {
    return (a<b) ? a : b;
}

template <typename type_t>
constexpr type_t min3(const type_t a,
            const type_t b,
            const type_t c) {
    return min2(min2(a, b), c);
}

template <typename type_t>
constexpr type_t max2(const type_t a,
            const type_t b) {
    return (a>b) ? a : b;
}

template <typename type_t>
constexpr type_t max3(const type_t a,
            const type_t b,
            const type_t c) {
    return max2(max2(a, b), c);
}

template <typename type_t>
constexpr type_t smoothstep(const type_t x)
{
    return x * x * (3 - 2 * x);
}

template <typename type_t>
constexpr type_t clampv(const type_t lo, const type_t in, const type_t hi)
{
    return (in < lo) ? lo : (in > hi) ? hi : in;
}

float epsilon(float a, float b) {
    const float C_EPSILON = 0.00001f;
    return (a > b - C_EPSILON && a < b + C_EPSILON);
}
} // namespace {}
