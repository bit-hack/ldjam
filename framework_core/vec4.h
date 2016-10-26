#pragma once
#include <cmath>

namespace tengu {
template <typename type_t>
struct vec4_t {
    type_t x, y, z, w;

    static float sqrt(const float& x)
    {
        return sqrtf(x);
    }

    static float isqrt(const float& val)
    {
        const float threehalfs = 1.5f;
        float x2 = val * 0.5f;
        float y = val;
        long i = *(long*)&y;
        i = 0x5f3759df-(i>>1);
        y = *(float*)&i;
        y = y * (threehalfs-(x2 * y * y));
        return y;
    }

    void operator+=(const vec4_t& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
    }

    void operator-=(const vec4_t& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
    }

    void operator*=(const type_t v)
    {
        x *= v;
        y *= v;
        z *= v;
        w *= v;
    }

    void operator/=(const type_t v)
    {
        x /= v;
        y /= v;
        z /= v;
        w /= v;
    }

    static type_t length(const vec4_t& v)
    {
        return sqrt(v.x * v.x+v.y * v.y+v.z * v.z+v.w * v.w);
    }

    static vec4_t normalize(const vec4_t& v)
    {
        const type_t l = length(v);
        return vec4_t{
            v.x/l,
            v.y/l,
            v.z/l,
            v.w/l
        };
    }

    static type_t distance(
        const vec4_t& a,
        const vec4_t& b)
    {
        return length(b-a);
    }

    static vec4_t zero()
    {
        return vec4_t{
            0, 0, 0, 0
        };
    }

    // project v onto u
    static vec4_t project(
        const vec4_t& u,
        const vec4_t& v)
    {
        const type_t vu = v * u;
        const type_t uu = u * u;
        return u * (vu/uu);
    }

    template <typename other_t>
    static vec4_t cast(const vec4_t<other_t> & a) {
        return vec4_t{
            type_t(a.x),
            type_t(a.y),
            type_t(a.z),
            type_t(a.w)
        };
    }

    static vec4_t lerp(
        const vec4_t& a,
        const vec4_t& b,
        const type_t i)
    {
        return a+(b-a) * i;
    }
};

namespace {

template <typename type_t>
vec4_t<type_t> operator+(
    const vec4_t<type_t>& a,
    const vec4_t<type_t>& b)
{
    return vec4_t<type_t>{
        a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w
    };
}

template <typename type_t>
vec4_t<type_t> operator-(
    const vec4_t<type_t>& a,
    const vec4_t<type_t>& b)
{
    return vec4_t<type_t>{
        a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w
    };
}

template <typename type_t>
type_t operator*(
    const vec4_t<type_t>& a,
    const vec4_t<type_t>& b)
{
    return a.x * b.x+a.y * b.y+a.z * b.z + a.w * b.w;
}

template <typename type_t>
vec4_t<type_t> operator*(
    const vec4_t<type_t>& a,
    const type_t s)
{
    return vec4_t<type_t>{
        a.x * s, a.y * s, a.z * s, a.w * s
    };
}

template <typename type_t>
vec4_t<type_t> operator*(
    const type_t s,
    const vec4_t<type_t>& a)
{
    return vec4_t<type_t>{
        a.x * s, a.y * s, a.z * s, a.w * s
    };
}

template <typename type_t>
vec4_t<type_t> operator/(
    const vec4_t<type_t>& a,
    const type_t s)
{
    return vec4_t<type_t>{
        a.x/s, a.y/s, a.z/s, a.w/s
    };
}

template <typename type_t>
vec4_t<type_t> operator-(
    const vec4_t<type_t>& a)
{
    return vec4_t<type_t>{
        -a.x, -a.y, -a.z, -a.w
    };
}

typedef vec4_t<float> vec4f_t;

} // namespace {}
} // namespace tengu
