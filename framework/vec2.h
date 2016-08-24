#pragma once
#include <cmath>

namespace
{
    float sqrt(const float & x)
    {
        return sqrtf(x);
    }

    float isqrt(const float & val)
    {
        const float threehalfs = 1.5f;
        float x2 = val * 0.5f;
        float y  = val;
        long i  = *(long*) &y;
        i  = 0x5f3759df - (i >> 1);
        y  = * (float *) &i;
        y  = y * (threehalfs - (x2 * y * y));
        return y;
    }
}

template <typename type_t>
struct vec2_t
{
    type_t x, y;

    void operator += (const vec2_t & v)
    {
        x += v.x;
        y += v.y;
    }

    void operator -= (const vec2_t & v)
    {
        x -= v.x;
        y -= v.y;
    }

    void operator *= (const type_t v)
    {
        x *= v;
        y *= v;
    }

    void operator /= (const type_t v)
    {
        x /= v;
        y /= v;
    }

    static type_t length(const vec2_t & v)
    {
        return sqrt(v.x*v.x + v.y*v.y);
    }

    static vec2_t normalize(const vec2_t & v)
    {
        const type_t l = length(v);
        return vec2_t {
            v.x / l,
            v.y / l,
        };
    }

    static vec2_t cross(const vec2_t & a)
    {
        return vec2_t { a.y, -a.x };
    }

    static type_t distance(
        const vec2_t & a,
        const vec2_t & b)
    {
        return length(b - a);
    }

    static vec2_t zero() {
        return vec2_t {
            0, 0
        };
    }

    // project v onto u
    static vec2_t project(
        const vec2_t & u,
        const vec2_t & v)
    {
        const type_t vu = v * u;
        const type_t uu = u * u;
        return u * (vu / uu);
    }

    static vec2_t rotate(
        const vec2_t & v,
        const type_t angle)
    {
        const type_t s = sinf(angle);
        const type_t c = cosf(angle);
        return vec2_t {
            c * v.x + s * v.y,
            c * v.y - s * v.x
        };
    }

    static vec2_t lerp(
        const vec2_t & a,
        const vec2_t & b,
        const type_t i
    )
    {
        return a + (b - a) * i;
    }
};

namespace {

    template <typename type_t>
    vec2_t<type_t> operator + (
        const vec2_t<type_t> & a,
        const vec2_t<type_t> & b)
    {
        return vec2_t<type_t> {
            a.x + b.x,
            a.y + b.y
        };
    }

    template <typename type_t>
    vec2_t<type_t> operator - (
        const vec2_t<type_t> & a,
        const vec2_t<type_t> & b)
    {
        return vec2_t<type_t> {
            a.x - b.x,
            a.y - b.y
        };
    }

    template <typename type_t>
    type_t operator * (
        const vec2_t<type_t> & a,
        const vec2_t<type_t> & b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template <typename type_t>
    vec2_t<type_t> operator * (
        const vec2_t<type_t> & a,
        const type_t s)
    {
        return vec2_t<type_t> {
            a.x * s,
            a.y * s
        };
    }

    template <typename type_t>
    vec2_t<type_t> operator * (
        const type_t s,
        const vec2_t<type_t> & a)
    {
        return vec2_t<type_t> {
            a.x * s,
            a.y * s
        };
    }

    template <typename type_t>
    vec2_t<type_t> operator / (
        const vec2_t<type_t> & a,
        const type_t s)
    {
        return vec2_t<type_t> {
            a.x / s,
            a.y / s
        };
    }

    template <typename type_t>
    vec2_t<type_t> operator - (
        const vec2_t<type_t> & a)
    {
        return vec2_t<type_t> {
            -a.x,
            -a.y
        };
    }

    typedef vec2_t<float> vec2f_t;

} // namespace {}
