#pragma once

#include <cassert>
#include <cstdint>

namespace tengu {
namespace {
const float C_PI = 3.14159265359f;
const float C_2PI = 6.28318530718f;

constexpr int32_t ipartv(float v)
{
    return int32_t(v);
}

template <typename type_t>
constexpr type_t signv(const type_t v)
{
    return (v == type_t(0)) ? type_t(0) : ((v > type_t(0)) ? type_t(1) : type_t(-1));
}

template <typename type_t>
void swapv(type_t& a, type_t& b)
{
    type_t c = a;
    a = b;
    b = c;
}

constexpr float fpart(const float& in)
{
    return in - float(int(in));
}

template <typename type_t>
constexpr type_t absv(const type_t v)
{
    return (v < 0) ? -v : v;
}

template <typename type_t>
constexpr type_t minv(type_t a,
    type_t b)
{
    return (a < b) ? a : b;
}

template <typename type_t>
constexpr type_t maxv(type_t a,
    type_t b)
{
    return (a > b) ? a : b;
}

template <typename type_t>
constexpr type_t minv(type_t a,
    type_t b,
    type_t c)
{
    return minv(minv(a, b), c);
}

template <typename type_t>
constexpr type_t maxv(type_t a,
    type_t b,
    type_t c)
{
    return maxv(maxv(a, b), c);
}

template <typename type_t>
constexpr type_t minv(type_t a,
    type_t b,
    type_t c,
    type_t d)
{
    return minv(minv(a, b), minv(c, d));
}

template <typename type_t>
constexpr type_t maxv(type_t a,
    type_t b,
    type_t c,
    type_t d)
{
    return maxv(maxv(a, b), maxv(c, d));
}

template <typename type_t>
constexpr type_t smoothstep(const type_t x)
{
    return x * x * (3 - 2 * x);
}

template <typename type_t>
constexpr float lerp(type_t x, type_t y, type_t i)
{
    return x + (y - x) * i;
}

template <typename type_t>
constexpr float slerp(type_t x, type_t y, type_t i)
{
    return x + (y - x) * smoothstep(i);
}

template <typename type_t>
constexpr type_t clampv(const type_t lo, const type_t in, const type_t hi)
{
    return (in < lo) ? lo : (in > hi) ? hi : in;
}

float fp_cmp(const float a, const float b)
{
    const float C_EPSILON = 0.0001f;
    return (a > b - C_EPSILON && a < b + C_EPSILON);
}

template <typename type_t>
constexpr type_t falloff(const type_t x, const type_t r)
{
    return clampv<type_t>(0, 1 - (x * x) / (r * r), 1);
}

// cubic interpolation
// 0     1 --> 2     3
float cubic(const float* v, float x)
{
    const float &a = v[0], &b = v[1], &c = v[2], &d = v[3];
    const float p = (d - c) - (a - b);
    return p * x * x * x + ((a - b) - p) * x * x + (c - a) * x + b;
}

template <typename type_t>
constexpr type_t quantize(const type_t in, const type_t divisor)
{
    return ((in < 0) ? (in - divisor) : in) / divisor;
}

template <typename type_t>
constexpr type_t falloff(const type_t lo, const type_t in, const type_t hi)
{
    return clampv(0.f, (in - lo) / (hi - lo), 1.f);
}

// todo: replace with weak_t
template <typename type_t>
struct valid_t {

    valid_t()
        : valid_(false)
    {
    }

    valid_t(type_t& data)
        : valid_(true)
        , data_(data)
    {
    }

    type_t* operator->()
    {
        assert(valid_);
        return &data_;
    }

    const type_t* operator->() const
    {
        assert(valid_);
        return &data_;
    }

    bool valid() const
    {
        return valid_;
    }

    type_t get()
    {
        return data_;
    }

    const type_t get() const
    {
        return data_;
    }

    void operator=(const type_t& rhs)
    {
        data_ = rhs;
        valid_ = true;
    }

protected:
    bool valid_;
    type_t data_;
};

} // namespace {}
} // namespace tengu
